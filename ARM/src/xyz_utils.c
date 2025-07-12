#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <complex.h>

// #include "adi_fft_wrapper.h"
#include "syslog.h"
#include "wav_file.h"
#include "fft.h"
#include "xyz_utils.h"

#define N_FFT 4096

#pragma align 32
cplx          audioInBuffer[N_FFT];
#pragma align 32
float         audioOutBuffer[N_FFT];
// #pragma align 32
// complex_float tempBuffer[N_FFT/2];

XYZ_Scale XYZ_Maj = {0, 2, 4, 5, 7, 9, 11};
XYZ_Scale XYZ_Min = {0, 2, 3, 5, 7, 8, 10};

float XYZ_hz_to_midi(float freq) {
    float midi;
    midi = 12 * (log2(freq) - log2(440.0)) + 69;
    return midi;
}

void XYZ_midi_to_note(float midi_note, char *buffer, size_t buffer_size) {
    // XYZ_Note note;
    // A lookup table for the 12 pitch classes. Stored in read-only memory.
    const char* note_names[12] = { "C", "C#", "D", "D#", "E", "F",
                                   "F#", "G", "G#", "A", "A#", "B" };
    int midi = round(midi_note);
    // Handle out-of-range MIDI numbers
    if (midi < 0 || midi > 127) {
        snprintf(buffer, buffer_size, "Invalid");
        return;
    }

    // 1. Find the note name using the modulo operator
    const char* note_name = note_names[midi % 12];
    // 2. Calculate the octave. MIDI note 60 is C4, so (60/12)-1 = 4.
    int octave = (midi / 12) - 1;

    // 3. Safely format the final string into the user-provided buffer
    snprintf(buffer, buffer_size, "%s%d", note_name, octave);
}

XYZ_TrackLength *xyz_get_length(char *fname) {
   /* 1. Open the WAV file in binary read mode (fopen(filename, "rb")).
   2. Read the header. The WAV header is typically 44 bytes. You would read this into a custom struct.
   3. Extract key fields:
       * Sample Rate: Located at byte offset 24 (4 bytes).
       * Number of Channels: Located at byte offset 22 (2 bytes).
       * Bits Per Sample: Located at byte offset 34 (2 bytes).
       * Data Size: The size of the raw audio data, located at byte offset 40 (4 bytes).
   4. Calculate the duration:
       * BytesPerSample = BitsPerSample / 8
       * TotalSamples = DataSize / (NumChannels * BytesPerSample)
       * DurationInSeconds = TotalSamples / SampleRate */

    // TODO: Add check to see if it is actually a wav file

    XYZ_TrackLength *tl = NULL;
    WAV_FILE *wf = NULL;

    tl = (XYZ_TrackLength *)malloc(sizeof(XYZ_TrackLength));
    if (tl == NULL) {
        syslog_printf("Failed to allocate memory for XYZ_TrackLength struct\n");
        return NULL;
    }
    memset(tl, 0, sizeof(XYZ_TrackLength));

    wf = (WAV_FILE *)malloc(sizeof(WAV_FILE));
    if (wf == NULL) {
        syslog_printf("Failed to allocate memory for WAV_FILE struct\n");
        free(tl);
        return NULL;
    }
    memset(wf, 0, sizeof(WAV_FILE));

    wf->fname = (char *)fname;
    wf->isSrc = true;

    int ok;
    ok = openWave(wf);
    if (!ok) {
        syslog_printf("Failed to open %s\n", wf->fname);
        free(tl);
        tl = NULL;
    } else {
        syslog_printf("Opened %s successfully.\n", wf->fname);
        syslog_printf("Sample rate: %u\n", wf->sampleRate);
        syslog_printf("Number of channels: %u\n", wf->channels);
        syslog_printf("Bits per sample: %lu\n", wf->waveInfo.bitsPerSample);
        syslog_printf("Data size: %lu\n", wf->dataSize);

        // Ensure byteRate is not zero to avoid division by zero
        if (wf->waveInfo.byteRate > 0) {
            // Use integer scaling for millisecond precision.
            // Use uint64_t to avoid overflow during multiplication.
            uint64_t total_milliseconds =
                ((uint64_t)wf->waveInfo.dataSize * 1000) /
                wf->waveInfo.byteRate;
            tl->hours = total_milliseconds / (1000 * 60 * 60);
            total_milliseconds %= (1000 * 60 * 60);
            tl->mins = total_milliseconds / (1000 * 60);
            total_milliseconds %= (1000 * 60);
            tl->secs = total_milliseconds / 1000;
            tl->ms = total_milliseconds % 1000;

            syslog_printf("Track length: %d:%02d:%02d.%03d\n", tl->hours, tl->mins,
                              tl->secs, tl->ms);
        }
        closeWave(wf);
    }
    free(wf);

    return tl;
}

XYZ_Key *xyz_estimate_key(char *fname) {

    XYZ_Key *key = NULL;
    WAV_FILE *wf = NULL;

    key = (XYZ_Key *)malloc(sizeof(XYZ_Key));
    if (key == NULL) {
        syslog_printf("Failed to allocate memory for XYZ_Key struct\n");
        return key;
    }
    memset(key, 0, sizeof(XYZ_Key));
    wf = (WAV_FILE *)malloc(sizeof(WAV_FILE));
    if (wf == NULL) {
        syslog_printf("Failed to allocate memory for WAV_FILE struct\n");
        free(key);
        return NULL;
    }
    memset(wf, 0, sizeof(WAV_FILE));

    key->KEY_UNKNOWN = true;

    wf->fname = (char *)fname;
    wf->isSrc = true;

    int ok;
    ok = openWave(wf);
    if (!ok) {
        syslog_printf("Failed to open %s\n", wf->fname);
    } else {

        void *result;
        // void *result_fft;
        // void *result_mag;
        size_t samplesRead;
        int twiddle_stride = 1;

        syslog_printf("Taking rfft with %d samples.\n", N_FFT);
        // Investigating the following syntax

        // Zero out the buffers before use
        memset(audioInBuffer, 0, sizeof(audioInBuffer));
        memset(audioOutBuffer, 0, sizeof(audioOutBuffer));

        // Create a temporary buffer for 16-bit integer samples
        int16_t tempAudioBuffer[N_FFT];
        memset(tempAudioBuffer, 0, sizeof(tempAudioBuffer));

        samplesRead = readWave(wf, tempAudioBuffer, N_FFT);
        if (samplesRead > 0) {
            // Convert 16-bit integer samples to complex double and normalize
            for (size_t i = 0; i < samplesRead; i++) {
                audioInBuffer[i] = (double)tempAudioBuffer[i] / 32768.0 + 0.0 * I;
            }

            // Process the 'samplesRead' samples in 'audioInBuffer'
            // result = accel_rfft_large_mag_sq(
            //     audioInBuffer, audioOutBuffer, tempBuffer,
            //     accel_twiddles_4096, twiddle_stride,
            //     1.0, N_FFT);
            // if (!result) {
            //     syslog_printf("Error while taking rfft.\n");
            result = fft_mag(audioInBuffer, audioOutBuffer, N_FFT);
            if (!result) {
                syslog_printf("Error while taking fft.\n");
            }
        }

        /*
        samplesRead = readWave(wf, audioInBuffer, N_FFT);
        if (samplesRead > 0) {
            result_fft = rfft4096(audioInBuffer, tempBuffer);
            if (!result_fft) {
                syslog_printf("Error while taking rfft.\n");
            }
            result_mag = rfft_mag(tempBuffer, audioOutBuffer, N_FFT);
            if (!result_mag) {
                syslog_printf("Error while taking mag.\n");
            } else {
                syslog_printf("Nice one bro!");
            }
        }
        */
        // syslog_printf("fft_mag of first sample: %.6f", audioOutBuffer[0]);

        float hps_result[N_FFT / 4];
        for (int i = 0; i < N_FFT / 4; i++) {
            hps_result[i] = audioOutBuffer[i] *      // Original
                            audioOutBuffer[i * 2] *  // Downsampled by 2
                            audioOutBuffer[i * 3] *  // Downsampled by 3
                            audioOutBuffer[i * 4];   // Downsampled by 4
        }
        int max_index;
        float freq;

        max_index = 0;
        for (int i = 1; i < N_FFT / 4; i++) {
            if (hps_result[i] > hps_result[max_index]) {
                max_index = i;
            }
        }
        freq = max_index * wf->sampleRate / N_FFT;
        syslog_printf("Estimated fundamental freq: %f", freq);

        float midi;

        char *keyBuffer[8];

        midi = XYZ_hz_to_midi(freq);
        XYZ_midi_to_note(midi, keyBuffer, sizeof(keyBuffer));
        syslog_printf("Estimated key: %s", keyBuffer);

        closeWave(wf);
     }
    free(wf);
    if (!(key->KEY_UNKNOWN)) {
            xyz_update_key_string(key);
    }

    return key;
}

void xyz_update_key_string(XYZ_Key *key) {
    char key_string[32] = "";

    switch (key->tonic) {
        case 0:
            strcat(key_string, "C ");
            break;
        case 2:
            strcat(key_string, "D ");
            break;
        case 4:
            strcat(key_string, "E ");
            break;
        case 5:
            strcat(key_string, "F ");
            break;
        case 7:
            strcat(key_string, "G ");
            break;
        case 9:
            strcat(key_string, "A ");
            break;
        case 11:
            strcat(key_string, "B ");
            break;
    }
    switch (key->accidental) {
        case 1:
            strcat(key_string, "# ");
            break;
        case 0:
            strcat(key_string, " ");
            break;
        case -1:
            strcat(key_string, "b");
            break;
    }
    switch (key->scale) {
        case 1:
            strcat(key_string, "Maj");
            break;
        case 2:
            strcat(key_string, "Min");
            break;
    }

    strcpy(key->key_string, key_string);
}

XYZ_BPM xyz_estimate_bpm(char *fname) {
    XYZ_BPM bpm = {-1, -1};

    return bpm;
}

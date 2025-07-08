#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <adi_fft_wrapper.h>

#include "syslog.h"
#include "wav_file.h"
#include "xyz_utils.h"

XYZ_Scale XYZ_Maj = {0, 2, 4, 5, 7, 9, 11};
XYZ_Scale XYZ_Min = {0, 2, 3, 5, 7, 8, 10};

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

/*
    int ok;
    ok = openWave(wf);
    if (!ok) {
        syslog_printf("Failed to open %s\n", wf->fname);
    } else {

        // FFT
        // Define a buffer to hold the audio samples
        #define BUFFER_SIZE 4096
        #define N_FFT 4096

        // int16_t audioBuffer[BUFFER_SIZE];
        #pragma align 32
        static float         audioBuffer[BUFFER_SIZE];
        #pragma align 32
        static complex_float output[BUFFER_SIZE];

        size_t samplesRead;
        int twiddle_stride = 1;
        syslog_printf("Taking rfft with %d samples.\n", N_FFT);
        do {
            samplesRead = readWave(wf, audioBuffer, BUFFER_SIZE);
            if (samplesRead > 0) {
                // Process the 'samplesRead' samples in 'audioBuffer'
                complex_float *result = accel_rfft_large(audioBuffer, output,
                                                         accel_twiddles_4096,
                                                         twiddle_stride, 1.0, N_FFT);
                if (!result)
                    syslog_printf("Error while taking rfft.\n");
            }
        } while (samplesRead > 0);
        closeWave(wf);
     }
*/
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

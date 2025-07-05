#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <adi_fft_wrapper.h>

#include "syslog.h"
#include "wav_file.h"
#include "xyz_utils.h"

TrackLength xyz_get_length(char *fname) {
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

    TrackLength tl = {-1, -1, -1, -1};

    WAV_FILE *wf = NULL;

    /* WAV_FILE *wf = malloc(sizeof(WAV_FILE)); // Allocate memory */
    wf = (WAV_FILE *)malloc(sizeof(WAV_FILE));
    if (wf == NULL) {
        syslog_printf("Failed to allocate memory for WAV_FILE struct\n");
        return tl;
    }
    memset(wf, 0, sizeof(WAV_FILE));

    wf->fname = (char *)fname;
    wf->isSrc = true;

    ok = openWave(wf);
    if (!ok) {
        syslog_printf("Failed to open %s\n", wf->fname);
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
            tl.hours = total_milliseconds / (1000 * 60 * 60);
            total_milliseconds %= (1000 * 60 * 60);
            tl.mins = total_milliseconds / (1000 * 60);
            total_milliseconds %= (1000 * 60);
            tl.secs = total_milliseconds / 1000;
            tl.ms = total_milliseconds % 1000;

            syslog_printf("Track length: %lu:%lu:%lu\n", tl.mins, tl.secs,
                              tl.ms);
        }
    }
    closeWave(wf);
    free(wf);

    return tl;
}

KEY xyz_estimate_key(char *fname) {

    WAV_FILE *wf = NULL;

    wf = (WAV_FILE *)malloc(sizeof(WAV_FILE));
    if (wf == NULL) {
        syslog_printf("Failed to allocate memory for WAV_FILE struct\n");
        return tl;
    }
    memset(wf, 0, sizeof(WAV_FILE));

    wf->fname = (char *)fname;
    wf->isSrc = true;

    ok = openWave(wf);
    if (!ok) {
        syslog_printf("Failed to open %s\n", wf->fname);
    } else {

        // FFT
        // Define a buffer to hold the audio samples
        #define BUFFER_SIZE 4096
        #define N_FFT 4096

        /* int16_t audioBuffer[BUFFER_SIZE]; */
        #pragma align 32
        float         audioBuffer[BUFFER_SIZE];
        #pragma align 32
        complex_float output[BUFFER_SIZE];

        size_t samplesRead;
        int twiddle_stride = 1;

        do {
            samplesRead = readWave(&myWaveFile, audioBuffer, BUFFER_SIZE);
            if (samplesRead > 0) {
                // Process the 'samplesRead' samples in 'audioBuffer'
                complex_float *result = accel_rfft_large(audioBuffer, output,
                                                         accel_twiddles_4096,
                                                         twiddle_stride, 1.0, N_FFT);
                if (!result)
                    syslog_printf("Unable to take rfft.\n");
            }
        } while (samplesRead > 0);
     }

    closeWave(wf);
    free(wf);

    return KEY_UNKNOWN;
}

BPM xyz_estimate_bpm(char *fname) {
    BPM bpm = {-1, -1};

    return bpm;
}

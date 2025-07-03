#ifndef _xyz_utils_h
#define _xyz_utils_h

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include <adi_fft_wrapper.h>

typedef enum {
    KEY_UNKNOWN = -1,
    KEY_A = 0,
    KEY_ASHARP_BFLAT = 1,
    KEY_B = 2,
    KEY_C = 3,
    KEY_CSHARP_DFLAT = 4,
    KEY_D = 5,
    KEY_DSHARP_EFLAT = 6,
    KEY_E = 7,
    KEY_F = 8,
    KEY_FSHARP_GFLAT = 9,
    KEY_G = 10,
    KEY_GSHARP_AFLAT = 11
} KEY;

typedef struct {
    int hours;
    int mins;
    int secs;
    int ms;
} TrackLength;

typedef struct {
    int whole;
    int decimal;
} BPM;

/*!****************************************************************
 * @brief Queries the track length for a specified file.
 *
 * @param [in]  fname      Pointer to a filename
 *
 * @return TrackLength, where {-1, -1, -1, -1} means an error occurred,
 * position 1 is hours, position 2 is minutes, position 3 is seconds,
 * pos 4 is milliseconds.
 ******************************************************************/
TrackLength xyz_get_length(char *fname);

/*!****************************************************************
 * @brief Estimates the key for a specified file.
 *
 * @param [in]  fname      Pointer to a filename
 *
 * @return KEY, where -1 means an error occurred, or one of 12 musical
 * keys from the traditional western scale.
 ******************************************************************/
KEY xyz_estimate_key(char *fname);

/*!****************************************************************
 * @brief Estimates the beats per minute for a specified file.
 *
 * @param [in]  fname      Pointer to a filename
 *
 * @return BPM where {-1, -1} means an error occurred, position 1 is
 * an integer representing the whole part of the decimal and position
 * 2 is an integer representing the decimal part.
 ******************************************************************/
BPM xyz_estimate_bpm(char *fname);

#endif // _xyz_utils_h

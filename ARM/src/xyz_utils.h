#ifndef _xyz_utils_h
#define _xyz_utils_h

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include <adi_fft_wrapper.h>

// Inspiration for structures comes from librosa
// https://librosa.org/
// XYZ_TONIC + XYZ_ACC = TOTAL_OFFSET
typedef enum {
    XYZ_C = 0,
    XYZ_D = 2,
    XYZ_E = 4,
    XYZ_F = 5,
    XYZ_G = 7,
    XYZ_A = 9,
    XYZ_B = 11
} XYZ_TONIC;

typedef enum {
    XYZ_SHARP = 1,
    XYZ_NAT = 0,
    XYZ_FLAT = -1,
    XYZ_DOUBLE_SHARP = 2,
    XYZ_DOUBLE_FLAT = -2
} XYZ_ACC;

typedef int XYZ_Scale[7];

XYZ_Scale XYZ_Maj = {0, 2, 4, 5, 7, 9, 11};
XYZ_Scale XYZ_Min = {0, 2, 3, 5, 7, 8, 10};

/* # A dictionary converting the tonic name to the associated major key, e.g. C Dorian uses the notes of the Bb major scale, hence MAJOR_DICT['dor']['C'] = 'B♭' */
/* MAJOR_DICT = { */
/*     'ion': {'C': 'C', 'D': 'D', 'E': 'E', 'F': 'F', 'G': 'G', 'A': 'A', 'B': 'B'}, */
/*     'dor': {'C': 'B♭', 'D': 'C', 'E': 'D', 'F': 'E♭', 'G': 'F', 'A': 'G', 'B': 'A'}, */
/*     'phr': {'C': 'A♭', 'D': 'B♭', 'E': 'C', 'F': 'D♭', 'G': 'E♭', 'A': 'F', 'B': 'G'}, */
/*     'lyd': {'C': 'G', 'D': 'A', 'E': 'B', 'F': 'C', 'G': 'D', 'A': 'E', 'B': 'F♯'}, */
/*     'mix': {'C': 'F', 'D': 'G', 'E': 'A', 'F': 'B♭', 'G': 'C', 'A': 'D', 'B': 'E'}, */
/*     'aeo': {'C': 'E♭', 'D': 'F', 'E': 'G', 'F': 'A♭', 'G': 'B♭', 'A': 'C', 'B': 'D'}, */
/*     'loc': {'C': 'D♭', 'D': 'E♭', 'E': 'F', 'F': 'G♭', 'G': 'A♭', 'A': 'B♭', 'B': 'C'} */
/* } */

typedef enum {
    XYZ_ION = 0,
    XYZ_DOR = 1,
    XYZ_PHR = 2,
    XYZ_LYD = 3,
    XYZ_MIX = 4,
    XYZ_AEO = 5,
    XYZ_LOC = 6
} XYZ_MODE;

typedef struct {
    XYZ_TONIC note;
    XYZ_ACC accidental;
    int octave;
    int cents;
} XYZ_Note;

typedef struct {
    XYZ_TONIC tonic;
    XYZ_ACC accidental;
    XYZ_Scale scale;
    XYZ_MODE mode;
} XYZ_Key;

/* typedef enum { */
/*     KEY_UNKNOWN = -1, */
/*     KEY_A = 0, */
/*     KEY_ASHARP_BFLAT = 1, */
/*     KEY_B = 2, */
/*     KEY_C = 3, */
/*     KEY_CSHARP_DFLAT = 4, */
/*     KEY_D = 5, */
/*     KEY_DSHARP_EFLAT = 6, */
/*     KEY_E = 7, */
/*     KEY_F = 8, */
/*     KEY_FSHARP_GFLAT = 9, */
/*     KEY_G = 10, */
/*     KEY_GSHARP_AFLAT = 11 */
/* } KEY; */

typedef struct {
    int hours;
    int mins;
    int secs;
    int ms;
} XYZ_TrackLength;

typedef struct {
    int whole;
    int decimal;
} XYZ_BPM;

void XYZ_key_to_degrees(XYZ_Key key);

/*!****************************************************************
 * @brief Queries the track length for a specified file.
 *
 * @param [in]  fname      Pointer to a filename
 *
 * @return TrackLength, where {-1, -1, -1, -1} means an error occurred,
 * position 1 is hours, position 2 is minutes, position 3 is seconds,
 * pos 4 is milliseconds.
 ******************************************************************/
XYZ_TrackLength xyz_get_length(char *fname);

/*!****************************************************************
 * @brief Estimates the key for a specified file.
 *
 * @param [in]  fname      Pointer to a filename
 *
 * @return KEY, where -1 means an error occurred, or one of 12 musical
 * keys from the traditional western scale.
 ******************************************************************/
XYZ_Key xyz_estimate_key(char *fname);

/*!****************************************************************
 * @brief Estimates the beats per minute for a specified file.
 *
 * @param [in]  fname      Pointer to a filename
 *
 * @return BPM where {-1, -1} means an error occurred, position 1 is
 * an integer representing the whole part of the decimal and position
 * 2 is an integer representing the decimal part.
 ******************************************************************/
XYZ_BPM xyz_estimate_bpm(char *fname);

#endif // _xyz_utils_h

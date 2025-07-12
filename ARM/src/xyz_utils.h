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

typedef enum {
    XYZ_MAJ = 1,
    XYZ_MIN = 2,
} XYZ_SCALE;

typedef int XYZ_Scale[8];

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
    char note_name[8];
} XYZ_Note;

typedef struct {
    bool KEY_UNKNOWN;
    XYZ_TONIC tonic;
    XYZ_ACC accidental;
    XYZ_SCALE scale;
    XYZ_MODE mode;
    char key_string[32];
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

float XYZ_hz_to_midi(float freq);

void XYZ_midi_to_note(float midi_note, char *buffer, size_t buffer_size);

void XYZ_key_to_degrees(XYZ_Key key);

/*!****************************************************************
 * @brief Queries the track length for a specified file.
 *
 * This function returns a pointer to a dynamically allocated
 * XYZ_TrackLength struct. The caller is responsible for freeing
 * the memory allocated for the struct using free().
 *
 * @param [in]  fname      Pointer to a filename
 *
 * @return A pointer to an XYZ_TrackLength struct on success,
 *         or NULL on failure.
 *
 * @note Example usage:
 *
 *   XYZ_TrackLength *length = xyz_get_length("my_song.wav");
 *   if (length) {
 *       printf("Track length: %d:%02d:%02d.%03d\n",
 *              length->hours, length->mins, length->secs, length->ms);
 *       free(length);
 *   } else {
 *       printf("Could not get track length.\n");
 *   }
 ******************************************************************/
XYZ_TrackLength *xyz_get_length(char *fname);

/*!****************************************************************
 * @brief Estimates the key for a specified file.
 *
 * @param [in]  fname      Pointer to a filename
 *
 * @return A pointer to an XYZ_Key struct on success, or NULL on failure.
 * The caller is responsible for freeing the memory.
 ******************************************************************/
XYZ_Key *xyz_estimate_key(char *fname);

void xyz_update_key_string(XYZ_Key *key);

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

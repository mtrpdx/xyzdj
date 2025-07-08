#ifndef _browse_h
#define _browse_h

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "FreeRTOS.h"
#include "semphr.h"
#include "shell.h"

#include "browse_cfg.h"

/*!****************************************************************
 * @brief BROWSE_TERM_WRITE and BROWSE_TERM_READ must be defined.
 *
 * These functions have no defaults. Their definitions can be
 * provided in browse_cfg.h.
 ******************************************************************/
#ifndef BROWSE_TERM_WRITE
#error You must define BROWSE_TERM_WRITE
#endif

#ifndef BROWSE_TERM_READ
#error You must define BROWSE_TERM_READ
#endif

/*!****************************************************************
 * @brief  The function used for time.
 ******************************************************************/
#ifndef BROWSE_TIME
#define BROWSE_TIME    time
#endif


/*!****************************************************************
 * @brief  The function used to allocate memory for browse.
 *
 * This defaults to the standard C library malloc if not defined
 * otherwise.
 ******************************************************************/
#ifndef BROWSE_MALLOC
#define BROWSE_MALLOC  malloc
#endif

/*!****************************************************************
 * @brief  The function used to free memory.
 *
 * This defaults to the standard C library free if not defined
 * otherwise.
 ******************************************************************/
#ifndef BROWSE_FREE
#define BROWSE_FREE    free
#endif

/*!****************************************************************
 * @brief  The function used to reallocate memory for browse.
 *
 * This defaults to the standard C library malloc if not defined
 * otherwise.
 ******************************************************************/
#ifndef BROWSE_REALLOC
#define BROWSE_REALLOC realloc
#endif

/*!****************************************************************
 * @brief Simple append buffer.
 *
 * Heap-allocated string that can be appended to. Allows for flushing
 * buffer to standard output in a single call, avoids flickering
 * effects.
 ******************************************************************/
#ifndef _ABUF
#define _ABUF
struct abuf {
    char *b;
    int len;
};
#define ABUF_INIT {NULL,0}
#endif

/*!****************************************************************
 * @brief The supported user actions.
 ******************************************************************/
enum KEY_ACTION{
        KEY_NULL = 0,       /* NULL */
        ENTER = 13,         /* Enter */
        Q = 17,        /* Ctrl-q */
        ESC = 27,           /* Escape */
        BACKSPACE =  127,   /* Backspace */
        /* The following are just soft codes, not really reported by the
         * terminal directly. */
        ARROW_LEFT = 1000,
        ARROW_RIGHT,
        ARROW_UP,
        ARROW_DOWN,
        DEL_KEY,
        HOME_KEY,
        END_KEY,
        PAGE_UP,
        PAGE_DOWN
};

/*!****************************************************************
 * @brief Used by browse to determine how to display info.
 *
 * If an entry is a file, that file's extension is displayed.
 ******************************************************************/
typedef enum {
    FT_FILE,
    FT_DIR,
    FT_INFO
} FileType;

/*!****************************************************************
 * @brief The supported file extensions.
 *
 * FileExt < 0 means the file is not supported.
 ******************************************************************/
typedef enum {
    EXT_UNKNOWN = -1,
    EXT_WAV = 0,
    EXT_MP3 = 1
} FileExt;

/*!****************************************************************
 * @brief Holds info about a single file.
 ******************************************************************/
typedef struct {
    char name[MAX_FILENAME_LENGTH];
    FileExt ext;
    FileType type;
} FileEntry;

/*!****************************************************************
 * @brief A single line of console output.
 ******************************************************************/
typedef struct brow {
    int idx;            /* Row index in the browser, zero-based. */
    int size;           /* Size of the row, excluding the null term. */
    int rsize;          /* Size of the rendered row. */
    char *chars;        /* Row content. */
    char *render;       /* Row content "rendered" for screen (for TABs). */
    unsigned char *hl;  /* Syntax highlight type for each character in render.*/
    FileEntry fentry;      /* File entry (file or directory). */
} brow;

/*!****************************************************************
 * @brief Unused for now.
 ******************************************************************/
typedef struct hlcolor {
    int r,g,b;
} hlcolor;

/*!****************************************************************
 * @brief Browse context block.
 ******************************************************************/
struct browseContext {
    int cx, cy;      /* Cursor x and y position in characters */
    int rowoff;      /* Offset of row displayed. */
    int coloff;      /* Offset of column displayed. */
    int screenrows;  /* Number of rows that we can show */
    int screencols;  /* Number of cols that we can show */
    int numrows;     /* Number of rows */
    brow *row;       /* Rows */
    char *dirname;   /* Currently open folder */
    char statusmsg[80];
    time_t statusmsg_time;
    void *usr;
    int info_screen_active; /* Added to track file info screen state */
};


/* typedef struct LIBRARY { */
/*     char lname[64]; */
/*     int num_files; */

/*     typedef struct MUSIC_FILE { */
/*         char *fname; // filename */
/*         FILE *f; */
/*         char fext[4]; // file extension, assumed to be 3 chars */
/*         void *lock; */
/*         size_t dataSize; */
/*         unsigned channels; */
/*         unsigned sampleRate; */
/*     } MUSIC_FILE; */
/* } LIBRARY; */

typedef struct MUSIC_FILE_INFO {
    uint16_t numChannels;
    uint32_t sampleRate;
} MUSIC_FILE_INFO;


typedef struct MUSIC_FILE {
    char *fname; // filename
    FILE *f;
    char fext[4]; // file extension, assumed to be 3 chars
    /* size_t dataSize; */
    /* unsigned channels; */
    /* unsigned sampleRate; */
} MUSIC_FILE;

/* int browse_init(); */

/* Globals */

// Buffer for the current working directory path
/* static char g_currPath[MAX_PATH_LENGTH]; */

/* Function Prototypes */

/*!****************************************************************
 * @brief Custom strcmp implementation
 *
 * @param [in]  s1         Pointer to first string
 * @param [in]  s2         Pointer to second string
 *
 * @return Returns the difference between s1 and s2.
 ******************************************************************/
static int browse_strcmp(const char *s1, const char *s2);

/*!****************************************************************
 * @brief Retrieves the extension for a specified file.
 *
 * @param [in]  fname      Pointer to a filename
 *
 * @return Returns FileExt where a value < 0 is an unknown extension.
 ******************************************************************/
static FileExt getFileExtension(const char *fname);

/*!****************************************************************
 * @brief Set a browser status message.
 *
 * Sets a status message at the bottom of the screen.
 *
 * @param [in]  fmt      Pointer to a string, including formatting
 ******************************************************************/
void browseSetStatusMessage(const char *fmt, ...);

/*!****************************************************************
 * @brief Called at start to set up.
 *
 * Uses low-level terminal commands to clear screen and go home.
 ******************************************************************/
void browseAtStart(void);

/*!****************************************************************
 * @brief Called at exit to clean up.
 *
 * Uses low-level terminal commands to clear screen and go home.
 ******************************************************************/
void browseAtExit(void);

/*!****************************************************************
 *  @brief Reads user keypress.
 *
 * Read a key from the terminal put in raw mode, trying to handle
 * escape sequences.
 *
 * @return Returns KEY_ACTION
 ******************************************************************/
int browseReadKey();

/*!****************************************************************
 * @brief Query the horizontal cursor position.
 *
 *
 * Use the ESC [6n escape sequence to query the horizontal cursor position
 * and return it. On error -1 is returned, on success the position of the
 * cursor is stored at *rows and *cols and 0 is returned.

 * @param [in]  rows       Pointer to store the row position
 * @param [in]  cols       Pointer to store the col position
 *
 * @return Returns -1 if error, or stores position of cursor at *rows.
 * and *cols and returns 0 if succesful.
 ******************************************************************/
int browseGetCursorPosition(int *rows, int *cols);

/*!****************************************************************
 * @brief Query the number of columns in current terminal.
 *
 * This call never fails even if incorrect.

 * @param [in]  rows       Pointer to store the row position
 * @param [in]  cols       Pointer to store the col position
 *
 * @return Stores position of cursor at *rows and *cols and
 * returns 0 if succesful.
 ******************************************************************/
int browseGetWindowSize(int *rows, int *cols);

/*!****************************************************************
 *  @brief Update the rendered version of a row.
 *
 * @param [in]  row        Pointer to instance of brow, which is a
 *                         custom structure.
 ******************************************************************/
void browseUpdateRow(brow *row);


/*!****************************************************************
 * @brief Insert row at specified position.
 *
 * Shifts the rows below if required.
 *
 * @param [in]  at         Row location for insertion
 * @param [in]  s          Pointer to string to insert
 * @param [in]  len        Length of string including null termination
 * @param [in]  type       Filetype for printing purposes. FT_INFO is
 *                         general purpose
 *****************************************************/
void browseInsertRow(int at, const char *s, size_t len, FileType type);

/*!****************************************************************
 * @brief Turn the browser rows into a single heap-allocated string.
 *
 * Returns the pointer to the heap-allocated string and populate the
 * integer pointed by 'buflen' with the size of the string, excluding
 * the final nulterm.

 * @param [in]  buflen     Pointer to store length of string
 *
 * @return Populate the integer pointed by 'buflen' with the size of
 * the string, excluding the final nulterm. Returns the pointer to the
 * heap-allocated string.
 * ******************************************************************/
char *browseRowsToString(int *buflen);

/*!****************************************************************
 * @brief Free memory used by browse.
 * ******************************************************************/
void browseFreeMemory();

/*!****************************************************************
 * @brief Append to 'abuf' append buffer.
 *
 * @param [in]  ab       Pointer to append buffer
 * @param [in]  s        Pointer to string to append
 * @param [in]  len      length of string s including null termination
 * ******************************************************************/
void browseAbAppend(struct abuf *ab, const char *s, int len);

/*!****************************************************************
 * @brief Wrapper around BROWSE_FREE.
 *
 * @param [in]  ab       Pointer to append buffer
 * ******************************************************************/
void browseAbFree(struct abuf *ab);

/*!****************************************************************
 * @brief Writes entire screen with VT100 escape characters.
 *
 * @param       B        Global browserContext initial logical state
 * ******************************************************************/
void browseRefreshScreen(void);

/*!****************************************************************
 * @brief Handle cursor position change.
 *
 * @param [in]  key      Key input, either up or down
 * ******************************************************************/
void browseMoveCursor(int key);

/*!****************************************************************
 * @brief Show file info for a specified row.
 *
 * @param [in]  row        Pointer to instance of brow, which is a
 *                         custom structure.
 ******************************************************************/
static void browseShowFileInfo(brow *row);

/*!****************************************************************
 * @brief 'Enter' directory.
 *
 * Updates B.dirname with new path at row and rescans directory
 *
 * @param [in]  row        Pointer to instance of brow, which is a
 *                         custom structure.
 ******************************************************************/
static void browseEnterDir(brow *row);

/*!****************************************************************
 * @brief Process standard input from terminal.
 *
 * @return Performs action based on browseReadKey input. Returns 1
 * if successful.
 ******************************************************************/
int browseProcessKeypress(void);

/*!****************************************************************
 * @brief Update window size on terminal resize.
 *
 * @return If error, return -1, else return 0.
 ******************************************************************/
int browseUpdateWindowSize(void);

/*!****************************************************************
 * @brief Initialize browseContext and update window size.
 *
 * @return Result from browseUpdateWindowSize
 ******************************************************************/
int initBrowse(void);

/*!****************************************************************
 * @brief Compare rows for sorting.
 *
 * @param [in]  a        Pointer to row a
 * @param [in]  b        Pointer to row b
 *
 * @return 0 if no sorting required (FT_INFO), -1 if a==FT_DIR and
 * b==FT_FILE, 1 if a==FT_FILE and b==FT_DIR, return result of
 * browse_stricmp if both a and b are same type.
 * ******************************************************************/
static int compare_brows(const void *a, const void *b);

/*!****************************************************************
 * @brief Scan directory.
 *
 * @param [in]  path     Pointer to path of directory
 *
 * @return -1 if error. Else insert sorted rows and return 0 if
 * successful.
 * ******************************************************************/
static int scanDir(const char* path);

/*!****************************************************************
 * @brief Main function for browse.
 *
 * @param [in]  argc     Number of input arguments
 * @param [in]  argv     Pointer to array of input args
 * @param [in]  usr      Pointer to user context
 * @param [in]  crtname  Pointer to name of root directory
 *
 *
 * @return Frees memory and returns upon error. If no error and user
 * does not exit, should not return from this function.
 * * ******************************************************************/
int browse_main(int argc, char **argv, void *usr, const char *crtname);

#endif // _browse_h

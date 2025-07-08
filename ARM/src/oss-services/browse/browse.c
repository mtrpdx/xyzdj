/* Browse -- Simple file browser designed for use in embedded linux
 *           environment. Emphasis is on portability and simplicity.
 *
 * -----------------------------------------------------------------------
 * Copyright (C) 2025 Martin Rodriguez <mtrpdx at gmail dot com>
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *  Redistributions of source code must retain the above copyright
 *  notice, this list of conditions and the following disclaimer.
 *
 *  Redistributions in binary form must reproduce the above copyright
 *  notice, this list of conditions and the following disclaimer in the
 *  documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 *
 */

#define BROWSE_VERSION "0.0.1"

#ifdef __linux__
#define _POSIX_C_SOURCE 200809L
#endif

#include <stdio.h>
#include "syslog.h"
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdarg.h>
#include <fcntl.h>
#include <signal.h>

#include "ff.h"
#include "fs_devman.h"
#include "term.h"
#include "shell.h"
#include "browse.h"
#include "xyz_utils.h"

static struct browseContext B;

static int browse_stricmp(const char *s1, const char *s2) {
    unsigned char c1, c2;
    while (*s1 && *s2) {
        c1 = tolower((unsigned char)*s1);
        c2 = tolower((unsigned char)*s2);
        if (c1 != c2) {
            return c1 - c2;
        }
        s1++;
        s2++;
    }
    return tolower((unsigned char)*s1) - tolower((unsigned char)*s2);
}

static FileExt getFileExtension(const char *fname) {
    const char *dot = strrchr(fname, '.');
    if (!dot || dot == fname) return EXT_UNKNOWN; // No extension or file starts with a dot.
    const char *ext = dot + 1;

    if (browse_stricmp(ext, "wav") == 0) {
        return EXT_WAV;
    } else if (browse_stricmp(ext, "mp3") == 0) {
        return EXT_MP3;
    }

    return EXT_UNKNOWN;
}


/* ======================= Low level terminal handling ====================== */

/* Called at start to set up */
void browseAtStart(void) {
    BROWSE_TERM_WRITE(B.usr, "\x1b[2J", 4); // Clear screen
    BROWSE_TERM_WRITE(B.usr, "\x1b[H", 3);  // Home
    BROWSE_TERM_WRITE(B.usr, "\x1b=", 2);   // Application keypad mode
}

/* Called at exit to clean up */
void browseAtExit(void) {
    BROWSE_TERM_WRITE(B.usr, "\x1b[2J", 4); // Clear screen
    BROWSE_TERM_WRITE(B.usr, "\x1b[H", 3);  // Home
    BROWSE_TERM_WRITE(B.usr, "\x1b>", 2);   // Numeric keypad mode
}

/* Read a key from the terminal put in raw mode, trying to handle
 * escape sequences. */
int browseReadKey() {
    int nread;
    char c, seq[3];
    while ((nread = BROWSE_TERM_READ(B.usr,&c,1)) == 0);
    if (nread == -1) return -1;

    while(1) {
        switch(c) {
        case ESC:    /* escape sequence */
            /* If this is just an ESC, we'll timeout here. */
            if (BROWSE_TERM_READ(B.usr,seq,1) == 0) return ESC;
            if (BROWSE_TERM_READ(B.usr,seq+1,1) == 0) return ESC;

            /* ESC [ sequences. */
            if (seq[0] == '[') {
                if (seq[1] >= '0' && seq[1] <= '9') {
                    /* Extended escape, read additional byte. */
                    if (BROWSE_TERM_READ(B.usr,seq+2,1) == 0) return ESC;
                    if (seq[2] == '~') {
                        switch(seq[1]) {
                        break;
                        }
                    }
                } else {
                    switch(seq[1]) {
                    case 'A': return ARROW_UP;
                    case 'B': return ARROW_DOWN;
                    }
                }
            }

            /* ESC O sequences. */
            else if (seq[0] == 'O') {
                switch(seq[1]) {
                case 'r': return ARROW_DOWN;
                case 'x': return ARROW_UP;
                }
            }
            break;
        default:
            return c;
        }
    }
}

/* Use the ESC [6n escape sequence to query the horizontal cursor position
 * and return it. On error -1 is returned, on success the position of the
 * cursor is stored at *rows and *cols and 0 is returned. */
int browseGetCursorPosition(int *rows, int *cols) {
    char buf[32];
    unsigned int i = 0;

    /* Report cursor location */
    if (BROWSE_TERM_WRITE(B.usr, "\x1b[6n", 4) != 4) return -1;

    /* Read the response: ESC [ rows ; cols R */
    while (i < sizeof(buf)-1) {
        if (BROWSE_TERM_READ(B.usr,buf+i,1) != 1) break;
        if (buf[i] == 'R') break;
        i++;
    }
    buf[i] = '\0';

    /* Parse it. */
    if (buf[0] != ESC || buf[1] != '[') return -1;
    if (sscanf(buf+2,"%d;%d",rows,cols) != 2) return -1;
    return 0;
}

/* Try to get the number of columns in the current terminal.
 * This call never fails even if wrong. */
int browseGetWindowSize(int *rows, int *cols) {
    /* Try to query the terminal itself. */
    int orig_row, orig_col, retval;

    /* Get the initial position so we can restore it later. */
    retval = browseGetCursorPosition(&orig_row,&orig_col);
    if (retval == -1) goto failed;

    /* Go to right/bottom margin and get position. */
    if (BROWSE_TERM_WRITE(B.usr,"\x1b[999C\x1b[999B",12) != 12) goto failed;
    retval = browseGetCursorPosition(rows,cols);
    if (retval == -1) goto failed;

    /* Restore position. */
    char seq[32];
    snprintf(seq,32,"\x1b[%d;%dH",orig_row,orig_col);
    if (BROWSE_TERM_WRITE(B.usr,seq,strlen(seq)) == -1) {
        /* Can't recover... */
    }
    return 0;

failed:
    return -1;
}

/* ======================= Browser rows implementation ======================= */

/* Update the rendered version and the syntax highlight of a row. */
void browseUpdateRow(brow *row) {
    unsigned int nonprint = 0;
    int j, idx;

   /* Create a version of the row we can directly print on the screen,
     * respecting tabs, substituting non printable characters with '?'. */
    BROWSE_FREE(row->render);
    /* for (j = 0; j < row->size; j++) */
    /*     if (row->chars[j] == TAB) tabs++; */

    unsigned long long allocsize =
        (unsigned long long) row->size + nonprint*9 + 1;
    if (allocsize > UINT32_MAX) {
        printf("Some filename is too long for Browse\n");
        return;
    }

    row->render = BROWSE_MALLOC(row->size + nonprint*9 + 1);
    idx = 0;
    for (j = 0; j < row->size; j++) {
        /* if (row->chars[j] == TAB) { */
        /*     row->render[idx++] = ' '; */
        /*     while((idx+1) % 8 != 0) row->render[idx++] = ' '; */
        /* } else { */
        /*     row->render[idx++] = row->chars[j]; */
        /* } */
        row->render[idx++] = row->chars[j];
    }
    row->rsize = idx;
    row->render[idx] = '\0';

}

/* Insert a row at the specified position, shifting the other rows on the bottom
 * if required. */
void browseInsertRow(int at, const char *s, size_t len, FileType type) {
    if (at > B.numrows) return;
    B.row = BROWSE_REALLOC(B.row,sizeof(brow)*(B.numrows+1));
    if (at != B.numrows) {
        memmove(B.row+at+1,B.row+at,sizeof(B.row[0])*(B.numrows-at));
        for (int j = at+1; j <= B.numrows; j++) B.row[j].idx++;
    }
    B.row[at].size = len;
    B.row[at].chars = BROWSE_MALLOC(len+1);
    memcpy(B.row[at].chars,s,len+1);
    B.row[at].hl = NULL;
    B.row[at].render = NULL;
    B.row[at].rsize = 0;
    B.row[at].idx = at;
    B.row[at].fentry.type = type;
    if (type != FT_INFO) {
        strncpy(B.row[at].fentry.name, s, MAX_FILENAME_LENGTH);
        B.row[at].fentry.name[MAX_FILENAME_LENGTH - 1] = '\0'; // Ensure null termination
    } else {
        B.row[at].fentry.name[0] = '\0';
    }
    browseUpdateRow(B.row+at);
    B.numrows++;
}

/* Free row's heap allocated stuff. */
void browseFreeRow(brow *row) {
    BROWSE_FREE(row->render);
    BROWSE_FREE(row->chars);
    BROWSE_FREE(row->hl);
}

/* Turn the browser rows into a single heap-allocated string.
 * Returns the pointer to the heap-allocated string and populate the
 * integer pointed by 'buflen' with the size of the string, excluding
 * the final nulterm. */
/* TODO: maybe remove at some point  */
char *browseRowsToString(int *buflen) {
    char *buf = NULL, *p;
    int totlen = 0;
    int j;

    /* Compute count of bytes */
    for (j = 0; j < B.numrows; j++)
        totlen += B.row[j].size+ 1; /* +1 or + 2 is for "\n" or "\r\n" at end of every row */
    *buflen = totlen;
    totlen++; /* Also make space for nulterm */

    p = buf = BROWSE_MALLOC(totlen);
    for (j = 0; j < B.numrows; j++) {
        memcpy(p,B.row[j].chars,B.row[j].size);
        p += B.row[j].size;
        *p = '\n';
        p++;
    }
    *p = '\0';
    return buf;
}

void browseFreeMemory() {
    int j;
    if (B.row) {
        for (j = 0; j < B.numrows; j++) {
            if (B.row[j].chars) {
                BROWSE_FREE(B.row[j].chars);
            }
            if (B.row[j].render) {
                BROWSE_FREE(B.row[j].render);
            }
            if (B.row[j].hl) {
                BROWSE_FREE(B.row[j].hl);
            }
        }
        BROWSE_FREE(B.row);
    }

    if (B.dirname) {
        BROWSE_FREE(B.dirname);
    }
}


/* ============================= Terminal update ============================ */

void browseAbAppend(struct abuf *ab, const char *s, int len) {
    char *new = BROWSE_REALLOC(ab->b,ab->len+len);

    if (new == NULL) return;
    memcpy(new+ab->len,s,len);
    ab->b = new;
    ab->len += len;
}

void browseAbFree(struct abuf *ab) {
    BROWSE_FREE(ab->b);
}

/* This function writes the whole screen using VT100 escape characters
 * starting from the logical state of the browser in the global state 'B'. */
void browseRefreshScreen(void) {
    int y;
    brow *r;
    char buf[32];
    struct abuf ab = ABUF_INIT;

    /* syslog_printf("browseRefreshScreen: B.numrows=%d, B.rowoff=%d, B.cy=%d\n", B.numrows, B.rowoff, B.cy); */

    browseAbAppend(&ab,"\x1b[?25l",6); /* Hide cursor. */
    browseAbAppend(&ab,"\x1b[H",3); /* Go home. */

    // Draw top border
    browseAbAppend(&ab, "+", 1);
    for (int i = 0; i < B.screencols - 2; i++) browseAbAppend(&ab, "-", 1);
    browseAbAppend(&ab, "+\r\n", 3);

    for (y = 0; y < B.screenrows; y++) {
        int filerow = B.rowoff+y;

        browseAbAppend(&ab, "|", 1); // Left border

        if (filerow >= B.numrows) {
            for (int i = 0; i < B.screencols - 2; i++) browseAbAppend(&ab, " ", 1);
        } else {
            r = &B.row[filerow];
            /* syslog_printf("  Rendering row %d: '%s' (type %d)\n", filerow, r->chars, r->fentry.type); */
            int content_width = B.screencols - 4; // 2 for borders, 2 for padding

            if (y == B.cy) {
                browseAbAppend(&ab, "\x1b[7m", 4); // Inverse video on
            }

            browseAbAppend(&ab, " ", 1); // Left padding

            // Get and display the file extension
            FileExt ext = getFileExtension(r->render);
            const char *ext_str = "";
            if (ext == EXT_WAV) ext_str = "wav";
            else if (ext == EXT_MP3) ext_str = "mp3";

            char *render_ptr = r->render + B.coloff;
            int rlen = strlen(render_ptr);
            int ext_len = strlen(ext_str);

            // Create the line content
            char line_buf[256];
            int line_len = snprintf(line_buf, sizeof(line_buf), "%s%*s%s",
                                    render_ptr,
                                    content_width - rlen - ext_len > 0 ? content_width - rlen - ext_len : 0,
                                    "",
                                    ext_str);

            // Truncate if necessary
            if (line_len > content_width) {
                line_len = content_width;
            }

            browseAbAppend(&ab, line_buf, line_len);

            // Fill remaining space if line is shorter than content_width
            int remaining_space = content_width - line_len;
            for(int i=0; i<remaining_space; ++i) {
                browseAbAppend(&ab, " ", 1);
            }

            browseAbAppend(&ab, " ", 1); // Right padding

            if (y == B.cy) {
                browseAbAppend(&ab, "\x1b[0m", 4); // All attributes off
            }
        }

        browseAbAppend(&ab, "|", 1); // Right border
        browseAbAppend(&ab, "\r\n", 2);
    }

    // Draw bottom border
    browseAbAppend(&ab, "+", 1);
    for (int i = 0; i < B.screencols - 2; i++) browseAbAppend(&ab, "-", 1);
    browseAbAppend(&ab, "+\r\n", 3);

    /* Second row depends on E.statusmsg and the status message update time. */
    browseAbAppend(&ab,"\x1b[0K",4);
    int msglen = strlen(B.statusmsg);
    if (msglen && BROWSE_TIME(NULL)-B.statusmsg_time < 5)
        browseAbAppend(&ab,B.statusmsg,msglen <= B.screencols ? msglen : B.screencols);

    /* Put cursor at its current position. Note that the horizontal position
     * at which the cursor is displayed may be different compared to 'E.cx'
     * because of TABs. */
    int j;
    int cx = 1;
    int filerow = B.rowoff+B.cy;
    brow *row = (filerow >= B.numrows) ? NULL : &B.row[filerow];
    if (row) {
        for (j = B.coloff; j < (B.cx+B.coloff); j++) {
            /* if (j < row->size && row->chars[j] == TAB) cx += 7-((cx)%8); */
            cx++;
        }
    }
    snprintf(buf,sizeof(buf),"\x1b[%d;%dH",B.cy+1,cx);
    browseAbAppend(&ab,buf,strlen(buf));
    /* browseAbAppend(&ab,"\x1b[?25h",6); /\* Show cursor. *\/ */
    BROWSE_TERM_WRITE(B.usr,ab.b,ab.len);
    browseAbFree(&ab);
}

/* Set a browser status message for the second line of the status, at the
 * end of the screen. */
void browseSetStatusMessage(const char *fmt, ...) {
    va_list ap;
    va_start(ap,fmt);
    vsnprintf(B.statusmsg,sizeof(B.statusmsg),fmt,ap);
    va_end(ap);
    B.statusmsg_time = BROWSE_TIME(NULL);
}

/* ========================= Browser events handling  ======================== */

/* Handle cursor position change because arrow keys were pressed. */
void browseMoveCursor(int key) {
    if (B.info_screen_active) return;
    int filerow = B.rowoff+B.cy;
    int filecol = B.coloff+B.cx;
    int rowlen;
    brow *row = (filerow >= B.numrows) ? NULL : &B.row[filerow];

    switch(key) {
    /* case ARROW_LEFT: */
    /*     if (E.cx == 0) { */
    /*         if (E.coloff) { */
    /*             E.coloff--; */
    /*         } else { */
    /*             if (filerow > 0) { */
    /*                 E.cy--; */
    /*                 E.cx = E.row[filerow-1].size; */
    /*                 if (E.cx > E.screencols-1) { */
    /*                     E.coloff = E.cx-E.screencols+1; */
    /*                     E.cx = E.screencols-1; */
    /*                 } */
    /*             } */
    /*         } */
    /*     } else { */
    /*         E.cx -= 1; */
    /*     } */
    /*     break; */
    /* case ARROW_RIGHT: */
    /*     if (row && filecol < row->size) { */
    /*         if (E.cx == E.screencols-1) { */
    /*             E.coloff++; */
    /*         } else { */
    /*             E.cx += 1; */
    /*         } */
    /*     } else if (row && filecol == row->size) { */
    /*         E.cx = 0; */
    /*         E.coloff = 0; */
    /*         if (E.cy == E.screenrows-1) { */
    /*             E.rowoff++; */
    /*         } else { */
    /*             E.cy += 1; */
    /*         } */
    /*     } */
    /*     break; */
    case ARROW_UP:
        if (B.cy == 0) {
            if (B.rowoff) B.rowoff--;
        } else {
            B.cy -= 1;
        }
        break;
    case ARROW_DOWN:
        if (filerow < B.numrows - 1) {
            if (B.cy == B.screenrows-1) {
                B.rowoff++;
            } else {
                B.cy += 1;
            }
        }
        break;
    /* case HOME_KEY: */
    /*     E.cx = 0; */
    /*     break; */
    /* case END_KEY: */
    /*     E.cx = row->size; */
    /*     break; */
    }
    /* Fix cx if the current line has not enough chars. */
    filerow = B.rowoff+B.cy;
    filecol = B.coloff+B.cx;
    row = (filerow >= B.numrows) ? NULL : &B.row[filerow];
    rowlen = row ? row->size : 0;
    if (filecol > rowlen) {
        B.cx -= filecol-rowlen;
        if (B.cx < 0) {
            B.coloff += B.cx;
            B.cx = 0;
        }
    }
}


static void browseShowFileInfo(brow *row) {
    B.info_screen_active = 1;
    // TODO: Update to show actual music file info
    char row1[MAX_PATH_LENGTH];
    char row2[MAX_PATH_LENGTH];
    char row3[MAX_PATH_LENGTH];

    B.numrows = 0;
    B.row = NULL;
    B.cx = 0;
    B.cy = 0;
    B.rowoff = 0;
    B.coloff = 0;

    // Call function to get file length
    XYZ_TrackLength *tl = xyz_get_length(row->fentry.name);
    // Call function to get key
    /* XYZ_Key *key = xyz_estimate_key(row->fentry.name); */
    // Call function to get BPM
    // bpm = xyz_get_bpm();

    if (tl) {
        snprintf(row1, sizeof(row1), "  Length: %d:%d", tl->mins, tl->secs);
        free(tl);
    } else {
        snprintf(row1, sizeof(row1), "  Length: Unknown");
    }
    /* if (key) {
        if (key->KEY_UNKNOWN) {
            snprintf(row2, sizeof(row2), "  Key: Unknown");
        } else {
            snprintf(row2, sizeof(row2), "  Key: %s", key->key_string);
        }
        free(key);
    } */

    snprintf(row2, sizeof(row2), "  Key: Unknown");

    snprintf(row3, sizeof(row3), "  BPM: 128");
    browseInsertRow(B.numrows, _T("../"), 3, FT_DIR);
    browseInsertRow(B.numrows, "", 0, FT_INFO);
    browseInsertRow(B.numrows, row->fentry.name, strlen(row->fentry.name), FT_INFO);
    browseInsertRow(B.numrows, "", 0, FT_INFO);
    browseInsertRow(B.numrows, row1, strlen(row1), FT_INFO);
    browseInsertRow(B.numrows, row2, strlen(row2), FT_INFO);
    browseInsertRow(B.numrows, row3, strlen(row3), FT_INFO);

}

static void browseEnterDir(brow *row) {
    char new_path[MAX_PATH_LENGTH];
    /* syslog_printf("browseEnterDir: B.dirname='%s', row->fentry.name='%s'\n", B.dirname, row->fentry.name); */
    if (strcmp(row->fentry.name, _T("../")) == 0) {
        // Navigate up one directory
        if (B.dirname[1] == '\0') { // Already at root "/"
            strcpy(new_path, "/");
        } else {
            // B.dirname is like "/foo/bar/"
            char temp[MAX_PATH_LENGTH];
            strncpy(temp, B.dirname, MAX_PATH_LENGTH);
            temp[MAX_PATH_LENGTH - 1] = '\0';
            // remove trailing slash: "/foo/bar"
            temp[strlen(temp) - 1] = '\0';
            // find last slash: pointer to "/" in "/foo"
            char *p = strrchr(temp, '/');
            // copy up to and including that slash
            strncpy(new_path, temp, p - temp + 1);
            new_path[p - temp + 1] = '\0';
        }
    } else {
        // Construct the new path for entering a subdirectory
        if (B.dirname[1] == '\0') { // Root directory "/"
            snprintf(new_path, sizeof(new_path), "/%s", row->fentry.name);
        } else {
            snprintf(new_path, sizeof(new_path), "%s%s", B.dirname, row->fentry.name);
        }
    }
    /* syslog_printf("browseEnterDir: new_path='%s'\n", new_path); */
    // Free current memory and reset browser state
    browseFreeMemory();
    B.numrows = 0;
    B.row = NULL;
    B.cx = 0;
    B.cy = 0;
    B.rowoff = 0;
    B.coloff = 0;
    B.info_screen_active = 0;
    // Update current directory
    B.dirname = BROWSE_MALLOC(strlen(new_path) + 1);
    strcpy(B.dirname, new_path);
    // Re-scan the new directory
    int ok;
    ok = scanDir(B.dirname);
}

/* Process events arriving from the standard input from terminal. */
int browseProcessKeypress(void) {

    int c = browseReadKey();
    switch(c) {
    case ENTER:         /* Enter */
        if (B.numrows > 0) {
            brow *selected_row = &B.row[B.rowoff + B.cy];
            if (selected_row->fentry.type == FT_DIR) {
                browseEnterDir(selected_row);
            } else if (selected_row->fentry.type == FT_FILE) {
                browseShowFileInfo(selected_row);
            }
        }
        break;
    case Q:        /* q */
        /* Quit*/
        return 0;
        break;
    case PAGE_UP:
    case PAGE_DOWN:
        {
        int times = B.screenrows;
        while(times--)
            browseMoveCursor(c == PAGE_UP ? ARROW_UP:
                                            ARROW_DOWN);
        }
        break;

    case ARROW_UP:
    case ARROW_DOWN:
        browseMoveCursor(c);
        break;
    case ESC:
        /* Nothing to do for ESC in this mode. */
        break;
    
    }

    return 1;
}

// TODO : update function names to be unique
int browseUpdateWindowSize(void) {
    if (browseGetWindowSize(&B.screenrows,&B.screencols) == -1) {
        return -1;
    }
    B.screenrows -= 3; /* Get room for borders and status bar. */
    return 0;
}

int initBrowse(void) {
    int ok;
    B.cx = 0;
    B.cy = 0;
    B.rowoff = 0;
    B.coloff = 0;
    B.numrows = 0;
    B.row = NULL;
    B.info_screen_active = 0;
    B.dirname = BROWSE_MALLOC(MAX_PATH_LENGTH);
    if (B.dirname == NULL) return -1;
    strcpy(B.dirname, "/"); // Default to root directory
    ok = browseUpdateWindowSize();
    return(ok);
}

static int compare_brows(const void *a, const void *b) {
    brow *brow_a = (brow *)a;
    brow *brow_b = (brow *)b;

    // If the row is of type FT_INFO, no sorting is required.
    if (brow_a->fentry.type == FT_INFO || brow_b->fentry.type == FT_INFO) {
        return 0;
    }

    /* syslog_printf("Comparing '%s' (type %d) with '%s' (type %d)\n", brow_a->chars, brow_a->fentry.type, brow_b->chars, brow_b->fentry.type); */

    // Directories come before files
    if (brow_a->fentry.type == FT_DIR && brow_b->fentry.type == FT_FILE) {
        /* syslog_printf("  -> Dir before File: -1\n"); */
        return -1;
    }
    if (brow_a->fentry.type == FT_FILE && brow_b->fentry.type == FT_DIR) {
        /* syslog_printf("  -> File after Dir: 1\n"); */
        return 1;
    }

    // If same type, sort alphabetically by name (case-insensitive)
    int result = browse_stricmp(brow_a->chars, brow_b->chars);
    /* syslog_printf("  -> Stricmp result: %d\n", result); */
    return result;
}

static int scanDir(const char* path) {

    void *d;
    FS_DEVMAN_DIRENT *ent;
    int initial_numrows = B.numrows;

    syslog_printf("Scanning directory: %s\n", path);

    if ((d = fs_devman_opendir(path)) != NULL)
    {
        // Add ".." entry for navigating up, unless it's the root directory
        if (strcmp(path, _T("/")) != 0) {
            browseInsertRow(B.numrows, _T("../"), 3, FT_DIR);
        }

        while ((ent = fs_devman_readdir(d)) != NULL) {
            // File begins with '.'
            if (ent->fname[0] == '.') continue;
            // Is directory
            if (ent->flags & FS_DEVMAN_DIRENT_FLAG_DIR) {
                size_t len = strlen(ent->fname);
                char *dirname = BROWSE_MALLOC(len + 2); /* for '/' and '\0' */
                if (dirname == NULL) continue;
                memcpy(dirname, ent->fname, len);
                dirname[len] = '/';
                dirname[len+1] = '\0';
                browseInsertRow(B.numrows, dirname, len + 1, FT_DIR);
                BROWSE_FREE(dirname);
            } else {
                // File extension not recognized
                if (getFileExtension(ent->fname) == EXT_UNKNOWN) continue;
                else browseInsertRow(B.numrows, ent->fname, strlen(ent->fname), FT_FILE);
            }
         }
         fs_devman_closedir(d);
         syslog_printf("Finished scanning. Total entries: %d\n", B.numrows - initial_numrows);
    } else {
        syslog_printf("Failed to open directory: %s\n", path);
        return -1;
    }

    // Sort the entries after scanning all of them
    qsort(B.row, B.numrows, sizeof(brow), compare_brows);

    return 0;
}

/* static char * get_file_extension(const char *fname) */
/* { */
/*     size_t end = strlen(fname); */
/*     // TODO: Currently only works with three letter extensions. */
/*     // Maybe a better method is to search for last period in filename? */
/*     char *curr_ext = (char *)BROWSE_MALLOC(4 * sizeof(char)); */

/*     if (curr_ext == NULL) { */
/*         printf("Memory allocation failed!\n"); */
/*     } */

/*     curr_ext[0] = (fname)[end-3]; */
/*     curr_ext[1] = (fname)[end-2]; */
/*     curr_ext[2] = (fname)[end-1]; */
/*     curr_ext[3] = '\0'; */

/*     return curr_ext; */
/* } */

int browse_main(int argc, char **argv, void *usr, const char *crtname) {
    int ok;

    B.usr = usr;
    ok = initBrowse();
    if (ok < 0) {
        goto failed;
    }
    /* editorSelectSyntaxHighlight(argv[1]); */
    /* ok = editorOpen(argv[1]); */
    /* if (ok < 0) { */
    /*     goto failed; */
    /* } */
    browseSetStatusMessage(
        "HELP: up arrow / down arrow = move | Ctrl-Q = quit | ENTER = open");
    browseAtStart();

    // Initial scan of the current directory
    ok = scanDir(B.dirname);
    if (ok < 0) {
        goto failed;
    }

    ok = 1;
    while(ok) {
        browseRefreshScreen();
        ok = browseProcessKeypress();
    }
    browseAtExit();

failed:
    browseFreeMemory();

    return ok;
}

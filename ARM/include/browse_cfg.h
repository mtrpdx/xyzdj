/**
 * Copyright (c) 2023 - Analog Devices Inc. All Rights Reserved.
 * This software is proprietary and confidential to Analog Devices, Inc.
 * and its licensors.
 *
 * This software is subject to the terms and conditions of the license set
 * forth in the project LICENSE file. Downloading, reproducing, distributing or
 * otherwise using the software constitutes acceptance of the license. The
 * software may not be used except as expressly authorized under the license.
 */
#ifndef _browse_cfg_h
#define _browse_cfg_h

ssize_t shell_browse_write(void *usr, const void *buf, size_t len);
ssize_t shell_browse_read(void *usr, const void *buf, size_t len);
ssize_t getline(char **buf, size_t *bufsiz, FILE *fp);

#include "umm_malloc.h"
#define BROWSE_MALLOC  umm_malloc
#define BROWSE_FREE    umm_free
#define BROWSE_REALLOC umm_realloc

#include "util.h"
#define BROWSE_TIME       util_time

/* These must be defined */
#define BROWSE_TERM_WRITE shell_browse_write
#define BROWSE_TERM_READ  shell_browse_read

/* Other config */

#define MAX_PATH_LENGTH         256
#define MAX_FILENAME_LENGTH     128
#define MAX_FILES_IN_DIR        100

#endif

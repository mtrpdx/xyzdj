#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ff.h"
#include "fs_devman.h"
#include "shell.h"
#include "test_util.h"

static int testutil_mkdir(const char *path) {
    FRESULT res = f_mkdir(path);
    if (res == FR_OK) {
        return 0;
    } else {
        return -1;
    }
}

static int testutil_touch(const char *path) {
    FIL fil;
    FRESULT res = f_open(&fil, path, FA_CREATE_NEW | FA_WRITE);
    if (res == FR_OK) {
        f_close(&fil);
        return 0;
    } else {
        return -1;
    }
}

static void testutil_cmd(int argc, char **argv, void *usr) {
    if (argc < 3) {
        printf("Usage: testutil [mkdir|touch] <path>\n");
        return;
    }

    const char *cmd = argv[1];
    const char *path = argv[2];

    if (strcmp(cmd, "mkdir") == 0) {
        if (testutil_mkdir(path) == 0) {
            printf("Directory created: %s\n", path);
        } else {
            printf("Failed to create directory: %s\n", path);
        }
    } else if (strcmp(cmd, "touch") == 0) {
        if (testutil_touch(path) == 0) {
            printf("File created: %s\n", path);
        } else {
            printf("Failed to create file: %s\n", path);
        }
    } else {
        printf("Unknown command: %s\n", cmd);
    }
}

void testutil_register(void) {
    shell_register_command("testutil", testutil_cmd, NULL);
}

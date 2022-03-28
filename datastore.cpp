#include "datastore.h"

// Target board has QSPI Flash
#if !(COMPONENT_QSPIF)
#error [ERROR] Storage unavailable.
#endif

#include "mbed.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <functional>

#define PRIVATE_KEY_LENGTH 20
#define PRIVATE_KEY_PATH "/fs/private_key.txt"
#define RESET_KEY_LENGTH 20
#define RESET_KEY_PATH "/fs/reset_key.txt"
#define LOGS_PATH "/fs/logs.txt"

#define BUFFER_MAX_LEN 10

BlockDevice* bd = BlockDevice::get_default_instance();

#include "LittleFileSystem.h"
LittleFileSystem fs("fs");

void erase() {
    printf("Initializing the block device... ");
    fflush(stdout);
    int err = bd->init();
    printf("%s\n", (err ? "Failed on init" : "OK"));
    if (err) {
        error("error: %s (%d)\n", strerror(-err), err);
    }

    printf("Erasing the block device... ");
    fflush(stdout);
    err = bd->erase(0, bd->size());
    printf("%s\n", (err ? "Failed on erase" : "OK"));
    if (err) {
        error("error: %s (%d)\n", strerror(-err), err);
    }

    printf("Deinitializing the block device... ");
    fflush(stdout);
    err = bd->deinit();
    printf("%s\n", (err ? "Failed on deinit" : "OK"));
    if (err) {
        error("error: %s (%d)\n", strerror(-err), err);
    }
}

int mount_fs(){
    int err = fs.mount(bd);
    printf("%s\n", (err ? "No file system found" : "OK"));
    if (err) {
        // Reformat if we can't mount the filesystem
        printf("Formatting... ");
        fflush(stdout);
        err = fs.reformat(bd);
        printf("%s\n", (err ? "Reformat failed" : "OK"));
        if (err) {
            error("error: %s (%d)\n", strerror(-err), err);
            return -1;
        }
    }
    return 0;
}

int get_private_key(char* buf){
    FILE *f = fopen(PRIVATE_KEY_PATH, "r");
    if (f){
        fgets(buf, PRIVATE_KEY_LENGTH + 1, f);
        fclose (f);
        return 0;
    }
    printf("Cannot open file for read %s: %s\n", PRIVATE_KEY_PATH, strerror(errno));
    return -1;
}

int get_reset_key(char* buf){
    FILE *f = fopen(RESET_KEY_PATH, "r");
    if (f){
        fgets(buf, RESET_KEY_LENGTH + 1, f);
        fclose (f);
        return 0;
    }
    printf("Cannot open file for read %s: %s\n", RESET_KEY_PATH, strerror(errno));
    return -1;
}

int set_private_key(const char* key){
    FILE *f = fopen(PRIVATE_KEY_PATH, "w");
    if (f){
        fprintf(f, "%*s", PRIVATE_KEY_LENGTH + 1, key);
        fclose (f);
        return 0;
    }
    printf("Cannot open file for write %s: %s\n", PRIVATE_KEY_PATH, strerror(errno));
    return -1;
}

int set_reset_key(const char* key){
    FILE *f = fopen(RESET_KEY_PATH, "w");
    if (f){
        fprintf(f, "%*s", RESET_KEY_LENGTH + 1, key);
        fclose (f);
        return 0;
    }
    printf("Cannot open file for write %s: %s\n", RESET_KEY_PATH, strerror(errno));
    return -1;
}

int write_log(const char* log){
    time_t seconds = time(NULL);
    char * formatted_time = (char *)malloc(20);
    struct tm * timeinfo = localtime(&seconds);
    strftime(formatted_time, 20, "%m/%d/%y %H:%M:%S", timeinfo);

    FILE *f = fopen(LOGS_PATH, "a");
    if (f){
        fprintf(f, "[%s] %s\n", formatted_time, log);
        fclose (f);
        return 0;
    }
    printf("Cannot open file for append %s: %s\n", LOGS_PATH, strerror(errno));
    return -1;
}

int print_logs(){
    FILE *f = fopen(LOGS_PATH, "r");
    int c;
    if (f){
        while ((c = getc(f)) != EOF){
            putchar(c);
        }
        fclose(f);
        return 0;
    }
    printf("Cannot open file for read %s: %s\n", LOGS_PATH, strerror(errno));
    return -1;
}
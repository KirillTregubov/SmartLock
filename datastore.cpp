// Target board has QSPI Flash
#if !(COMPONENT_QSPIF)
#error [ERROR] Storage unavailable.
#endif

#include "mbed.h"
#include <stdio.h>
#include <errno.h>
#include <functional>

#define BUFFER_MAX_LEN 10

BlockDevice* bd = BlockDevice::get_default_instance();

#include "LittleFileSystem.h"
LittleFileSystem fs("fs");

void erase() {
    printf("Initializing the block device... ");
    fflush(stdout);
    int err = bd->init();
    printf("%s\n", (err ? "Fail :(" : "OK"));
    if (err) {
        error("error: %s (%d)\n", strerror(-err), err);
    }

    printf("Erasing the block device... ");
    fflush(stdout);
    err = bd->erase(0, bd->size());
    printf("%s\n", (err ? "Fail :(" : "OK"));
    if (err) {
        error("error: %s (%d)\n", strerror(-err), err);
    }

    printf("Deinitializing the block device... ");
    fflush(stdout);
    err = bd->deinit();
    printf("%s\n", (err ? "Fail :(" : "OK"));
    if (err) {
        error("error: %s (%d)\n", strerror(-err), err);
    }
}

void mount(){
    int err = fs.mount(bd);
    printf("%s\n", (err ? "Fail :(" : "OK"));
    if (err) {
        // Reformat if we can't mount the filesystem
        printf("formatting... ");
        fflush(stdout);
        err = fs.reformat(bd);
        printf("%s\n", (err ? "Fail :(" : "OK"));
        if (err) {
            error("error: %s (%d)\n", strerror(-err), err);
        }
    }
}

void read_file_as_string(const char *file_name, char *buffer){
    FILE *f = fopen(file_name, "r+");
    fseek (f, 0, SEEK_END);
    long file_length = ftell (f);
    fseek (f, 0, SEEK_SET);
    buffer = (char *)malloc(file_length);
    if (buffer)
    {
        fread (buffer, 1, file_length, f);
    }
    fclose (f);
}
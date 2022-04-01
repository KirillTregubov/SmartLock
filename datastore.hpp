/**
 * @file datastore.hpp
 * @author Kirill Tregubov (KirillTregubov)
 * @author Philip Cai (Gadnalf)
 * @copyright Copyright (c) 2022 Kirill Tregubov & Philip Cai
 *
 * @brief This header defines functionality for initializing and using a
 * filesystem.
 * @bug No known bugs.
 */
#ifndef DATASTORE_H
#define DATASTORE_H

// Target board has QSPI Flash
#if !(COMPONENT_QSPIF)
#error[ERROR] Storage unavailable.
#endif

#include "LittleFileSystem.h"
#include "mbed.h"
#include <errno.h>
#include <functional>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PRIVATE_KEY_LENGTH 20
#define PRIVATE_KEY_PATH "/fs/private_key.txt"
#define RESET_KEY_LENGTH 20
#define RESET_KEY_PATH "/fs/reset_key.txt"
#define LOGS_PATH "/fs/logs.txt"
#define BUFFER_MAX_LEN 10

int mount_fs();

int get_private_key(char *buf);

int get_reset_key(char *buf);

int set_private_key(const char *key);

int set_reset_key(const char *key);

int write_log(const char *log);

int print_logs();

#endif // DATASTORE_H

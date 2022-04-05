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

#include "mbed.h"
#include "LittleFileSystem.h"
#include "keys.hpp"
#include <errno.h>
#include <functional>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Target board has QSPI Flash
#if !(COMPONENT_QSPIF)
#error[ERROR] Storage unavailable.
#endif

#define PRIVATE_KEY_PATH "/fs/private_key.txt"
#define RECOVERY_KEY_PATH "/fs/recovery_key.txt"
#define LOGS_PATH "/fs/logs.txt"
#define BUFFER_MAX_LEN 10

/**
 * @brief Mounts and initializes the file system.
 * @return 0 upon success, -1 on error / failure.
 */
int mount_fs();

/**
 * @brief Erases the file system.
 * @return 0 upon success, -1 on error / failure.
 */
void erase_fs();

/**
 * @brief Get the private key from memory.
 *
 * @param buf Buffer to store the private key.
 * @return 0 upon success, -1 on error / failure.
 */
int get_private_key(char *buf);

/**
 * @brief Get the reset keys from memory.
 *
 * @param buf Buffer to store the reset key.
 * @return 0 upon success, -1 on error / failure.
 */
int get_recovery_keys(char *buf);

/**
 * @brief Sets the private key in memory.
 *
 * @param key The key string to write.
 * @return 0 upon success, -1 on error / failure.
 */
int set_private_key(const char *key);

/**
 * @brief Sets the reset keys in memory.
 *
 * @param key The key string to write.
 * @return 0 upon success, -1 on error / failure.
 */
int set_recovery_keys(const char *key);

/**
 * @brief Writes a timestamped log to the log file.
 *
 * @param log The log message.
 * @return 0 upon success, -1 on error / failure.
 */
int write_log(const char *log);

/**
 * @brief Prints the entire log file to stdout.
 * @return 0 upon success, -1 on error / failure.
 */
int print_logs();

#endif // DATASTORE_H

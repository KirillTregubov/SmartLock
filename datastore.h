// Header file for datastore.
#ifndef DATASTORE_H
#define DATASTORE_H

int mount_fs();

int get_private_key(char* buf);

int get_reset_key(char* buf);

int set_private_key(const char* key);

int set_reset_key(const char* key);

int write_log(const char* log);

int print_logs();

#endif
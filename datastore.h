// Header file for datastore.
#ifndef DATASTORE
#define DATASTORE

void mount();

int get_private_key(char* buf);

int get_reset_key(char* buf);

int set_private_key(const char* key);

int set_reset_key(const char* key);

#endif
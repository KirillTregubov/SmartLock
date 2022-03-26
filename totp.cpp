#include <mbed.h>
#include <cstdint>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define SHA1_DIGEST_LENGTH 20

// Just in case
void manual_HMAC(uint8_t *secret, uint8_t *counter, uint8_t *digest){
	//H((secret xor ipad) + counter)
	uint8_t i_key[PSA_HASH_MAX_SIZE] = {0};
	for (int i = 0; i < 10; i++){
		i_key[i] = secret[i] ^ 0x36;
	}
	for (int i = 10; i < PSA_HASH_MAX_SIZE; i++){
		i_key[i] = 0x36;
	}
	//printf("first 12 bytes of ikey: %12x\n", *i_key);

    // The following four are used for both hash operations
    psa_status_t status;
    psa_algorithm_t alg = PSA_ALG_SHA_1;
    psa_hash_operation_t operation = PSA_HASH_OPERATION_INIT;
    size_t actual_hash_len;

	uint8_t i_sha[SHA1_DIGEST_LENGTH];
    
    /* Clean up hash operation context prior to init, just in case */
    psa_hash_abort(&operation);
    /* Compute hash of inner message  */
    status = psa_hash_setup(&operation, alg);
    if (status != PSA_SUCCESS) {
        printf("Failed to begin inner hash operation\n");
        return;
    }
    status = psa_hash_update(&operation, i_key, PSA_HASH_MAX_SIZE);
    if (status != PSA_SUCCESS) {
        printf("Failed to update inner hash operation (1)\n");
        return;
    }
    status = psa_hash_update(&operation, counter, 8);
    if (status != PSA_SUCCESS) {
        printf("Failed to update inner hash operation (2)\n");
        return;
    }
    status = psa_hash_finish(&operation, i_sha, SHA1_DIGEST_LENGTH,
                             &actual_hash_len);
    if (status != PSA_SUCCESS) {
        printf("Failed to finish inner hash operation\n");
        return;
    }
    /* Clean up hash operation context */
    psa_hash_abort(&operation);

	//printf("sha1: %20x\n", *i_sha);

	//HMAC = H[(secret xor opad) + H((secret xor ipad) + counter)];
	uint8_t o_key[PSA_HASH_MAX_SIZE] = {0};
	for (int i = 0; i < 10; i++){
		o_key[i] = secret[i] ^ 0x5c;
	}
	for (int i = 10; i < PSA_HASH_MAX_SIZE; i++){
		o_key[i] = 0x5c;
	}
	//printf("first 12 bytes of okey: %12x\n", *o_key);

    /* Compute hash of outer message  */
    status = psa_hash_setup(&operation, alg);
    if (status != PSA_SUCCESS) {
        printf("Failed to begin outer hash operation\n");
        return;
    }
    status = psa_hash_update(&operation, o_key, PSA_HASH_MAX_SIZE);
    if (status != PSA_SUCCESS) {
        printf("Failed to update outer hash operation (1)\n");
        return;
    }
    status = psa_hash_update(&operation,  i_sha, SHA1_DIGEST_LENGTH);
    if (status != PSA_SUCCESS) {
        printf("Failed to update outer hash operation (2)\n");
        return;
    }
    status = psa_hash_finish(&operation, digest, SHA1_DIGEST_LENGTH,
                             &actual_hash_len);
    if (status != PSA_SUCCESS) {
        printf("Failed to finish outer hash operation\n");
        return;
    }
    /* Clean up hash operation context */
    psa_hash_abort(&operation);

	return;
}

void psa_HMAC(uint8_t *secret, uint8_t *counter, uint8_t *digest){
    // Coming soon
}

int DT(uint8_t * hmac_result){
	// take the lowest 4 bits of hmac
	uint8_t offset = hmac_result[19] & 0xf;
	// idk why they just labelled this one p in the docs
	int p = (hmac_result[offset]  & 0x7f) << 24
           | (hmac_result[offset+1] & 0xff) << 16
           | (hmac_result[offset+2] & 0xff) <<  8
           | (hmac_result[offset+3] & 0xff) ;

    return p;   
}

int validateTOTP(const char * secret_hex, char * TOTP_string, time_t unix_time){
	// t0 = 0, timestep = 30
	long counter = unix_time / 30;
	uint8_t* counter_bytes = (uint8_t *)malloc(8);
	counter_bytes[0] = (counter >> 56) & 0xFF;
	counter_bytes[1] = (counter >> 48) & 0xFF;
	counter_bytes[2] = (counter >> 40) & 0xFF;
	counter_bytes[3] = (counter >> 32) & 0xFF;
	counter_bytes[4] = (counter >> 24) & 0xFF;
	counter_bytes[5] = (counter >> 16) & 0xFF;
	counter_bytes[6] = (counter >> 8) & 0xFF;
	counter_bytes[7] = counter & 0xFF;

	//printf("counter: %08x\n", *counter_bytes);

	// convert to binary (byte array)
	uint8_t* secret_bytes = (uint8_t*) malloc(10);
	int pos = 0;
	for (int i = 0; i < 10; i++) {
        sscanf(secret_hex + pos, "%02hhx", &secret_bytes[i]);
        pos += 2;
    }
	//printf("secret: %10x\n", *secret_bytes);

	uint8_t* hmac_out = (uint8_t*) malloc(SHA1_DIGEST_LENGTH);

	manual_HMAC(secret_bytes, counter_bytes, hmac_out);
	//printf("hmac: %20x\n", *hmac_out);
	int TOTP = DT(hmac_out) % 1000000;
	int TOTP_input = atoi(TOTP_string);
	printf("Calculated TOTP Value: %d\n", TOTP);

	return TOTP == TOTP_input;
}
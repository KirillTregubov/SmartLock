#include <mbed.h>
#include <crypto.h>
#include <cstdint>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define SHA1_DIGEST_LENGTH 20

// Just in case
int manual_HMAC(uint8_t *secret, uint8_t *counter, uint8_t *digest){
	//H((secret xor ipad) + counter)
	uint8_t i_key[PSA_HASH_MAX_SIZE] = {0};
	for (int i = 0; i < 10; i++){
		i_key[i] = secret[i] ^ 0x36;
	}
	for (int i = 10; i < PSA_HASH_MAX_SIZE; i++){
		i_key[i] = 0x36;
	}

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
        return -1;
    }
    status = psa_hash_update(&operation, i_key, PSA_HASH_MAX_SIZE);
    if (status != PSA_SUCCESS) {
        printf("Failed to update inner hash operation (1)\n");
        return -1;
    }
    status = psa_hash_update(&operation, counter, 8);
    if (status != PSA_SUCCESS) {
        printf("Failed to update inner hash operation (2)\n");
        return -1;
    }
    status = psa_hash_finish(&operation, i_sha, SHA1_DIGEST_LENGTH,
                             &actual_hash_len);
    if (status != PSA_SUCCESS) {
        printf("Failed to finish inner hash operation\n");
        return -1;
    }
    /* Clean up hash operation context */
    psa_hash_abort(&operation);

	//HMAC = H[(secret xor opad) + H((secret xor ipad) + counter)];
	uint8_t o_key[PSA_HASH_MAX_SIZE] = {0};
	for (int i = 0; i < 10; i++){
		o_key[i] = secret[i] ^ 0x5c;
	}
	for (int i = 10; i < PSA_HASH_MAX_SIZE; i++){
		o_key[i] = 0x5c;
	}

    /* Compute hash of outer message  */
    status = psa_hash_setup(&operation, alg);
    if (status != PSA_SUCCESS) {
        printf("Failed to begin outer hash operation\n");
        return -1;
    }
    status = psa_hash_update(&operation, o_key, PSA_HASH_MAX_SIZE);
    if (status != PSA_SUCCESS) {
        printf("Failed to update outer hash operation (1)\n");
        return -1;
    }
    status = psa_hash_update(&operation,  i_sha, SHA1_DIGEST_LENGTH);
    if (status != PSA_SUCCESS) {
        printf("Failed to update outer hash operation (2)\n");
        return -1;
    }
    status = psa_hash_finish(&operation, digest, SHA1_DIGEST_LENGTH,
                             &actual_hash_len);
    if (status != PSA_SUCCESS) {
        printf("Failed to finish outer hash operation\n");
        return -1;
    }
    /* Clean up hash operation context */
    psa_hash_abort(&operation);

	return 0;
}

// Would use this but this inexplicably fails to compile with message: Undefined symbol psa_mac_compute (referred from BUILD/DISCO_L475VG_IOT01A/ARMC6/totp.o).
// int psa_HMAC(mbedtls_svc_key_id_t key_id, uint8_t *counter, uint8_t *digest){
//     psa_status_t status;
//     psa_algorithm_t alg = PSA_ALG_HMAC(PSA_ALG_SHA_1);
//     size_t actual_hash_len;
//     status = psa_mac_compute(key_id, alg, counter, 8, digest, SHA1_DIGEST_LENGTH, &actual_hash_len);
//     if (status != PSA_SUCCESS) {
//         printf("Failed to compute HMAC\n");
//         return -1;
//     }
//     return 0;
// }

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

int validate_for_time(const char * secret_hex, const char * TOTP_string, time_t unix_time){
	// t0 = 0, timestep = 30
	unsigned long counter = unix_time / 30;
	uint8_t* counter_bytes = (uint8_t *)malloc(8);
	for (int i = 7; i>=0; i--) {
		counter_bytes[i] = counter;
		counter >>= 8;
	}

	// convert to binary (byte array)
	uint8_t* secret_bytes = (uint8_t*) malloc(10);
	int pos = 0;
	for (int i = 0; i < 10; i++) {
        sscanf(secret_hex + pos, "%02hhx", &secret_bytes[i]);
        pos += 2;
    }

	uint8_t* hmac_out = (uint8_t*) malloc(SHA1_DIGEST_LENGTH);

	manual_HMAC(secret_bytes, counter_bytes, hmac_out);
	int TOTP = DT(hmac_out) % 1000000;
	int TOTP_input = atoi(TOTP_string);
	printf("Calculated TOTP Value: %d\n", TOTP);

	return TOTP == TOTP_input;
}

int validate(const char *secret_hex, const char *TOTP_string){
    time_t current_time = time(NULL);
    return validate_for_time(secret_hex, TOTP_string, current_time) |
    validate_for_time(secret_hex, TOTP_string, current_time + 30) |
    validate_for_time(secret_hex, TOTP_string, current_time - 30);
}
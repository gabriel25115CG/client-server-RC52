#include "access_code.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

static char current_code[CODE_LEN + 1] = "000000";
static time_t expiry = 0;

void generate_default_code(char *out_code) {
    if (!out_code) return;
    unsigned int seed = (unsigned int)time(NULL) ^ (unsigned int)getpid();
    srand(seed);
    for (int i = 0; i < CODE_LEN; ++i) {
        out_code[i] = '0' + (rand() % 10);
    }
    out_code[CODE_LEN] = '\0';
}

int set_code(const char *new_code, time_t duration_seconds) {
    if (!new_code || strlen(new_code) != CODE_LEN) return -1;
    for (int i = 0; i < CODE_LEN; ++i) if (new_code[i] < '0' || new_code[i] > '9') return -1;

    strncpy(current_code, new_code, CODE_LEN + 1);
    if (duration_seconds > 0) {
        expiry = time(NULL) + duration_seconds;
    } else {
        expiry = 0; // pas d'expiration
    }
    return 0;
}

int validate_code(const char *code) {
    if (!code) return 0;
    if (strlen(code) != CODE_LEN) return 0;
    return (strncmp(code, current_code, CODE_LEN) == 0) ? 1 : 0;
}

const char* get_current_code(void) {
    return current_code;
}

time_t get_code_expiry(void) {
    return expiry;
}

void check_and_rotate_code_if_expired(void) {
    if (expiry == 0) return; // pas d'expiration
    time_t now = time(NULL);
    if (now >= expiry) {
        char newcode[CODE_LEN+1];
        generate_default_code(newcode);
        set_code(newcode, 0); // nouveau code sans durée par défaut
        // expiry déjà remis à 0 dans set_code
        printf("[access_code] Code expired -> rotated to %s\n", newcode);
    }
}

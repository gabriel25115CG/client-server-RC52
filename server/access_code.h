#ifndef ACCESS_CODE_H
#define ACCESS_CODE_H

#include <time.h>

#define CODE_LEN 6

void generate_default_code(char *out_code); // remplit out_code (taille CODE_LEN+1)
int set_code(const char *new_code, time_t duration_seconds); // retourne 0 ok, -1 erreur
int validate_code(const char *code); // 1 valide, 0 invalide
const char* get_current_code(void); // pointeur vers la chaîne (read-only)
time_t get_code_expiry(void);
void check_and_rotate_code_if_expired(void); // génère nouveau code si expiré

#endif // ACCESS_CODE_H

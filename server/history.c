#include "history.h"
#include <stdio.h>
#include <time.h>

void append_history(const char *filename, const char *ip, int port, const char *attempted_code, const char *result) {
    FILE *f = fopen(filename, "a");
    if (!f) return;
    time_t now = time(NULL);
    struct tm tm = *localtime(&now);
    fprintf(f, "%04d-%02d-%02d %02d:%02d:%02d | %s:%d | code=%s | %s\n",
            tm.tm_year+1900, tm.tm_mon+1, tm.tm_mday,
            tm.tm_hour, tm.tm_min, tm.tm_sec,
            ip, port, attempted_code ? attempted_code : "-", result);
    fclose(f);
}

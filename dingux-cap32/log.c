#include "log.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void log(const char* tag, const char* message) {
   time_t now;
   time(&now);
   printf("%s [%s]: %s\n", ctime(&now), tag, message);
}
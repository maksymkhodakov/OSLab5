#include "g.h"
#include <unistd.h>

int g(int x) {
    sleep(3);
    return x;
}
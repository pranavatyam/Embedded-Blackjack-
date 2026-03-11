#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"


int main() {
    stdio_init_all();

    init_adc();

    for(;;) {
        joystick();
    }
}

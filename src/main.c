#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"


int main() {
    stdio_init_all();

    init_adc();

    char input = 0; 
    // R:right, L:left, U:up, D:down
    for(;;) {
        input = joystick();
        printf("User selected: %c\n", input);
        sleep_ms(250);
    }
}

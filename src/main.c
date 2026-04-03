#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"


int main() {
    stdio_init_all();

    init_adc();

    int input = 0; 
    // 1:right, 2:left, 3:up, 4:down
    for(;;) {
        input = joystick();
        //printf("User inputted %d\n", input);
        sleep_ms(250);
    }
}

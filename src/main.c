#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"

void init_adc();

void init_adc() {
    gpio_init(45);
    adc_init();
    adc_gpio_init(45);
    adc_select_input(5);
}

int main() {
    stdio_init_all();

    init_adc();

    for(;;) {
        printf("ADC Result: %d     \r", read_adc());

        fflush(stdout);
        sleep_ms(250);
    }
}

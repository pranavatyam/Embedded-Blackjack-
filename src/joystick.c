#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"

void init_adc();
void joystick();
uint16_t read_adc();

void init_adc() {
    gpio_init(45);
    adc_init();
    adc_gpio_init(45);
    adc_select_input(5);
}

uint16_t read_adc() {
    return(adc_read());
}

void joystick() {
    printf("ADC Result: %d     \r", read_adc());
    fflush(stdout);
    sleep_ms(250);
}

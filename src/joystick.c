#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"

void init_adc();
int joystick();
uint16_t read_adc();

void init_adc() {
    gpio_init(45);
    gpio_init(40);
    adc_init();
    adc_gpio_init(45);
    adc_gpio_init(40);
    adc_select_input(5);
    adc_select_input(0);
}

uint16_t read_adc() {
    return(adc_read());
}

int joystick() {
    uint16_t adc_out = read_adc();
    return adc_out;
    printf("ADC Result: %d     \r", read_adc());
    fflush(stdout);
    
}

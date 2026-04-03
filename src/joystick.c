#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"

void init_adc();
char joystick();
uint16_t read_adc();

uint16_t vertical;
uint16_t horizontal;

void init_adc() {
    gpio_init(45);
    adc_init();
    adc_gpio_init(45);
    //adc_set_round_robin(33u);
}

uint16_t read_adc() {
    return(adc_read());
}

char joystick() {
    char input = '0';
    while(input == '0') {
        adc_select_input(5);
        vertical = read_adc();
        
        adc_select_input(0);
        horizontal = read_adc();

        //vertical <= 50
        //vertical >= 4095

        //horizontal <=50
        //horizontal >= 4095
        // 1:right, 2:left, 3:up, 4:down
        if(horizontal >= 4095) {
            input = 'R';
        } else if(horizontal <=50) {
            input = 'L';
        }

        if(vertical >= 4095) {
            input = 'U';
        } else if(vertical <= 50) {
            input = 'D';
        }
    }

    return(input);
    //printf("ADC Result V: %d, H: %d, in: %c     \r", vertical, horizontal, input);
    //fflush(stdout);
}

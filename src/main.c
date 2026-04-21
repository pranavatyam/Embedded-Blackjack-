#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"
#include "test_loop.h"
#include "game_logic.h"


int main(void) {
    stdio_init_all();
    sleep_ms(1500);

    Screen_Init();   // initialize TFT first
    BJ_Init();       // safe to show start screen now

    serial_test_loop();

    while (1) {
        sleep_ms(1000);
    }

    return 0;
}
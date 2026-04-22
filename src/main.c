#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"
#include "test_loop.h"
#include "game_logic.h"

// forward declarations for audio functions in pwm_speaker.c
void init_pwm_audio(void);
bool audio_is_playing(void);
void HAL_Audio_PlayWin(void);
void HAL_Audio_PlayLoss(void);
void HAL_Audio_PlayBlackjack(void);
void HAL_Audio_PlayBust(void);
void HAL_Audio_PlayPush(void);

int main(void) {
    stdio_init_all();
    sleep_ms(1500);

    leds_init();          // initialize LEDs
    init_pwm_audio();     // initialize audio
    Screen_Init();
    BJ_Init();

    serial_test_loop();

    while (1) {
        sleep_ms(1000);
    }

    return 0;
}
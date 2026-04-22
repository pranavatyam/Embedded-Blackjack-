#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"

// forward declarations for audio functions in pwm_speaker.c
void init_pwm_audio(void);
bool audio_is_playing(void);
void HAL_Audio_PlayWin(void);
void HAL_Audio_PlayLoss(void);
void HAL_Audio_PlayBlackjack(void);
void HAL_Audio_PlayBust(void);
void HAL_Audio_PlayPush(void);

int main() {
    stdio_init_all();

    init_adc();
    init_pwm_audio();   // initialize audio

    // --- Audio test: cycle through all sounds once at startup ---
    printf("Testing WIN...\n");
    HAL_Audio_PlayWin();
    while (audio_is_playing()) tight_loop_contents();
    sleep_ms(1000);

    printf("Testing BLACKJACK...\n");
    HAL_Audio_PlayBlackjack();
    while (audio_is_playing()) tight_loop_contents();
    sleep_ms(1000);

    printf("Testing LOSS...\n");
    HAL_Audio_PlayLoss();
    while (audio_is_playing()) tight_loop_contents();
    sleep_ms(1000);

    printf("Testing BUST...\n");
    HAL_Audio_PlayBust();
    while (audio_is_playing()) tight_loop_contents();
    sleep_ms(1000);

    printf("Testing PUSH...\n");
    HAL_Audio_PlayPush();
    while (audio_is_playing()) tight_loop_contents();
    sleep_ms(1000);

    printf("Audio test done. Starting joystick loop...\n");
    // --- End audio test ---

    char input = 0; 
    // R:right, L:left, U:up, D:down
    for(;;) {
        input = joystick();
        printf("User selected: %c\n", input);
        sleep_ms(250);
    }
}

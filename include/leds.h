#ifndef LEDS_H
#define LEDS_H

#include <stdbool.h>
#include <stdint.h>

#define LED_COUNT 72

void leds_init(void);
void leds_show(void);
void leds_clear(void);

void leds_set_pixel_rgb(int index, uint8_t r, uint8_t g, uint8_t b);
void leds_set_all_rgb(uint8_t r, uint8_t g, uint8_t b);
void leds_fill_count_rgb(uint8_t r, uint8_t g, uint8_t b, int count);

void leds_show_win_frame(uint8_t step);
void leds_show_loss_frame(bool on);

#endif
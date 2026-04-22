#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "ws2812.pio.h"
#include "leds.h"

#define WS2812_PIN 20
#define WIN_LEVEL  40
#define LOSS_LEVEL 48

static PIO pio = pio0;
static uint sm = 0;
static uint offset;
static bool leds_started = false;

static uint32_t pixels[LED_COUNT];

static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b) {
    return ((uint32_t)g << 16) | ((uint32_t)r << 8) | (uint32_t)b;
}

static inline void put_pixel(uint32_t pixel_grb) {
    pio_sm_put_blocking(pio, sm, pixel_grb << 8u);
}

static inline uint8_t scale_byte(uint8_t value, uint8_t level) {
    return (uint8_t)(((uint16_t)value * level) / 255u);
}

static uint32_t wheel_color(uint8_t pos) {
    uint8_t r;
    uint8_t g;
    uint8_t b;

    if (pos < 85) {
        r = 255 - pos * 3;
        g = pos * 3;
        b = 0;
    } else if (pos < 170) {
        pos -= 85;
        r = 0;
        g = 255 - pos * 3;
        b = pos * 3;
    } else {
        pos -= 170;
        r = pos * 3;
        g = 0;
        b = 255 - pos * 3;
    }

    r = scale_byte(r, WIN_LEVEL);
    g = scale_byte(g, WIN_LEVEL);
    b = scale_byte(b, WIN_LEVEL);

    return urgb_u32(r, g, b);
}

void leds_show(void) {
    if (!leds_started) {
        return;
    }

    for (int i = 0; i < LED_COUNT; i++) {
        put_pixel(pixels[i]);
    }

    sleep_us(80);
}

void leds_clear(void) {
    for (int i = 0; i < LED_COUNT; i++) {
        pixels[i] = 0;
    }

    leds_show();
}

void leds_init(void) {
    if (leds_started) {
        return;
    }

    offset = pio_add_program(pio, &ws2812_program);
    ws2812_program_init(pio, sm, offset, WS2812_PIN);

    leds_started = true;
    leds_clear();
}

void leds_set_pixel_rgb(int index, uint8_t r, uint8_t g, uint8_t b) {
    if (index < 0 || index >= LED_COUNT) {
        return;
    }

    pixels[index] = urgb_u32(r, g, b);
}

void leds_set_all_rgb(uint8_t r, uint8_t g, uint8_t b) {
    uint32_t color = urgb_u32(r, g, b);

    for (int i = 0; i < LED_COUNT; i++) {
        pixels[i] = color;
    }

    leds_show();
}

void leds_fill_count_rgb(uint8_t r, uint8_t g, uint8_t b, int count) {
    uint32_t color = urgb_u32(r, g, b);

    if (count < 0) {
        count = 0;
    }

    if (count > LED_COUNT) {
        count = LED_COUNT;
    }

    for (int i = 0; i < LED_COUNT; i++) {
        if (i < count) {
            pixels[i] = color;
        } else {
            pixels[i] = 0;
        }
    }

    leds_show();
}

void leds_show_win_frame(uint8_t step) {
    for (int i = 0; i < LED_COUNT; i++) {
        uint8_t color_index = (uint8_t)((i * 256 / LED_COUNT) + step);
        pixels[i] = wheel_color(color_index);
    }

    leds_show();
}

void leds_show_loss_frame(bool on) {
    uint32_t color = on ? urgb_u32(LOSS_LEVEL, 0, 0) : 0;

    for (int i = 0; i < LED_COUNT; i++) {
        pixels[i] = color;
    }

    leds_show();
}
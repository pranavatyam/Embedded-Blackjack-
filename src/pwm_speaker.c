/*
 * blackjack_audio.c
 *
 * PWM audio for the Blackjack game on RP2350.
 * Written to match the lab skeleton style:
 *   - Pico SDK (hardware/pwm.h, hardware/irq.h)
 *   - IRQ handler on PWM_IRQ_WRAP_0
 *   - Square-wave tone generation (no wavetable needed for simple beeps)
 *
 * GPIO 15 → PWM slice 7, channel A
 * Passive buzzer between GPIO 15 and GND.
 *
 * To test audio standalone (without display/buttons/game logic),
 * compile with -DTEST_AUDIO and this file becomes a self-contained
 * main that cycles through every sound effect.
 *
 * How it works:
 *   init_pwm_audio() sets up the slice at ~20 kHz wrap so the IRQ
 *   fires 20,000x per second. Each call we count IRQ ticks to time
 *   note duration, and toggle the compare level to generate a square
 *   wave — same pattern as your lab's pwm_audio_handler.
 */

#include <stdint.h>
#include <stdbool.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/irq.h"

/* ── Pin & timing constants ─────────────────────────────────────────── */

#define AUDIO_GPIO   15u
#define CLK_DIV      1.0f
#define WRAP_VALUE   7499u    /* 150 MHz / 1 / 7500 = 20 000 Hz IRQ rate */
#define IRQ_RATE     20000u

/* ── Note definitions ───────────────────────────────────────────────── */

#define NOTE_REST 0u

#define N_C4  262u
#define N_D4  294u
#define N_E4  330u
#define N_F4  349u
#define N_G4  392u
#define N_A4  440u
#define N_B4  494u
#define N_C5  523u
#define N_E5  659u

typedef struct {
    uint16_t freq_hz;       /* 0 = rest/silence                  */
    uint16_t duration_ms;   /* 0 = end-of-sequence sentinel       */
} Note;

/* ── Sound effect sequences ─────────────────────────────────────────── */

static const Note SFX_WIN[] = {
    { N_C4, 100 }, { N_E4, 100 }, { N_G4, 100 },
    { N_C5, 300 }, { NOTE_REST, 50 },
    { N_E5, 250 },
    { 0, 0 }
};

static const Note SFX_BLACKJACK[] = {
    { N_G4,  80 }, { N_G4,  80 }, { N_G4,  80 },
    { N_E4, 240 }, { NOTE_REST, 60 },
    { N_F4,  80 }, { N_F4,  80 }, { N_F4,  80 },
    { N_D4, 240 }, { NOTE_REST, 60 },
    { N_G4,  60 }, { N_A4,  60 }, { N_B4,  60 },
    { N_C5, 400 },
    { 0, 0 }
};

static const Note SFX_LOSS[] = {
    { N_E4, 200 }, { N_D4, 200 }, { N_C4, 400 },
    { 0, 0 }
};

static const Note SFX_BUST[] = {
    { N_A4, 80 }, { N_G4, 80 }, { N_E4, 80 }, { N_C4, 200 },
    { 0, 0 }
};

static const Note SFX_PUSH[] = {
    { N_G4, 150 }, { NOTE_REST, 50 }, { N_G4, 150 },
    { 0, 0 }
};

/* ── Sequencer state ─────────────────────────────────────────────────── */

static const Note   *seq_notes    = NULL;
static uint16_t      seq_index    = 0;
static volatile bool seq_active   = false;
static uint32_t      ticks_left   = 0;
static uint32_t      toggle_ticks = 0;
static uint32_t      toggle_count = 0;
static bool          wave_high    = false;

/* ── Internal: load the note at seq_index ───────────────────────────── */

static void load_note(uint16_t index) {
    const Note *n = &seq_notes[index];
    uint slice = pwm_gpio_to_slice_num(AUDIO_GPIO);

    /* Sentinel: duration 0 means sequence is done */
    if (n->duration_ms == 0) {
        seq_active = false;
        pwm_set_chan_level(slice, pwm_gpio_to_channel(AUDIO_GPIO), 0);
        return;
    }

    ticks_left = (uint32_t)n->duration_ms * IRQ_RATE / 1000u;
    if (ticks_left == 0) ticks_left = 1;

    if (n->freq_hz == NOTE_REST) {
        toggle_ticks = 0;
        toggle_count = 0;
        wave_high    = false;
        pwm_set_chan_level(slice, pwm_gpio_to_channel(AUDIO_GPIO), 0);
    } else {
        /* Half-period in IRQ ticks = IRQ_RATE / (2 * freq) */
        toggle_ticks = IRQ_RATE / (2u * n->freq_hz);
        if (toggle_ticks == 0) toggle_ticks = 1;
        toggle_count = 0;
        wave_high    = true;
        /* Start high: compare = half wrap → 50% duty cycle */
        pwm_set_chan_level(slice, pwm_gpio_to_channel(AUDIO_GPIO),
                           (WRAP_VALUE + 1u) / 2u);
    }
}

/* ── IRQ handler — same structure as your pwm_audio_handler ─────────── */

void blackjack_audio_handler(void) {
    uint slice = pwm_gpio_to_slice_num(AUDIO_GPIO);
    pwm_clear_irq(slice);   /* always first, same as your lab */

    if (!seq_active || seq_notes == NULL) return;

    /* Toggle square wave output */
    if (seq_notes[seq_index].freq_hz != NOTE_REST) {
        toggle_count++;
        if (toggle_count >= toggle_ticks) {
            toggle_count = 0;
            wave_high = !wave_high;
            uint32_t level = wave_high ? (WRAP_VALUE + 1u) / 2u : 0u;
            pwm_set_chan_level(slice, pwm_gpio_to_channel(AUDIO_GPIO), level);
        }
    }

    /* Count down the note duration */
    if (ticks_left > 0) ticks_left--;
    if (ticks_left == 0) {
        seq_index++;
        load_note(seq_index);
    }
}

/* ── init_pwm_audio — mirrors your lab's init exactly ───────────────── */

void init_pwm_audio(void) {
    gpio_set_function(AUDIO_GPIO, GPIO_FUNC_PWM);
    uint slice = pwm_gpio_to_slice_num(AUDIO_GPIO);
    pwm_set_clkdiv(slice, CLK_DIV);
    pwm_set_wrap(slice, WRAP_VALUE);
    pwm_set_chan_level(slice, pwm_gpio_to_channel(AUDIO_GPIO), 0);
    pwm_set_enabled(slice, true);
    pwm_set_irq_enabled(slice, true);
    irq_set_exclusive_handler(PWM_IRQ_WRAP_0, blackjack_audio_handler);
    irq_set_enabled(PWM_IRQ_WRAP_0, true);
}

/* ── Public API ──────────────────────────────────────────────────────── */

static void play_sequence(const Note *notes) {
    if (notes == NULL) return;
    irq_set_enabled(PWM_IRQ_WRAP_0, false);   /* guard shared state */
    seq_notes  = notes;
    seq_index  = 0;
    seq_active = true;
    load_note(0);
    irq_set_enabled(PWM_IRQ_WRAP_0, true);
}

void audio_stop(void) {
    irq_set_enabled(PWM_IRQ_WRAP_0, false);
    seq_active = false;
    seq_notes  = NULL;
    pwm_set_chan_level(pwm_gpio_to_slice_num(AUDIO_GPIO),
                       pwm_gpio_to_channel(AUDIO_GPIO), 0);
    irq_set_enabled(PWM_IRQ_WRAP_0, true);
}

bool audio_is_playing(void) { return seq_active; }

/* HAL bridge — drop-in replacements for the stubs in hal_rp2350.c */
void HAL_Audio_PlayWin(void)       { play_sequence(SFX_WIN);       }
void HAL_Audio_PlayLoss(void)      { play_sequence(SFX_LOSS);      }
void HAL_Audio_PlayBlackjack(void) { play_sequence(SFX_BLACKJACK); }
void HAL_Audio_PlayBust(void)      { play_sequence(SFX_BUST);      }
void HAL_Audio_PlayPush(void)      { play_sequence(SFX_PUSH);      }

/* ══════════════════════════════════════════════════════════════════════
   STANDALONE AUDIO TEST
   ──────────────────────────────────────────────────────────────────────
   Compile with -DTEST_AUDIO to get a self-contained main().
   You only need this file + the Pico SDK — no game logic, no display.

   In your CMakeLists.txt, add a separate target like:

       add_executable(test_audio blackjack_audio.c)
       target_compile_definitions(test_audio PRIVATE TEST_AUDIO)
       target_link_libraries(test_audio pico_stdlib hardware_pwm hardware_irq)
       pico_add_extra_outputs(test_audio)

   Flash test_audio.uf2 and open a serial terminal to see which
   effect is playing. Each effect plays once, then there's a 1-second
   gap before the next one. The whole cycle repeats forever.
   ══════════════════════════════════════════════════════════════════════ */
#ifdef TEST_AUDIO

typedef struct {
    void       (*play_fn)(void);
    const char  *name;
} SfxEntry;

static const SfxEntry sfx_list[] = {
    { HAL_Audio_PlayWin,       "WIN"       },
    { HAL_Audio_PlayBlackjack, "BLACKJACK" },
    { HAL_Audio_PlayLoss,      "LOSS"      },
    { HAL_Audio_PlayBust,      "BUST"      },
    { HAL_Audio_PlayPush,      "PUSH"      },
};
#define NUM_SFX (sizeof(sfx_list) / sizeof(sfx_list[0]))

int main(void) {
    stdio_init_all();
    init_pwm_audio();

    printf("=== Blackjack audio test ===\n");

    for (;;) {
        for (int i = 0; i < (int)NUM_SFX; i++) {
            printf("Playing: %s\n", sfx_list[i].name);
            sfx_list[i].play_fn();

            /* Spin until the IRQ handler finishes the sequence */
            while (audio_is_playing()) {
                tight_loop_contents();
            }

            sleep_ms(1000);   /* 1 s gap so each sound is clearly distinct */
        }

        printf("Cycle done, restarting...\n");
        sleep_ms(2000);
    }
}

#endif /* TEST_AUDIO */
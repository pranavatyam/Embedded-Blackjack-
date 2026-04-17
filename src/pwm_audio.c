// #include <stdio.h>
// #include <math.h>
// #include <stdlib.h>
// #include "pico/stdlib.h"
// #include "hardware/pwm.h"
// #include "hardware/irq.h"
// #include "queue.h"
// #include "support.h"

// static int duty_cycle = 100;
// static int dir = 1;
// static int color = 0;


// void init_pwm_static(uint32_t period, uint32_t duty_cycle) {
//     // fill in
//     gpio_set_function(37, GPIO_FUNC_PWM);
//     gpio_set_function(38, GPIO_FUNC_PWM);
//     gpio_set_function(39, GPIO_FUNC_PWM);

//     uint slice_num1 = pwm_gpio_to_slice_num(37);
//     pwm_set_clkdiv(slice_num1, 150.0);
//     uint slice_num2 = pwm_gpio_to_slice_num(38);
//     pwm_set_clkdiv(slice_num2, 150.0);
//     uint slice_num3 = pwm_gpio_to_slice_num(39);
//     pwm_set_clkdiv(slice_num3, 150.0);

//     pwm_set_wrap(slice_num1, period - 1);
//     pwm_set_wrap(slice_num2, period - 1);
//     pwm_set_wrap(slice_num3, period - 1);

//     pwm_set_chan_level(slice_num1, pwm_gpio_to_channel(37), duty_cycle);
//     pwm_set_chan_level(slice_num2, pwm_gpio_to_channel(38), duty_cycle);
//     pwm_set_chan_level(slice_num3, pwm_gpio_to_channel(39), duty_cycle);

//     pwm_set_enabled(slice_num1, true);
//     pwm_set_enabled(slice_num2, true);
//     pwm_set_enabled(slice_num3, true);

// }

// void pwm_breathing() {
//     // fill in
//     uint slice_num = pwm_gpio_to_slice_num(37);
//     pwm_clear_irq(slice_num);

//         if (dir == 0 && duty_cycle == 100) {
//             color = (color + 1) % 3;
//         }

//         if (duty_cycle == 100 && dir == 0) {
//             dir = 1;
//         } else if (duty_cycle == 0 && dir == 1) {
//             dir = 0;
//         }

//         if (dir == 0) {
//             duty_cycle++;
//         } else {
//             duty_cycle--;
//         }

//         uint32_t new_color = 37 + color;
//         uint32_t new_slice_num = pwm_gpio_to_slice_num(new_color);
//         uint32_t new_current_period = pwm_hw->slice[slice_num].top;
//         uint32_t channel = pwm_gpio_to_channel(new_color);
//         uint32_t new_duty_cycle = new_current_period * duty_cycle / 100;
//         pwm_set_chan_level(new_slice_num, channel, new_duty_cycle);

// }

// void init_pwm_irq() {
//     // fill in

//     uint slice_num = pwm_gpio_to_slice_num(37);
//     pwm_set_irq_enabled(slice_num, true);

//     irq_set_exclusive_handler(PWM_IRQ_WRAP_0, pwm_breathing);
//     irq_set_enabled(PWM_IRQ_WRAP_0, true);

//     uint32_t current_period = pwm_hw->slice[slice_num].top;

//     pwm_set_chan_level(slice_num, pwm_gpio_to_channel(37), current_period);
//     pwm_set_chan_level(slice_num, pwm_gpio_to_channel(38), current_period);
//     pwm_set_chan_level(slice_num, pwm_gpio_to_channel(39), current_period);

// }

// void pwm_audio_handler() {
//     // fill in
//     uint slice_num = pwm_gpio_to_slice_num(36);
//     pwm_clear_irq(slice_num);
//     offset0 += step0;
//     offset1 += step1;
//     if (offset0 >= (N << 16)) {
//         offset0 -= (N << 16);
//     }
//     if (offset1 >= (N << 16)) {
//         offset1 -= (N << 16);
//     }
//     int samp = wavetable[offset0 >> 16] + wavetable[offset1 >> 16];
//     samp /= 2;
//     uint32_t current_period = pwm_hw->slice[slice_num].top;
//     samp = (samp * current_period) / (1 << 16);
//     pwm_set_chan_level(slice_num, pwm_gpio_to_channel(36), samp);


// }

// void init_pwm_audio() {
//     // fill in
//     gpio_set_dir(36, GPIO_IN);
//     gpio_put(36, 0);
//     gpio_set_function(36, GPIO_FUNC_PWM);
//     uint slice_num = pwm_gpio_to_slice_num(36);
//     pwm_set_clkdiv(slice_num, 150.0);
//     pwm_set_wrap(slice_num, (1000000 / 20000 ) - 1);
//     pwm_set_chan_level(slice_num, pwm_gpio_to_channel(36), 0);
//     pwm_set_enabled(slice_num, true);

//     duty_cycle = 0;

//     init_wavetable();
//     pwm_set_irq_enabled(slice_num, true);
//     irq_set_exclusive_handler(PWM_IRQ_WRAP_0, pwm_audio_handler);
//     irq_set_enabled(PWM_IRQ_WRAP_0, true);
//     pwm_set_enabled(slice_num, true);

// }

// //////////////////////////////////////////////////////////////////////////////

// int main()
// {
//     // Configures our microcontroller to 
//     // communicate over UART through the TX/RX pins
//     stdio_init_all();

//     // Uncomment when you need to run autotest.
//     // Keep this commented out until you need it
//     // since it adds a lot of time to the upload process.
//     autotest();

//     // Make sure to copy in the latest display.c and keypad.c from your previous labs.
//     keypad_init_pins();
//     keypad_init_timer();
//     display_init_pins();
//     display_init_timer();

//     /*
//     *******************************************************
//     * Make sure to go through the code in the steps below.  
//     * A lot of it can be very useful for your projects.
//     *******************************************************
//     */

//     #ifdef STEP2
//     init_pwm_static(100, 50); // Start out with 500/1000, 50%
//     display_char_print("      50");
//     uint16_t percent = 50; // Set initial percentage for duty cycle, displayed 
//     uint16_t disp_buffer = 0;
//     char buf[9];

//     // Display initial duty cycle
//     snprintf(buf, sizeof(buf), "      50");
//     display_char_print(buf);

//     bool new_entry = true;  // Flag to track if we're starting a new entry
    
//     for (;;) {
//         uint16_t keyevent = key_pop(); // Pop a key event from the queue
//         if (keyevent & 0x100) {
//             char key = keyevent & 0xFF;
//             if (key >= '0' && key <= '9') {
//                 // If the key is a digit, check if we need to clear the buffer first
//                 if (new_entry) {
//                     disp_buffer = 0;  // Clear the buffer for new entry
//                     new_entry = false;  // No longer a new entry
//                 }
//                 // Shift into buffer
//                 disp_buffer = (disp_buffer * 10) + (key - '0');
//                 snprintf(buf, sizeof(buf), "%8d", disp_buffer);
//                 display_char_print(buf); // Display the new value
//             } else if (key == '#') {
//                 // If the key is '#', set the duty cycle
//                 percent = disp_buffer;
//                 if (percent > 100) {
//                     percent = 100; // Cap at 100%
//                 }
//                 init_pwm_static(100, percent); // Update PWM with new duty cycle
//                 snprintf(buf, sizeof(buf), "%8d", percent);
//                 display_char_print(buf); // Display the new duty cycle
//                 new_entry = true;  // Ready for new entry
//             }
//             else if (key == '*') {
//                 // If the key is '*', reset the buffer
//                 disp_buffer = 50;
//                 percent = 50;
//                 init_pwm_static(100, percent); // Reset PWM to 50% duty cycle
//                 snprintf(buf, sizeof(buf), "      50");
//                 display_char_print(buf); // Display reset
//                 new_entry = true;  // Ready for new entry
//             }
//             else {
//                 // Any other key also starts a new entry
//                 new_entry = true;
//             }
//         }
//     }
//     #endif

//     #ifdef STEP3
//     init_pwm_static(10000, 5000); // Start out with 500/1000, 50%
//     init_pwm_irq(); // Initialize PWM IRQ for variable duty cycle

//     for(;;) {
//         // The handler manages everything from now on.
//         // Use the CPU to do something else!
//         tight_loop_contents();
//     }
//     #endif
    
//     #ifdef STEP4
//     char freq_buf[9] = {0};
//     int pos = 0;
//     bool decimal_entered = false;
//     int decimal_pos = 0;
//     int current_channel = 0;

//     keypad_init_pins();
//     keypad_init_timer();
//     display_init_pins();
//     display_init_timer();

//     init_pwm_audio(); 

//     // set_freq(0, 440.0f); // Set initial frequency to 440 Hz (A4 note)
//     // set_freq(1, 0.0f); // Turn off channel 1 initially
//     // set_freq(0, 261.626f);
//     // set_freq(1, 329.628f);

//     set_freq(0, 440.0f); // Set initial frequency for channel 0
//     display_char_print(" 440.000 ");

//     for(;;) {
//         uint16_t keyevent = key_pop();

//         if (keyevent & 0x100) {
//             char key = keyevent & 0xFF;
//             if (key == 'A') {
//                 current_channel = 0;
//                 pos = 0;
//                 freq_buf[0] = '\0';
//                 decimal_entered = false;
//                 decimal_pos = 0;
//                 display_char_print("         ");
//             } else if (key == 'B') {
//                 current_channel = 1;
//                 pos = 0;
//                 freq_buf[0] = '\0';
//                 decimal_entered = false;
//                 decimal_pos = 0;
//                 display_char_print("         ");
//             } else if (key >= '0' && key <= '9') {
//                 if (pos == 0) {
//                     snprintf(freq_buf, sizeof(freq_buf), "        "); // Clear buffer on first digit
//                     display_char_print(freq_buf);
//                 }
//                 if (pos < 8) {
//                     freq_buf[pos++] = key;
//                     freq_buf[pos] = '\0';
//                     display_char_print(freq_buf);
//                     if (decimal_entered) decimal_pos++;
//                 }
//                 } else if (key == '*') {
//                 if (!decimal_entered && pos < 7) {
//                     freq_buf[pos++] = '.';
//                     freq_buf[pos] = '\0';
//                     display_char_print(freq_buf);
//                     decimal_entered = true;
//                     decimal_pos = 0;
//                 }
//                 } else if (key == '#') {
//                 float freq = 0.0f;
//                 if (decimal_entered) {
//                     freq = strtof(freq_buf, NULL);
//                 } else {
//                     freq = (float)atoi(freq_buf);
//                 }
//                 set_freq(current_channel, freq);
//                 snprintf(freq_buf, sizeof(freq_buf), "%8.3f", freq);
//                 display_char_print(freq_buf);
//                 pos = 0;
//                 freq_buf[0] = '\0';
//                 decimal_entered = false;
//                 decimal_pos = 0;
//             } else {
//                 // Reset on any other key
//                 pos = 0;
//                 freq_buf[0] = '\0';
//                 decimal_entered = false;
//                 decimal_pos = 0;
//                 display_char_print("        ");
//             }
//         }
//     }
//     #endif

//     #ifdef DRUM_MACHINE
//         drum_machine();
//     #endif

//     while (true) {
//         printf("Hello, world!\n");
//         sleep_ms(1000);
//     }

//     for(;;);
//     return 0;
// }
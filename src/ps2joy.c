/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2024 Destroyer
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */


#include "tusb.h"
#include "hardware/watchdog.h"
#include "hardware/gpio.h"
#include "ps2joy.h"
#include <stdbool.h>

#define SCAN_CODE_SET_F0 0xf0
#define SCAN_CODE_SET_E2 0xe2

#define ESP_JOY1LEFT 0x40
#define ESP_JOY1RIGHT 0x41
#define ESP_JOY1UP 0x42
#define ESP_JOY1DOWN 0x43
#define ESP_JOY1START 0x44
#define ESP_JOY1MODE 0x45
#define ESP_JOY1A 0x46
#define ESP_JOY1B 0x47
#define ESP_JOY1C 0x48
#define ESP_JOY1X 0x49
#define ESP_JOY1Y 0x4a
#define ESP_JOY1Z 0x4b
#define ESP_JOY2LEFT 0x4c
#define ESP_JOY2RIGHT 0x4d
#define ESP_JOY2UP 0x4e
#define ESP_JOY2DOWN 0x4f
#define ESP_JOY2START 0x50
#define ESP_JOY2MODE 0x51
#define ESP_JOY2A 0x52
#define ESP_JOY2B 0x53
#define ESP_JOY2C 0x54
#define ESP_JOY2X 0x55
#define ESP_JOY2Y 0x56
#define ESP_JOY2Z 0x57
#define SLEEP_TIME 25


bool button_pressed = false;  

void send_joy_action(u8 scancode, bool press) {
    printf("***** KEYBOARD CONTROL\n");
    kb_send(SCAN_CODE_SET_E2);
    sleep_ms(SLEEP_TIME);
    printf("----> KEY PRESSED: %s\n", press ? "TRUE" : "FALSE");
    if (!press) {
        kb_send(SCAN_CODE_SET_F0);
        sleep_ms(SLEEP_TIME);
    }
    kb_send(scancode);
    sleep_ms(SLEEP_TIME);
    printf("**********************\n");
}

void check_joystick() {
    bool current_state = !gpio_get(BUTTON_TESTING);
    if (current_state && !button_pressed) {
        button_pressed = true;
        send_joy_action(ESP_JOY1LEFT, true);
    } 
    else if (!current_state && button_pressed) {
        button_pressed = false;
        send_joy_action(ESP_JOY1LEFT, false);
    }
}
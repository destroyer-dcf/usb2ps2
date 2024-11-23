
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
#include "gamepad.h"
#include <stdbool.h>
#include "button.h"


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

// #define ESP_JOY2LEFT 0x4c
// #define ESP_JOY2RIGHT 0x4d
// #define ESP_JOY2UP 0x4e
// #define ESP_JOY2DOWN 0x4f
// #define ESP_JOY2START 0x50
// #define ESP_JOY2MODE 0x51
// #define ESP_JOY2A 0x52
// #define ESP_JOY2B 0x53
// #define ESP_JOY2C 0x54
// #define ESP_JOY2X 0x55
// #define ESP_JOY2Y 0x56
// #define ESP_JOY2Z 0x57
#define SLEEP_TIME 25

// DB9 Pin 1 --> GPIO 2 (PIN 4) --> UP
// DB9 Pin 2 --> GPIO 3 (PIN 5) --> DOWN
// DB9 Pin 3 --> GPIO 4 (PIN 6) --> LEFT
// DB9 Pin 4 --> GPIO 5 (PIN 7) --> RIGHT
// DB9 Pin 5 --> +5V    (PIN 49) -> -----
// DB9 Pin 6 --> GPIO 6 (PIN 9) --> FIRE
// DB9 Pin 7 --> GPIO 7 (PIN 10) -> SELECT
// DB9 Pin 8 --> GND    (PIN 8) --> -----
// DB9 Pin 9 --> GPIO 8 (PIN 11) -> 

#define GAMEPAD_SELECT 7
#define GAMEPAD_UP 2 
#define GAMEPAD_DOWN 3
#define GAMEPAD_LEFT 4 
#define GAMEPAD_RIGHT 5 
#define GAMEPAD_FIRE 6 
#define GAMEPAD_MODE 8 // Es mode???


void onchange(button_t *button_p) {
  button_t *button = (button_t*)button_p;
//RELEASED
  if(button->state) {
    printf ("el state del pin %d es %d ¿Hemos SOLTADO el boton?\n",button->pin,button->state);
    switch(button->pin){

        case GAMEPAD_UP:
            kb_send_key(0x52, 0, 0);
            break;
        case GAMEPAD_DOWN:
            kb_send_key(0x51, 0, 0);
            break;
        case GAMEPAD_LEFT:
            kb_send_key(0x50, 0, 0);
            break;
        case GAMEPAD_RIGHT:
            kb_send_key(0x4f, 0, 0);
            break;
        case GAMEPAD_FIRE: 
            kb_send_key(0x2b, 0, 0);
            break;
    }
    
    return;
  }
   
   //PUSH
   if(!button->state) {printf ("el state del pin %d es %d ¿Hemos pulsado el boton?\n",button->pin,button->state);
   switch(button->pin){

    case GAMEPAD_UP:
        kb_send_key(0x52, 1, 0);
        break;
    case GAMEPAD_DOWN:
        kb_send_key(0x51, 1, 0);
        break;
    case GAMEPAD_LEFT:
        kb_send_key(0x50, 1, 0);
        break;
    case GAMEPAD_RIGHT:
        kb_send_key(0x4f, 1, 0);
        break;
    case GAMEPAD_FIRE:
        kb_send_key(0x2b, 1, 0);
        break;
  }
   
   return; }

}

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

void GamePadController(){
  button_t *Gamepad_UP = create_button(GAMEPAD_UP, onchange);
  button_t *Gamepad_DOWN = create_button(GAMEPAD_DOWN, onchange);
  button_t *Gamepad_LEFT = create_button(GAMEPAD_LEFT, onchange);
  button_t *Gamepad_RIGHT = create_button(GAMEPAD_RIGHT, onchange);
  button_t *Gamepad_FIRE = create_button(GAMEPAD_FIRE, onchange);
}
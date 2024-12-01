
#include <stdbool.h>
#include "pico/stdlib.h"
#include <stdio.h>
#include "tusb.h"
#include "ps2out.h"
#include "ps2in.h"
#include "hardware/watchdog.h"
#include "scancodesets.h"

//START GAMEPAD

#define GAMEPAD_UP 2 
#define GAMEPAD_DOWN 3
#define GAMEPAD_LEFT 4 
#define GAMEPAD_RIGHT 5 
#define GAMEPAD_FIRE 6 

#define ESP_JOY1LEFT 0x40
#define ESP_JOY1RIGHT 0x41
#define ESP_JOY1UP 0x42
#define ESP_JOY1DOWN 0x43
#define ESP_JOYFIRE 0x46

#define SCAN_CODE_SET_F0 0xf0
#define SCAN_CODE_SET_E2 0xe2

bool gamepad_left_state = false;
bool gamepad_left_prev_state = false;
bool gamepad_right_state = false;
bool gamepad_right_prev_state = false;
bool gamepad_up_state = false;
bool gamepad_up_prev_state = false;
bool gamepad_down_state = false;
bool gamepad_down_prev_state = false;
bool gamepad_fire_state = false;
bool gamepad_fire_prev_state = false;

#define SCAN_CODE_SET_F0 0xf0
#define SCAN_CODE_SET_E2 0xe2
#define SLEEP_TIME 25

//END GAMEPAD
// u16 delay_ms;
// alarm_id_t repeater;
// u8 key2repeat = 0;

// ************************
// GAMEPAD CONTROL
// ************************


void sendGamePad(u8 scancode, bool press) {
    printf("***** GAMEPAD CONTROL\n");
    kb_send(SCAN_CODE_SET_E2);
    sleep_ms(SLEEP_TIME);
    printf("----> GAMEPAD PRESSED: %s\n", press ? "TRUE" : "FALSE");
    if (!press) {
        kb_send(KB_BREAK_2_3);
        sleep_ms(SLEEP_TIME);
    }
    kb_send(scancode);
    sleep_ms(SLEEP_TIME);
    printf("**********************\n");
}


void gamepad_controls() {
    
    gamepad_left_state  = !gpio_get(GAMEPAD_LEFT);
    gamepad_right_state = !gpio_get(GAMEPAD_RIGHT);
    gamepad_up_state    = !gpio_get(GAMEPAD_UP);
    gamepad_down_state  = !gpio_get(GAMEPAD_DOWN);
    gamepad_fire_state  = !gpio_get(GAMEPAD_FIRE);

    // LEFT CONTROL ******
    if (gamepad_left_state && !gamepad_left_prev_state) {
        //sendGamePad(ESP_JOY1LEFT,true);
        kb_send_key_gamepad_control(0,true);
    }
    if (!gamepad_left_state && gamepad_left_prev_state) {
        //sendGamePad(ESP_JOY1LEFT,false);
        kb_send_key_gamepad_control(0,false);
    }

    // RIGHT CONTROL ******
    if (gamepad_right_state && !gamepad_right_prev_state) {
        // sendGamePad(ESP_JOY1RIGHT,true);
        kb_send_key_gamepad_control(1,true);
    }
    if (!gamepad_right_state && gamepad_right_prev_state) {
        kb_send_key_gamepad_control(1,false);
        // sendGamePad(ESP_JOY1RIGHT,false);
    }

    // UP CONTROL ******
    if (gamepad_up_state && !gamepad_up_prev_state) {
        kb_send_key_gamepad_control(2,true);
        // sendGamePad(ESP_JOY1UP,true);
    }
    if (!gamepad_up_state && gamepad_up_prev_state) {
        kb_send_key_gamepad_control(2,false);
        // sendGamePad(ESP_JOY1UP,false);
    }

    // DOWN CONTROL ******
    if (gamepad_down_state && !gamepad_down_prev_state) {
        kb_send_key_gamepad_control(3,true);
        // sendGamePad(ESP_JOY1DOWN,true);
    }
    if (!gamepad_down_state && gamepad_down_prev_state) {
        // sendGamePad(ESP_JOY1DOWN,false);
        kb_send_key_gamepad_control(3,false);
    }

    // FIRE CONTROL ******
    if (gamepad_fire_state && !gamepad_fire_prev_state) {
        // sendGamePad(ESP_JOYFIRE,true);
        kb_send_key_gamepad_control(6,true);
    }
    if (!gamepad_fire_state && gamepad_fire_prev_state) {
        // sendGamePad(ESP_JOYFIRE,false);
        kb_send_key_gamepad_control(6,false);
    }

    gamepad_left_prev_state = gamepad_left_state;
    gamepad_right_prev_state = gamepad_right_state;
    gamepad_up_prev_state = gamepad_up_state;
    gamepad_down_prev_state = gamepad_down_state;
    gamepad_fire_prev_state = gamepad_fire_state;
}

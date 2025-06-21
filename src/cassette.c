
#include <stdbool.h>
#include "pico/stdlib.h"
#include <stdio.h>
#include "tusb.h"
#include "ps2out.h"
#include "ps2in.h"
#include "hardware/watchdog.h"
#include "scancodesets.h"

//START GAMEPAD

#define CASSETTE_REC 28
#define CASSETTE_PLAY 27
#define CASSETTE_REW 26
#define CASSETTE_FF 9
#define CASSETTE_STOP 8
#define CASSETTE_PAUSE 2

// #define ESP_JOY1LEFT 0x40
// #define ESP_JOY1RIGHT 0x41
// #define ESP_JOY1UP 0x42
// #define ESP_JOY1DOWN 0x43
// #define ESP_JOYFIRE 0x46

#define SCANCODE_REC 0x1C
#define SCANCODE_PLAY 0x32
#define SCANCODE_REW 0x21
#define SCANCODE_FF 0x23
#define SCANCODE_STOP 0x24
#define SCANCODE_PAUSE 0x2B


#define SCAN_CODE_SET_F0 0xf0
#define SCAN_CODE_SET_E2 0xe2

bool CASSETTE_REW_state = false;
bool CASSETTE_REW_prev_state = false;
bool CASSETTE_FF_state = false;
bool CASSETTE_FF_prev_state = false;
bool CASSETTE_REC_state = false;
bool CASSETTE_REC_prev_state = false;
bool CASSETTE_PLAY_state = false;
bool CASSETTE_PLAY_prev_state = false;
bool CASSETTE_STOP_state = false;
bool CASSETTE_STOP_prev_state = false;
bool CASSETTE_PAUSE_state = false;
bool CASSETTE_PAUSE_prev_state = false;

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
    printf("***** CASSETTE CONTROL\n");
    kb_send(SCAN_CODE_SET_E2);
    sleep_ms(SLEEP_TIME);
    printf("----> CASSETTE PRESSED: %s\n", press ? "TRUE" : "FALSE");
    if (!press) {
        kb_send(KB_BREAK_2_3);
        sleep_ms(SLEEP_TIME);
    }
    kb_send(scancode);
    sleep_ms(SLEEP_TIME);
    printf("**********************\n");
}


void cassette_control() {
    
    CASSETTE_REW_state  = !gpio_get(CASSETTE_REW);
    CASSETTE_FF_state = !gpio_get(CASSETTE_FF);
    CASSETTE_REC_state    = !gpio_get(CASSETTE_REC);
    CASSETTE_PLAY_state  = !gpio_get(CASSETTE_PLAY);
    CASSETTE_STOP_state  = !gpio_get(CASSETTE_STOP);
    CASSETTE_PAUSE_state  = !gpio_get(CASSETTE_PAUSE);

    // REC ******
    if (CASSETTE_REC_state && !CASSETTE_REC_prev_state) {
        //sendGamePad(SCANCODE_REC,true);
        kb_send_key_cassette_control(0,true);
    }
    if (!CASSETTE_REC_state && CASSETTE_REC_prev_state) {
        //sendGamePad(SCANCODE_REC,false);
        kb_send_key_cassette_control(0,false);
    }

    // PLAY ******
    if (CASSETTE_PLAY_state && !CASSETTE_PLAY_prev_state) {
        sendGamePad(SCANCODE_PLAY,true);
        //kb_send_key_cassette_control(1,true);
    }
    if (!CASSETTE_PLAY_state && CASSETTE_PLAY_prev_state) {
        //kb_send_key_cassette_control(1,false);
        sendGamePad(SCANCODE_PLAY,false);
    }

    // REW ******
    if (CASSETTE_REW_state && !CASSETTE_REW_prev_state) {
        //kb_send_key_cassette_control(2,true);
        sendGamePad(SCANCODE_REW,true);
    }
    if (!CASSETTE_REW_state && CASSETTE_REW_prev_state) {
        //kb_send_key_cassette_control(2,false);
        sendGamePad(SCANCODE_REW,false);
    }

    // FF ******
    if (CASSETTE_FF_state && !CASSETTE_FF_prev_state) {
        //kb_send_key_cassette_control(3,true);
        sendGamePad(SCANCODE_FF,true);
    }
    if (!CASSETTE_FF_state && CASSETTE_FF_prev_state) {
        sendGamePad(SCANCODE_FF,false);
        //kb_send_key_cassette_control(3,false);
    }

    // FIRE CONTROL ******
    if (CASSETTE_STOP_state && !CASSETTE_STOP_prev_state) {
        sendGamePad(SCANCODE_STOP,true);
        //kb_send_key_cassette_control(6,true);
    }
    if (!CASSETTE_STOP_state && CASSETTE_STOP_prev_state) {
        sendGamePad(SCANCODE_STOP,false);
        //kb_send_key_cassette_control(6,false);
    }

        // PAUSE ******
    if (CASSETTE_PAUSE_state && !CASSETTE_PAUSE_prev_state) {
        sendGamePad(SCANCODE_PAUSE,true);
    }
    if (!CASSETTE_PAUSE_state && CASSETTE_PAUSE_prev_state) {
        sendGamePad(SCANCODE_PAUSE,false);
    }

    CASSETTE_REW_prev_state = CASSETTE_REW_state;
    CASSETTE_FF_prev_state = CASSETTE_FF_state;
    CASSETTE_REC_prev_state = CASSETTE_REC_state;
    CASSETTE_PLAY_prev_state = CASSETTE_PLAY_state;
    CASSETTE_STOP_prev_state = CASSETTE_STOP_state;
    CASSETTE_PAUSE_prev_state = CASSETTE_PAUSE_state;
}

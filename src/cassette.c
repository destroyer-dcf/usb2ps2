
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"
#include "scancodesets.h"

#define CASSETTE_REC 28
#define CASSETTE_PLAY 27
#define CASSETTE_REW 26
#define CASSETTE_FF 9
#define CASSETTE_STOP 8
#define CASSETTE_PAUSE 2

#define SCANCODE_REC 0x83
#define SCANCODE_PLAY 0x3f
#define SCANCODE_REW 0x83
#define SCANCODE_FF 0x83
#define SCANCODE_STOP 0x03

#define SCAN_CODE_SET_F0 0xf0
#define SCAN_CODE_SET_E2 0xe2

typedef struct {
    bool rec;
    bool play;
    bool rew;
    bool ff;
    bool stop;
    bool pause;
    bool last_rec;
    bool last_play;
    bool last_rew;
    bool last_ff;
    bool last_stop;
    bool last_pause;
} CassetteState;

CassetteState cassette_state = {1, 1, 1, 1, 1, 1}; 

void setupGpios() {
    gpio_init(CASSETTE_REW);
    gpio_set_dir(CASSETTE_REW, GPIO_IN);
    gpio_pull_up(CASSETTE_REW);

    gpio_init(CASSETTE_FF);
    gpio_set_dir(CASSETTE_FF, GPIO_IN);
    gpio_pull_up(CASSETTE_FF);

    gpio_init(CASSETTE_REC);
    gpio_set_dir(CASSETTE_REC, GPIO_IN);
    gpio_pull_up(CASSETTE_REC);

    gpio_init(CASSETTE_PLAY);
    gpio_set_dir(CASSETTE_PLAY, GPIO_IN);
    gpio_pull_up(CASSETTE_PLAY);

    gpio_init(CASSETTE_STOP);
    gpio_set_dir(CASSETTE_STOP, GPIO_IN);
    gpio_pull_up(CASSETTE_STOP);

    gpio_init(CASSETTE_PAUSE);
    gpio_set_dir(CASSETTE_PAUSE, GPIO_IN);
    gpio_pull_up(CASSETTE_PAUSE);
}

void readCassetteButton(CassetteState *state) {
   
    // Leer el estado de cada botón 1 si no esta pulsado 0 si lo pulsamos
    state->rec = gpio_get(CASSETTE_REC);
    state->play = gpio_get(CASSETTE_PLAY);
    state->rew = gpio_get(CASSETTE_REW);
    state->ff = gpio_get(CASSETTE_FF);
    state->stop = gpio_get(CASSETTE_STOP);
    state->pause = gpio_get(CASSETTE_PAUSE);
}


void sendCassetteButton(uint8_t scancode, int pressed) {
    if (pressed) {
        kb_send(scancode);
    } else {
        kb_send(SCAN_CODE_SET_F0);
        kb_send(scancode);
    }
}

void Pause() {
    kb_send(0xE1);
    kb_send(0x14);
    kb_send(0x77);
    kb_send(0xE1);
    kb_send(SCAN_CODE_SET_F0);
    kb_send(0x14);
    kb_send(SCAN_CODE_SET_F0);
    kb_send(0x77);
}

void cassetteControl()
{
    readCassetteButton(&cassette_state);
    
    // ##### REC BUTTON #####
    if (cassette_state.rec == 0 && cassette_state.last_rec == 1) {
        kb_send_key_scs2(HID_KEY_F7, true, false);
    } else if (cassette_state.rec == 1 && cassette_state.last_rec == 1) {
        kb_send_key_scs2(HID_KEY_F7, false, false);
        cassette_state.last_rec = 0;
    }

    if (cassette_state.rec == 0 && cassette_state.last_rec == 0) {
        kb_send_key_scs2(HID_KEY_F7, true, false);
        cassette_state.last_rec = 1;
    }

    // ##### PLAY BUTTON #####
    if (cassette_state.play == 0 && cassette_state.last_play== 1) {
        kb_send_key_scs2(HID_KEY_F5, true, false);
    } else if (cassette_state.play == 1 && cassette_state.last_play == 1) {
        kb_send_key_scs2(HID_KEY_F5, false, false);
        cassette_state.last_play = 0;
    }

    if (cassette_state.play == 0 && cassette_state.last_play == 0) {
        kb_send_key_scs2(HID_KEY_F5, true, false);
        cassette_state.last_play = 1;
    }

    // ##### REW BUTTON #####
    if (cassette_state.rew == 0 && cassette_state.last_rew== 1) {
        kb_send_key_scs2(HID_KEY_SHIFT_LEFT, true, false);
        kb_send_key_scs2(HID_KEY_F4, true, false);      
    } else if (cassette_state.rew == 1 && cassette_state.last_rew == 1) {
        kb_send_key_scs2(HID_KEY_F4, false, false);      
        kb_send_key_scs2(HID_KEY_SHIFT_LEFT, false, false); 
        cassette_state.last_rew = 0;
    }

    if (cassette_state.rew == 0 && cassette_state.last_rew == 0) {
        kb_send_key_scs2(HID_KEY_SHIFT_LEFT, true, false); 
        kb_send_key_scs2(HID_KEY_F4, true, false);        
        cassette_state.last_rew = 1;
    }

    // ##### FF BUTTON #####
    if (cassette_state.ff == 0 && cassette_state.last_ff== 1) {
        kb_send_key_scs2(HID_KEY_SHIFT_LEFT, true, false); 
        kb_send_key_scs2(HID_KEY_F4, true, false);        
    } else if (cassette_state.ff == 1 && cassette_state.last_ff == 1) {
        kb_send_key_scs2(HID_KEY_F4, false, false);        
        kb_send_key_scs2(HID_KEY_SHIFT_LEFT, false, false); 
        cassette_state.last_ff = 0;
    }

    if (cassette_state.ff == 0 && cassette_state.last_ff == 0) {
        kb_send_key_scs2(HID_KEY_SHIFT_LEFT, true, false); // Presionar Shift
        kb_send_key_scs2(HID_KEY_F4, true, false);         // Presionar F4
        cassette_state.last_ff = 1;
    }

    // ##### STOP BUTTON #####
    if (cassette_state.stop == 0 && cassette_state.last_stop == 1) {
        // Flanco de bajada: se acaba de pulsar
        kb_send_key_scs2(HID_KEY_F4, true, false);
    }
    else if (cassette_state.stop == 1 && cassette_state.last_stop == 0) {
        // Flanco de subida: se acaba de soltar
        kb_send_key_scs2(HID_KEY_F4, false, false);
    }

    // Actualiza el estado anterior
    cassette_state.last_stop = cassette_state.stop;

    // ##### PAUSE BUTTON #####
    if (cassette_state.pause == 0 && cassette_state.last_pause== 1) {
        kb_send_key_scs2(HID_KEY_F5, true, false);   
    } else if (cassette_state.pause== 1 && cassette_state.last_pause == 1) {
        kb_send_key_scs2(HID_KEY_F5, false, false);       
        cassette_state.last_pause = 0;
    }

    if (cassette_state.pause == 0 && cassette_state.last_pause == 0) {
        kb_send_key_scs2(HID_KEY_F5, true, false);
        cassette_state.last_pause = 1;
    }
    // // Actualiza el estado anterior
    // cassette_state.last_stop = cassette_state.stop;

    // // ##### PAUSE BUTTON #####
    // static uint32_t last_pause_time = 0;
    // uint32_t now = time_us_32();

    // if (cassette_state.pause == 0 && (now - last_pause_time > 200000)) { 
    //     Pause();
    //     last_pause_time = now;
    // }
}

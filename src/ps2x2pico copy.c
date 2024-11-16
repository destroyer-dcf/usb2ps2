/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2024 No0ne (https://github.com/No0ne)
 *           (c) 2023 Dustin Hoffman
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "hardware/watchdog.h"
#include "hardware/gpio.h"
#include "bsp/board_api.h"
#include "tusb.h"
#include "ps2x2pico.h"

static void print_utf16(uint16_t *temp_buf, size_t buf_len);
void print_device_descriptor(tuh_xfer_t* xfer);

//######################################################################################################################
uint32_t last_button_check = 0;  // Última vez que se verificó el botón
static uint16_t lastState = 0; // Declara lastState para almacenar el estado anterior
static uint16_t currentState = 0; // Declara lastState para almacenar el estado anterior
#define SLEEP_TIME 25
#define GAMEPAD_SELECT 7
#define GAMEPAD_UP 2 
#define GAMEPAD_DOWN 3
#define GAMEPAD_LEFT 4 
#define GAMEPAD_RIGHT 5 
#define GAMEPAD_FIRE 6 
#define GAMEPAD_MODE 8 // Es mode???
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

// Definición de constantes y enumeraciones para el estado de los botones
typedef enum {
    SC_CTL_ON    = 1,    // El controlador está conectado
    SC_BTN_UP    = 2,
    SC_BTN_DOWN  = 4,
    SC_BTN_LEFT  = 8,
    SC_BTN_RIGHT = 16,
    SC_BTN_START = 32,
    SC_BTN_A     = 64,
    SC_BTN_B     = 128,
    SC_BTN_C     = 256,
    SC_BTN_X     = 512,
    SC_BTN_Y     = 1024,
    SC_BTN_Z     = 2048,
    SC_BTN_MODE  = 4096,
    SC_BTN_1     = 128,  // Compatibilidad con Master System
    SC_BTN_2     = 256   // Compatibilidad con Master System
};

// Definiciones para la configuración de pines y ciclos
#define SC_INPUT_PINS 6
#define SC_CYCLES 8
#define SC_READ_DELAY_MS 5 // Debe ser >= 3 para dar tiempo a que se reinicie el controlador de 6 botones

// Definición de la estructura GamePad
typedef struct {
    uint _selectPin;                // Pin de selección de salida
    uint _inputPins[SC_INPUT_PINS]; // Array de pines de entrada
    uint16_t _currentState;         // Estado actual de los botones
    uint32_t _lastReadTime;         // Última vez que se leyó el estado
    bool _sixButtonMode;            // Modo de seis botones activado/desactivado
} GamePad;

//######################################################################################################################

// Inicializa el GamePad con los pines correspondientes
void GamePad_init(GamePad* gamepad, uint db9_pin_7, uint db9_pin_1, uint db9_pin_2, uint db9_pin_3, uint db9_pin_4, uint db9_pin_6, uint db9_pin_9) {
    gamepad->_selectPin = db9_pin_7;

    gamepad->_inputPins[0] = db9_pin_1;
    gamepad->_inputPins[1] = db9_pin_2;
    gamepad->_inputPins[2] = db9_pin_3;
    gamepad->_inputPins[3] = db9_pin_4;
    gamepad->_inputPins[4] = db9_pin_6;
    gamepad->_inputPins[5] = db9_pin_9;

    gpio_init(gamepad->_selectPin);
    gpio_set_dir(gamepad->_selectPin, GPIO_OUT);
    gpio_put(gamepad->_selectPin, 1);

    for (int i = 0; i < SC_INPUT_PINS; i++) {
        gpio_init(gamepad->_inputPins[i]);
        gpio_set_dir(gamepad->_inputPins[i], GPIO_IN);
        gpio_pull_up(gamepad->_inputPins[i]);
    }

    gamepad->_currentState = 0;
    gamepad->_sixButtonMode = false;
    gamepad->_lastReadTime = to_ms_since_boot(get_absolute_time());
}

// Devuelve el estado actual de los botones
uint16_t GamePad_getState(GamePad* gamepad) {
    uint32_t currentTime = to_ms_since_boot(get_absolute_time());

    if ((currentTime - gamepad->_lastReadTime) < SC_READ_DELAY_MS) {
        return gamepad->_currentState;
    }

    uint32_t savedInterrupts = save_and_disable_interrupts();
    gamepad->_currentState = 0;

    for (int cycle = 0; cycle < SC_CYCLES; cycle++) {
        GamePad_readCycle(gamepad, cycle);
    }

    if (!(gamepad->_currentState & SC_CTL_ON)) {
        gamepad->_sixButtonMode = false;
    }

    restore_interrupts(savedInterrupts);
    gamepad->_lastReadTime = currentTime;
    return gamepad->_currentState;
}

// Lee un ciclo específico para obtener el estado de los botones
void GamePad_readCycle(GamePad* gamepad, int cycle) {
    gpio_put(gamepad->_selectPin, cycle % 2);

    switch (cycle) {
        case 2:
            gamepad->_currentState |= (gpio_get(gamepad->_inputPins[2]) == 0 && gpio_get(gamepad->_inputPins[3]) == 0) * SC_CTL_ON;
            
            if (gamepad->_currentState & SC_CTL_ON) {
                if (gpio_get(gamepad->_inputPins[4]) == 0) { gamepad->_currentState |= SC_BTN_A; }
                if (gpio_get(gamepad->_inputPins[5]) == 0) { gamepad->_currentState |= SC_BTN_START; }
            }
            break;

        case 3:
            if (gpio_get(gamepad->_inputPins[0]) == 0) { gamepad->_currentState |= SC_BTN_UP; }
            if (gpio_get(gamepad->_inputPins[1]) == 0) { gamepad->_currentState |= SC_BTN_DOWN; }
            if (gpio_get(gamepad->_inputPins[2]) == 0) { gamepad->_currentState |= SC_BTN_LEFT; }
            if (gpio_get(gamepad->_inputPins[3]) == 0) { gamepad->_currentState |= SC_BTN_RIGHT; }
            if (gpio_get(gamepad->_inputPins[4]) == 0) { gamepad->_currentState |= SC_BTN_B; }
            if (gpio_get(gamepad->_inputPins[5]) == 0) { gamepad->_currentState |= SC_BTN_C; }
            break;

        case 4:
            gamepad->_sixButtonMode = (gpio_get(gamepad->_inputPins[0]) == 0 && gpio_get(gamepad->_inputPins[1]) == 0);
            break;

        case 5:
            if (gamepad->_sixButtonMode) {
                if (gpio_get(gamepad->_inputPins[0]) == 0) { gamepad->_currentState |= SC_BTN_Z; }
                if (gpio_get(gamepad->_inputPins[1]) == 0) { gamepad->_currentState |= SC_BTN_Y; }
                if (gpio_get(gamepad->_inputPins[2]) == 0) { gamepad->_currentState |= SC_BTN_X; }
                if (gpio_get(gamepad->_inputPins[3]) == 0) { gamepad->_currentState |= SC_BTN_MODE; }
            }
            break;
    }
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

void check_joystick() {


    uint16_t currentState = GamePad_getState(&gamepad);

    // Verificar el cambio de estado para cada botón, evitando el rebote
    // uint32_t currentMillis = millis();
    
    // SC_BTN_UP    
    if (currentState != lastState)
    {
        // Verificar el cambio de estado para cada botón
        if ((currentState & SC_BTN_UP) != (lastState & SC_BTN_UP)) {
            if (currentState & SC_BTN_UP) {
                send_joy_action(ESP_JOY1UP, true);
            } else {
                send_joy_action(ESP_JOY1UP, false);
            }
        }

        if ((currentState & SC_BTN_DOWN) != (lastState & SC_BTN_DOWN)) {
            if (currentState & SC_BTN_DOWN) {
                send_joy_action(ESP_JOY1DOWN, true);
            } else {
                send_joy_action(ESP_JOY1DOWN, false);
            }
        }

        if ((currentState & SC_BTN_LEFT) != (lastState & SC_BTN_LEFT)) {
            if (currentState & SC_BTN_LEFT) {
                send_joy_action(ESP_JOY1LEFT, true);
            } else {
                send_joy_action(ESP_JOY1LEFT, false);
            }
        }

        if ((currentState & SC_BTN_RIGHT) != (lastState & SC_BTN_RIGHT)) {
            if (currentState & SC_BTN_RIGHT) {
                send_joy_action(ESP_JOY1RIGHT, true);
            } else {
                send_joy_action(ESP_JOY1RIGHT, false);
            }
        }

        if ((currentState & SC_BTN_START) != (lastState & SC_BTN_START)) {
            if (currentState & SC_BTN_START) {
                send_joy_action(ESP_JOY1START, true);
            } else {
                send_joy_action(ESP_JOY1START, false);
            }
        }

        if ((currentState & SC_BTN_A) != (lastState & SC_BTN_A)) {
            if (currentState & SC_BTN_A) {
                send_joy_action(ESP_JOY1A, true);
            } else {
                send_joy_action(ESP_JOY1A, false);
            }
        }

        if ((currentState & SC_BTN_B) != (lastState & SC_BTN_B)) {
            if (currentState & SC_BTN_B) {
                send_joy_action(ESP_JOY1B, true);
            } else {
                send_joy_action(ESP_JOY1B, false);
            }
        }

        if ((currentState & SC_BTN_C) != (lastState & SC_BTN_C)) {
            if (currentState & SC_BTN_C) {
                send_joy_action(ESP_JOY1C, true);
            } else {
                send_joy_action(ESP_JOY1C, false);
            }
        }

        if ((currentState & SC_BTN_X) != (lastState & SC_BTN_X)) {
            if (currentState & SC_BTN_X) {
                send_joy_action(ESP_JOY1X, true);
            } else {
                send_joy_action(ESP_JOY1X, false);
            }
        }

        if ((currentState & SC_BTN_Y) != (lastState & SC_BTN_Y)) {
            if (currentState & SC_BTN_Y) {
                send_joy_action(ESP_JOY1Y, true);
            } else {
                send_joy_action(ESP_JOY1Y, false);
            }
        }

        if ((currentState & SC_BTN_Z) != (lastState & SC_BTN_Z)) {
            if (currentState & SC_BTN_Z) {
                send_joy_action(ESP_JOY1Z, true);
            } else {
                send_joy_action(ESP_JOY1Z, false);
            }
        }

        if ((currentState & SC_BTN_1) != (lastState & SC_BTN_1)) {
            if (currentState & SC_BTN_1) {
                send_joy_action(ESP_JOY1A, true);
            } else {
                send_joy_action(ESP_JOY1A, false);
            }
        }

        if ((currentState & SC_BTN_2) != (lastState & SC_BTN_2)) {
            if (currentState & SC_BTN_2) {
                send_joy_action(ESP_JOY1Z, true);
            } else {
                send_joy_action(ESP_JOY1Z, false);
            }
        }

        if ((currentState & SC_BTN_MODE) != (lastState & SC_BTN_MODE)) {
            if (currentState & SC_BTN_MODE) {
                send_joy_action(ESP_JOY1MODE, true);
            } else {
                send_joy_action(ESP_JOY1MODE, false);
            }
        }

        lastState = currentState;
    }
}


void main() {
  
  static GamePad gamepad;
  GamePad_init(&gamepad, GAMEPAD_SELECT, GAMEPAD_UP, GAMEPAD_DOWN, GAMEPAD_LEFT, GAMEPAD_RIGHT, GAMEPAD_FIRE, 8);

    while (1) {
        check_joystick();
    }
}

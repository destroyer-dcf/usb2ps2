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


bool button_pressed = false;  


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

// void send_joy_action(u8 scancode, bool press) {
//     printf("***** KEYBOARD CONTROL\n");
//     kb_send(SCAN_CODE_SET_E2);
//     sleep_ms(SLEEP_TIME);
//     printf("----> KEY PRESSED: %s\n", press ? "TRUE" : "FALSE");
//     if (!press) {
//         kb_send(SCAN_CODE_SET_F0);
//         sleep_ms(SLEEP_TIME);
//     }
//     kb_send(scancode);
//     sleep_ms(SLEEP_TIME);
//     printf("**********************\n");
// }



static uint32_t milliseconds = 0;

// Función callback que se ejecuta cada milisegundo
// Cambiar la firma para que coincida con alarm_callback_t
int64_t timer_callback(int64_t alarm_time, void *user_data) {
    milliseconds++;  // Incrementa cada vez que el temporizador se desborda (1ms)
    return 0;  // Retorna 0 para indicar que el temporizador no se repite
}

// Función para inicializar el temporizador
void setup_timer() {
    // Configura el temporizador para que se ejecute cada milisegundo
    add_alarm_in_ms(1, timer_callback, NULL, true);  // Llama a 'timer_callback' cada 1ms
}

// Función millis() para obtener el tiempo transcurrido en milisegundos
uint32_t millis() {
    return milliseconds;
}

void check_joystick() {
    static GamePad gamepad;
    static bool initialized = false;
    static uint16_t lastState = 0; // Declara lastState para almacenar el estado anterior
    static uint32_t lastDebounceTime = 0;  // Almacena el tiempo del último rebote

    // Inicializa el GamePad solo una vez
    if (!initialized) {
        GamePad_init(&gamepad, GAMEPAD_SELECT, GAMEPAD_UP, GAMEPAD_DOWN, GAMEPAD_LEFT, GAMEPAD_RIGHT, GAMEPAD_FIRE, 8);
        initialized = true;
    }

    uint16_t currentState = GamePad_getState(&gamepad);

    // Verificar el cambio de estado para cada botón, evitando el rebote
    uint32_t currentMillis = millis();
    
    // SC_BTN_UP
    if ((currentState & SC_BTN_UP) != (lastState & SC_BTN_UP)) {
        if (currentMillis - lastDebounceTime > SLEEP_TIME) {  // 50ms de debounce
            if (currentState & SC_BTN_UP) {
                send_joy_action(ESP_JOY1UP, true);
            } else {
                send_joy_action(ESP_JOY1UP, false);
            }
            lastDebounceTime = currentMillis;
        }
    }

    // SC_BTN_DOWN
    if ((currentState & SC_BTN_DOWN) != (lastState & SC_BTN_DOWN)) {
        if (currentMillis - lastDebounceTime > SLEEP_TIME) {
            if (currentState & SC_BTN_DOWN) {
                send_joy_action(ESP_JOY1DOWN, true);
            } else {
                send_joy_action(ESP_JOY1DOWN, false);
            }
            lastDebounceTime = currentMillis;
        }
    }

    // SC_BTN_LEFT
    if ((currentState & SC_BTN_LEFT) != (lastState & SC_BTN_LEFT)) {
        if (currentMillis - lastDebounceTime > SLEEP_TIME) {
            if (currentState & SC_BTN_LEFT) {
                send_joy_action(ESP_JOY1LEFT, true);
            } else {
                send_joy_action(ESP_JOY1LEFT, false);
            }
            lastDebounceTime = currentMillis;
        }
    }

    // SC_BTN_RIGHT
    if ((currentState & SC_BTN_RIGHT) != (lastState & SC_BTN_RIGHT)) {
        if (currentMillis - lastDebounceTime > SLEEP_TIME) {
            if (currentState & SC_BTN_RIGHT) {
                send_joy_action(ESP_JOY1RIGHT, true);
            } else {
                send_joy_action(ESP_JOY1RIGHT, false);
            }
            lastDebounceTime = currentMillis;
        }
    }

    // SC_BTN_START
    if ((currentState & SC_BTN_START) != (lastState & SC_BTN_START)) {
        if (currentMillis - lastDebounceTime > SLEEP_TIME) {
            if (currentState & SC_BTN_START) {
                send_joy_action(ESP_JOY1START, true);
            } else {
                send_joy_action(ESP_JOY1START, false);
            }
            lastDebounceTime = currentMillis;
        }
    }

    // SC_BTN_A
    if ((currentState & SC_BTN_A) != (lastState & SC_BTN_A)) {
        if (currentMillis - lastDebounceTime > SLEEP_TIME) {
            if (currentState & SC_BTN_A) {
                send_joy_action(ESP_JOY1A, true);
            } else {
                send_joy_action(ESP_JOY1A, false);
            }
            lastDebounceTime = currentMillis;
        }
    }

    // SC_BTN_B
    if ((currentState & SC_BTN_B) != (lastState & SC_BTN_B)) {
        if (currentMillis - lastDebounceTime > SLEEP_TIME) {
            if (currentState & SC_BTN_B) {
                send_joy_action(ESP_JOY1B, true);
            } else {
                send_joy_action(ESP_JOY1B, false);
            }
            lastDebounceTime = currentMillis;
        }
    }

    // SC_BTN_C
    if ((currentState & SC_BTN_C) != (lastState & SC_BTN_C)) {
        if (currentMillis - lastDebounceTime > SLEEP_TIME) {
            if (currentState & SC_BTN_C) {
                send_joy_action(ESP_JOY1C, true);
            } else {
                send_joy_action(ESP_JOY1C, false);
            }
            lastDebounceTime = currentMillis;
        }
    }

    // SC_BTN_X
    if ((currentState & SC_BTN_X) != (lastState & SC_BTN_X)) {
        if (currentMillis - lastDebounceTime > SLEEP_TIME) {
            if (currentState & SC_BTN_X) {
                send_joy_action(ESP_JOY1X, true);
            } else {
                send_joy_action(ESP_JOY1X, false);
            }
            lastDebounceTime = currentMillis;
        }
    }

    // SC_BTN_Y
    if ((currentState & SC_BTN_Y) != (lastState & SC_BTN_Y)) {
        if (currentMillis - lastDebounceTime > 50) {
            if (currentState & SC_BTN_Y) {
                send_joy_action(ESP_JOY1Y, true);
            } else {
                send_joy_action(ESP_JOY1Y, false);
            }
            lastDebounceTime = currentMillis;
        }
    }

    // SC_BTN_Z
    if ((currentState & SC_BTN_Z) != (lastState & SC_BTN_Z)) {
        if (currentMillis - lastDebounceTime > 25) {
            if (currentState & SC_BTN_Z) {
                send_joy_action(ESP_JOY1Z, true);
            } else {
                send_joy_action(ESP_JOY1Z, false);
            }
            lastDebounceTime = currentMillis;
        }
    }

    // SC_BTN_1
    if ((currentState & SC_BTN_1) != (lastState & SC_BTN_1)) {
        if (currentMillis - lastDebounceTime > 25) {
            if (currentState & SC_BTN_1) {
                send_joy_action(ESP_JOY1A, true);
            } else {
                send_joy_action(ESP_JOY1A, false);
            }
            lastDebounceTime = currentMillis;
        }
    }

    // SC_BTN_2
    if ((currentState & SC_BTN_2) != (lastState & SC_BTN_2)) {
        if (currentMillis - lastDebounceTime > 50) {
            if (currentState & SC_BTN_2) {
                send_joy_action(ESP_JOY1Z, true);
            } else {
                send_joy_action(ESP_JOY1Z, false);
            }
            lastDebounceTime = currentMillis;
        }
    }

    // SC_BTN_MODE
    if ((currentState & SC_BTN_MODE) != (lastState & SC_BTN_MODE)) {
        if (currentMillis - lastDebounceTime > 50) {
            if (currentState & SC_BTN_MODE) {
                send_joy_action(ESP_JOY1MODE, true);
            } else {
                send_joy_action(ESP_JOY1MODE, false);
            }
            lastDebounceTime = currentMillis;
        }
    }

    lastState = currentState;  // Actualiza el estado anterior

    // // Configuración del botón de prueba
    // bool current_state = !gpio_get(BUTTON_TESTING);
    // static bool button_pressed = false;

    // if (current_state && !button_pressed) {
    //     button_pressed = true;
    //     send_joy_action(ESP_JOY1LEFT, true);
    // } else if (!current_state && button_pressed) {
    //     button_pressed = false;
    //     send_joy_action(ESP_JOY1LEFT, false);
    // }
}
    
    
    
//     void check_joystick() {
//     static GamePad gamepad;
//     static bool initialized = false;
//     static uint16_t lastState = 0; // Guarda el estado anterior
//     static uint32_t debounceTimes[16] = {0}; // Array para almacenar los tiempos de debounce de cada botón
//     static uint32_t last_button_check = 0;  // Última verificación para el debounce global

//     // Inicialización del GamePad
//     if (!initialized) {
//         GamePad_init(&gamepad, 7, 2, 3, 4, 5, 6, 8);
//         initialized = true;
//     }

//     uint16_t currentState = GamePad_getState(&gamepad);
//     uint32_t currentMillis = time_us_32();  // Obtener el tiempo actual en microsegundos

//     // Definimos el tiempo de debounce (50 ms = 50000 microsegundos)
//     #define DEBOUNCE_DELAY 50000  // 50 ms en microsegundos

//     // Lista de scancodes y las acciones que corresponden
//     uint16_t buttonScancodes[] = {SC_BTN_UP, SC_BTN_DOWN, SC_BTN_LEFT, SC_BTN_RIGHT, 
//                                   SC_BTN_START, SC_BTN_A, SC_BTN_B, SC_BTN_C, 
//                                   SC_BTN_X, SC_BTN_Y, SC_BTN_Z, SC_BTN_1, 
//                                   SC_BTN_2, SC_BTN_MODE};

//     uint8_t joyActions[] = {ESP_JOY1UP, ESP_JOY1DOWN, ESP_JOY1LEFT, ESP_JOY1RIGHT, 
//                             ESP_JOY1START, ESP_JOY1A, ESP_JOY1B, ESP_JOY1C, 
//                             ESP_JOY1X, ESP_JOY1Y, ESP_JOY1Z, ESP_JOY1A, 
//                             ESP_JOY1Z, ESP_JOY1MODE};

//     // Procesamos cada botón en el array
//     for (int i = 0; i < sizeof(buttonScancodes) / sizeof(buttonScancodes[0]); i++) {
//         // Si el estado del botón cambió
//         if ((currentState & buttonScancodes[i]) != (lastState & buttonScancodes[i])) {
//             // Si ha pasado el tiempo de debounce, se procesa el botón
//             if (currentMillis - debounceTimes[i] >= DEBOUNCE_DELAY) {
//                 // Si el botón está presionado, enviamos el scancode de presionar
//                 if (currentState & buttonScancodes[i]) {
//                     send_joy_action(joyActions[i], true);
//                 } else {
//                     send_joy_action(joyActions[i], false);
//                 }
//                 debounceTimes[i] = currentMillis; // Actualizar el tiempo de debounce del botón
//             }
//         }
//     }

//     // Configuración de un control de debounce general para todos los botones
//     if (currentMillis - last_button_check >= DEBOUNCE_DELAY) {
//         last_button_check = currentMillis; // Actualiza la última verificación
//     }

//     // Actualizamos el estado anterior para el próximo ciclo
//     lastState = currentState;
// }
    
    
    
    
    
    
    // // TESTING BUTTON
    // bool current_state = !gpio_get(BUTTON_TESTING);
    // if (current_state && !button_pressed) {
    //     button_pressed = true;
    //     send_joy_action(ESP_JOY1LEFT, true);
    // } 
    // else if (!current_state && button_pressed) {
    //     button_pressed = false;
    //     send_joy_action(ESP_JOY1LEFT, false);
    // }

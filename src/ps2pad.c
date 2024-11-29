
// #include <stdbool.h>
// #include "pico/stdlib.h"
// #include <stdio.h>
// #include "tusb.h"
// #include "ps2out.h"
// #include "ps2in.h"
// #include "hardware/watchdog.h"
// #include "scancodesets.h"
// #include "ps2pad.h"

// //START GAMEPAD

// // DB9 Pin 1 --> GPIO 2 (PIN 4) --> UP
// // DB9 Pin 2 --> GPIO 3 (PIN 5) --> DOWN
// // DB9 Pin 3 --> GPIO 4 (PIN 6) --> LEFT
// // DB9 Pin 4 --> GPIO 5 (PIN 7) --> RIGHT
// // DB9 Pin 5 --> +5V    (PIN 49) -> -----
// // DB9 Pin 6 --> GPIO 6 (PIN 9) --> FIRE
// // DB9 Pin 7 --> GPIO 7 (PIN 10) -> SELECT
// // DB9 Pin 8 --> GND    (PIN 8) --> -----
// // DB9 Pin 9 --> GPIO 8 (PIN 11) -> 

// #define GAMEPAD_SELECT 7
// #define GAMEPAD_MODE 8 // Es mode???

// #define GAMEPAD_UP 2 
// #define GAMEPAD_DOWN 3
// #define GAMEPAD_LEFT 4 
// #define GAMEPAD_RIGHT 5 
// #define GAMEPAD_FIRE 6 

// #define ESP_JOY1LEFT 0x40
// #define ESP_JOY1RIGHT 0x41
// #define ESP_JOY1UP 0x42
// #define ESP_JOY1DOWN 0x43
// #define ESP_JOYFIRE 0x46

// #define SCAN_CODE_SET_F0 0xf0
// #define SCAN_CODE_SET_E2 0xe2
// static uint32_t milliseconds = 0;
// #define SLEEP_TIME 25

// static GamePad gamepad;
// static bool initialized = false;
// static uint16_t lastState = 0; // Declara lastState para almacenar el estado anterior
// static uint32_t lastDebounceTime = 0;  // Almacena el tiempo del último rebote

// // ************************
// // GAMEPAD CONTROL
// // ************************
// // Función callback que se ejecuta cada milisegundo
// // Cambiar la firma para que coincida con alarm_callback_t
// int64_t timer_callback(int64_t alarm_time, void *user_data) {
//     milliseconds++;  // Incrementa cada vez que el temporizador se desborda (1ms)
//     return 0;  // Retorna 0 para indicar que el temporizador no se repite
// }


// // Función millis() para obtener el tiempo transcurrido en milisegundos
// uint32_t millis() {
//     return milliseconds;
// }

// // Inicializa el GamePad con los pines correspondientes
// void GamePad_init(GamePad* gamepad, uint db9_pin_7, uint db9_pin_1, uint db9_pin_2, uint db9_pin_3, uint db9_pin_4, uint db9_pin_6, uint db9_pin_9) {
//     gamepad->_selectPin = db9_pin_7;

//     gamepad->_inputPins[0] = db9_pin_1;
//     gamepad->_inputPins[1] = db9_pin_2;
//     gamepad->_inputPins[2] = db9_pin_3;
//     gamepad->_inputPins[3] = db9_pin_4;
//     gamepad->_inputPins[4] = db9_pin_6;
//     gamepad->_inputPins[5] = db9_pin_9;

//     gpio_init(gamepad->_selectPin);
//     gpio_set_dir(gamepad->_selectPin, GPIO_OUT);
//     gpio_put(gamepad->_selectPin, 1);

//     for (int i = 0; i < SC_INPUT_PINS; i++) {
//         gpio_init(gamepad->_inputPins[i]);
//         gpio_set_dir(gamepad->_inputPins[i], GPIO_IN);
//         gpio_pull_up(gamepad->_inputPins[i]);
//     }

//     gamepad->_currentState = 0;
//     gamepad->_sixButtonMode = false;
//     gamepad->_lastReadTime = to_ms_since_boot(get_absolute_time());
// }

// // Devuelve el estado actual de los botones
// uint16_t GamePad_getState(GamePad* gamepad) {
//     uint32_t currentTime = to_ms_since_boot(get_absolute_time());

//     if ((currentTime - gamepad->_lastReadTime) < SC_READ_DELAY_MS) {
//         return gamepad->_currentState;
//     }

//     uint32_t savedInterrupts = save_and_disable_interrupts();
//     gamepad->_currentState = 0;

//     for (int cycle = 0; cycle < SC_CYCLES; cycle++) {
//         GamePad_readCycle(gamepad, cycle);
//     }

//     if (!(gamepad->_currentState & SC_CTL_ON)) {
//         gamepad->_sixButtonMode = false;
//     }

//     restore_interrupts(savedInterrupts);
//     gamepad->_lastReadTime = currentTime;
//     return gamepad->_currentState;
// }

// // Lee un ciclo específico para obtener el estado de los botones
// void GamePad_readCycle(GamePad* gamepad, int cycle) {
//     gpio_put(gamepad->_selectPin, cycle % 2);

//     switch (cycle) {
//         case 2:
//             gamepad->_currentState |= (gpio_get(gamepad->_inputPins[2]) == 0 && gpio_get(gamepad->_inputPins[3]) == 0) * SC_CTL_ON;
            
//             if (gamepad->_currentState & SC_CTL_ON) {
//                 if (gpio_get(gamepad->_inputPins[4]) == 0) { gamepad->_currentState |= SC_BTN_A; }
//                 if (gpio_get(gamepad->_inputPins[5]) == 0) { gamepad->_currentState |= SC_BTN_START; }
//             }
//             break;

//         case 3:
//             if (gpio_get(gamepad->_inputPins[0]) == 0) { gamepad->_currentState |= SC_BTN_UP; }
//             if (gpio_get(gamepad->_inputPins[1]) == 0) { gamepad->_currentState |= SC_BTN_DOWN; }
//             if (gpio_get(gamepad->_inputPins[2]) == 0) { gamepad->_currentState |= SC_BTN_LEFT; }
//             if (gpio_get(gamepad->_inputPins[3]) == 0) { gamepad->_currentState |= SC_BTN_RIGHT; }
//             if (gpio_get(gamepad->_inputPins[4]) == 0) { gamepad->_currentState |= SC_BTN_B; }
//             if (gpio_get(gamepad->_inputPins[5]) == 0) { gamepad->_currentState |= SC_BTN_C; }
//             break;

//         case 4:
//             gamepad->_sixButtonMode = (gpio_get(gamepad->_inputPins[0]) == 0 && gpio_get(gamepad->_inputPins[1]) == 0);
//             break;

//         case 5:
//             if (gamepad->_sixButtonMode) {
//                 if (gpio_get(gamepad->_inputPins[0]) == 0) { gamepad->_currentState |= SC_BTN_Z; }
//                 if (gpio_get(gamepad->_inputPins[1]) == 0) { gamepad->_currentState |= SC_BTN_Y; }
//                 if (gpio_get(gamepad->_inputPins[2]) == 0) { gamepad->_currentState |= SC_BTN_X; }
//                 if (gpio_get(gamepad->_inputPins[3]) == 0) { gamepad->_currentState |= SC_BTN_MODE; }
//             }
//             break;
//     }
// }


// void gamepad_controls() {
    


//     // Inicializa el GamePad solo una vez
//     if (!initialized) {
//         GamePad_init(&gamepad, GAMEPAD_SELECT, GAMEPAD_UP, GAMEPAD_DOWN, GAMEPAD_LEFT, GAMEPAD_RIGHT, GAMEPAD_FIRE, 8);
//         initialized = true;
//     }

//     uint16_t currentState = GamePad_getState(&gamepad);

//     // Verificar el cambio de estado para cada botón, evitando el rebote
//     uint32_t currentMillis = millis();

//     // SC_BTN_UP
//     if ((currentState & SC_BTN_UP) != (lastState & SC_BTN_UP)) {
//         if (currentMillis - lastDebounceTime > SLEEP_TIME) {  // 50ms de debounce
//             if (currentState & SC_BTN_UP) {
//                 kb_send_key_gamepad_control(2,true); //UP
//             } else {
//                 kb_send_key_gamepad_control(2,false); //UP
//             }
//             lastDebounceTime = currentMillis;
//         }
//     }

//     // SC_BTN_DOWN
//     if ((currentState & SC_BTN_DOWN) != (lastState & SC_BTN_DOWN)) {
//         if (currentMillis - lastDebounceTime > SLEEP_TIME) {
//             if (currentState & SC_BTN_DOWN) {
//                 kb_send_key_gamepad_control(3,true); //DOWN
//             } else {
//                 kb_send_key_gamepad_control(3,false); //DOWN
//             }
//             lastDebounceTime = currentMillis;
//         }
//     }

//     // SC_BTN_LEFT
//     if ((currentState & SC_BTN_LEFT) != (lastState & SC_BTN_LEFT)) {
//         if (currentMillis - lastDebounceTime > SLEEP_TIME) {
//             if (currentState & SC_BTN_LEFT) {
//                 kb_send_key_gamepad_control(0,true); //LEFT
//             } else {
//                 kb_send_key_gamepad_control(0,false); //LEFT
//             }
//             lastDebounceTime = currentMillis;
//         }
//     }

//     // SC_BTN_RIGHT
//     if ((currentState & SC_BTN_RIGHT) != (lastState & SC_BTN_RIGHT)) {
//         if (currentMillis - lastDebounceTime > SLEEP_TIME) {
//             if (currentState & SC_BTN_RIGHT) {
//                 kb_send_key_gamepad_control(1,true); //RIGHT
//             } else {
//                 kb_send_key_gamepad_control(1,false); //RIGHT
//             }
//             lastDebounceTime = currentMillis;
//         }
//     }

//     // SC_BTN_START
//     if ((currentState & SC_BTN_START) != (lastState & SC_BTN_START)) {
//         if (currentMillis - lastDebounceTime > SLEEP_TIME) {
//             if (currentState & SC_BTN_START) {
//                 kb_send_key_gamepad_control(4,true); //START
//             } else {
//                 kb_send_key_gamepad_control(4,false); //START
//             }
//             lastDebounceTime = currentMillis;
//         }
//     }

//     // SC_BTN_A
//     if ((currentState & SC_BTN_A) != (lastState & SC_BTN_A)) {
//         if (currentMillis - lastDebounceTime > SLEEP_TIME) {
//             if (currentState & SC_BTN_A) {
//                 kb_send_key_gamepad_control(6,true); //FIRE
//             } else {
//                 kb_send_key_gamepad_control(6,false); //FIRE
//             }
//             lastDebounceTime = currentMillis;
//         }
//     }

//     // SC_BTN_B
//     if ((currentState & SC_BTN_B) != (lastState & SC_BTN_B)) {
//         if (currentMillis - lastDebounceTime > SLEEP_TIME) {
//             if (currentState & SC_BTN_B) {
//                 kb_send_key_gamepad_control(7,true); //BUTTON_B
//             } else {
//                 kb_send_key_gamepad_control(7,false); //BUTTON_B
//             }
//             lastDebounceTime = currentMillis;
//         }
//     }

//     // SC_BTN_C
//     if ((currentState & SC_BTN_C) != (lastState & SC_BTN_C)) {
//         if (currentMillis - lastDebounceTime > SLEEP_TIME) {
//             if (currentState & SC_BTN_C) {
//                 kb_send_key_gamepad_control(8,true); //BUTTON_C
//             } else {
//                 kb_send_key_gamepad_control(8,false); //BUTTON_C
//             }
//             lastDebounceTime = currentMillis;
//         }
//     }

//     // SC_BTN_X
//     if ((currentState & SC_BTN_X) != (lastState & SC_BTN_X)) {
//         if (currentMillis - lastDebounceTime > SLEEP_TIME) {
//             if (currentState & SC_BTN_X) {
//                 kb_send_key_gamepad_control(9,true); //BUTTON_X
//             } else {
//                 kb_send_key_gamepad_control(9,false); //BUTTON_X
//             }
//             lastDebounceTime = currentMillis;
//         }
//     }

//     // SC_BTN_Y
//     if ((currentState & SC_BTN_Y) != (lastState & SC_BTN_Y)) {
//         if (currentMillis - lastDebounceTime > 50) {
//             if (currentState & SC_BTN_Y) {
//                 kb_send_key_gamepad_control(10,true); //BUTTON_Y
//             } else {
//                 kb_send_key_gamepad_control(10,false); //BUTTON_Y
//             }
//             lastDebounceTime = currentMillis;
//         }
//     }

//     // SC_BTN_Z
//     if ((currentState & SC_BTN_Z) != (lastState & SC_BTN_Z)) {
//         if (currentMillis - lastDebounceTime > 25) {
//             if (currentState & SC_BTN_Z) {
//                 kb_send_key_gamepad_control(11,true); //BUTTON_Z
//             } else {
//                 kb_send_key_gamepad_control(11,false); //BUTTON_Z
//             }
//             lastDebounceTime = currentMillis;
//         }
//     }

//     // SC_BTN_1
//     if ((currentState & SC_BTN_1) != (lastState & SC_BTN_1)) {
//         if (currentMillis - lastDebounceTime > 25) {
//             if (currentState & SC_BTN_1) {
//                 kb_send_key_gamepad_control(6,true); //BUTTON_1
//             } else {
//                 kb_send_key_gamepad_control(6,false); //BUTTON_C
//             }
//             lastDebounceTime = currentMillis;
//         }
//     }

//     // SC_BTN_2
//     if ((currentState & SC_BTN_2) != (lastState & SC_BTN_2)) {
//         if (currentMillis - lastDebounceTime > 50) {
//             if (currentState & SC_BTN_2) {
//                 kb_send_key_gamepad_control(11,true); //BUTTON_Z
//             } else {
//                 kb_send_key_gamepad_control(11,false); //BUTTON_Z
//             }
//             lastDebounceTime = currentMillis;
//         }
//     }

//     // SC_BTN_MODE
//     if ((currentState & SC_BTN_MODE) != (lastState & SC_BTN_MODE)) {
//         if (currentMillis - lastDebounceTime > 50) {
//             if (currentState & SC_BTN_MODE) {
//                 kb_send_key_gamepad_control(5,true); //MODE
//             } else {
//                 kb_send_key_gamepad_control(5,false); //FIRE
//             }
//             lastDebounceTime = currentMillis;
//         }
//     }

//     lastState = currentState;  // Actualiza el estado anterior

// }

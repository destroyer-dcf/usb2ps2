#include "GamePad.h"
#include "hardware/gpio.h"

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

// int GamePad::detectJoystickType() {
//     // Establecer el pin SELECT en LOW y leer las respuestas
//     digitalWrite(_selectPin, LOW);
//     delay(10);
//     bool segaResponseLow = (digitalRead(_inputPins[4]) == LOW || digitalRead(_inputPins[5]) == LOW);

//     // Establecer el pin SELECT en HIGH y leer las respuestas
//     digitalWrite(_selectPin, HIGH);
//     delay(10);
//     bool segaResponseHigh = (digitalRead(_inputPins[0]) == LOW || digitalRead(_inputPins[1]) == LOW);

//     if (segaResponseLow && segaResponseHigh) {
//         return 1; // Sega Joystick detectado
//     } else if (!segaResponseLow && !segaResponseHigh) {
//         return 2; // Amstrad Joystick detectado o no responde a SELECT
//     } else {
//         return 0; // Joystick no detectado o desconocido
//     }
// }
// #include <stdbool.h>
// #include "pico/stdlib.h"

// // ps2joy.h
// #ifndef PS2JOY_H
// #define PS2JOY_H

// #include <stdbool.h>
// #include "pico/stdlib.h"

// // Definición de constantes y enumeraciones para el estado de los botones
// enum {
//     SC_CTL_ON    = 1,    // El controlador está conectado
//     SC_BTN_UP    = 2,
//     SC_BTN_DOWN  = 4,
//     SC_BTN_LEFT  = 8,
//     SC_BTN_RIGHT = 16,
//     SC_BTN_START = 32,
//     SC_BTN_A     = 64,
//     SC_BTN_B     = 128,
//     SC_BTN_C     = 256,
//     SC_BTN_X     = 512,
//     SC_BTN_Y     = 1024,
//     SC_BTN_Z     = 2048,
//     SC_BTN_MODE  = 4096,
//     SC_BTN_1     = 128,  // Compatibilidad con Master System
//     SC_BTN_2     = 256   // Compatibilidad con Master System
// };

// // Definiciones para la configuración de pines y ciclos
// #define SC_INPUT_PINS 6
// #define SC_CYCLES 8
// #define SC_READ_DELAY_MS 5 // Debe ser >= 3 para dar tiempo a que se reinicie el controlador de 6 botones
// static bool initialized = false;
// // Definición de la estructura GamePad
// typedef struct {
//     uint _selectPin;                // Pin de selección de salida
//     uint _inputPins[SC_INPUT_PINS]; // Array de pines de entrada
//     uint16_t _currentState;         // Estado actual de los botones
//     uint32_t _lastReadTime;         // Última vez que se leyó el estado
//     bool _sixButtonMode;            // Modo de seis botones activado/desactivado
// } GamePad;


// extern bool button_pressed;
// void kb_send(u8 scancode); // Declaración de kb_send
// void GamePad_init(GamePad* gamepad, uint db9_pin_7, uint db9_pin_1, uint db9_pin_2, uint db9_pin_3, uint db9_pin_4, uint db9_pin_6, uint db9_pin_9);
// uint16_t GamePad_getState(GamePad* gamepad);
// int GamePad_detectJoystickType(GamePad* gamepad); // Nueva función para detectar el tipo de joystick (opcional)
// void GamePad_readCycle(GamePad* gamepad, int cycle);

// #endif // PS2JOY_H
// typedef uint8_t u8;
// void kb_send_key_gamepad_control(u8 key, bool is_key_pressed);
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


#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

void tuh_kb_set_leds(u8 leds);
void kb_usb_receive(u8 const* report, u16 len);
bool kb_task();
void kb_init(u8 gpio_out, u8 gpio_in);
void kb_set_defaults();

void ms_init(u8 gpio_out, u8 gpio_in);
void ms_usb_receive(u8 const* report);
bool ms_task();
#ifndef PS2X2PICO_H
#define PS2X2PICO_H
enum {
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
// static bool initialized = false;
// Definición de la estructura GamePad
typedef struct {
    uint _selectPin;                // Pin de selección de salida
    uint _inputPins[SC_INPUT_PINS]; // Array de pines de entrada
    uint16_t _currentState;         // Estado actual de los botones
    uint32_t _lastReadTime;         // Última vez que se leyó el estado
    bool _sixButtonMode;            // Modo de seis botones activado/desactivado
} GamePad;


extern bool button_pressed;
void kb_send(u8 scancode); // Declaración de kb_send
void GamePad_init(GamePad* gamepad, uint db9_pin_7, uint db9_pin_1, uint db9_pin_2, uint db9_pin_3, uint db9_pin_4, uint db9_pin_6, uint db9_pin_9);
uint16_t GamePad_getState(GamePad* gamepad);
int GamePad_detectJoystickType(GamePad* gamepad); // Nueva función para detectar el tipo de joystick (opcional)
void GamePad_readCycle(GamePad* gamepad, int cycle);


typedef uint8_t u8;
void kb_send_key_gamepad_control(u8 key, bool is_key_pressed);
#endif // PS2X2PICO_H
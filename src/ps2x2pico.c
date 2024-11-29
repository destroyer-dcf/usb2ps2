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
// #include "hardware/i2c.h"
#include "hardware/watchdog.h"
#include "hardware/gpio.h"
#include "bsp/board_api.h"
#include "tusb.h"
#include "ps2x2pico.h"
// #include "ps2pad.h"

static void print_utf16(uint16_t *temp_buf, size_t buf_len);
void print_device_descriptor(tuh_xfer_t* xfer);

u8 kb_addr = 0;
u8 kb_inst = 0;
u8 kb_leds = 0;  
char device_str[50];
char manufacturer_str[50];



//START GAMEPAD

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
#define GAMEPAD_MODE 8 // Es mode???

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
static uint32_t milliseconds = 0;
#define SLEEP_TIME 25

static GamePad gamepad;
static bool initialized = false;
static uint16_t lastState = 0; // Declara lastState para almacenar el estado anterior
static uint32_t lastDebounceTime = 0;  // Almacena el tiempo del último rebote

// ************************
// GAMEPAD CONTROL
// ************************
// Función callback que se ejecuta cada milisegundo
// Cambiar la firma para que coincida con alarm_callback_t
int64_t timer_callback(int64_t alarm_time, void *user_data) {
    milliseconds++;  // Incrementa cada vez que el temporizador se desborda (1ms)
    return 0;  // Retorna 0 para indicar que el temporizador no se repite
}


// Función millis() para obtener el tiempo transcurrido en milisegundos
uint32_t millis() {
    return milliseconds;
}

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


void gamepad_controls() {
    



    uint16_t currentState = GamePad_getState(&gamepad);

    // Verificar el cambio de estado para cada botón, evitando el rebote
    uint32_t currentMillis = millis();

    // SC_BTN_UP
    if ((currentState & SC_BTN_UP) != (lastState & SC_BTN_UP)) {
        if (currentMillis - lastDebounceTime > SLEEP_TIME) {  // 50ms de debounce
            if (currentState & SC_BTN_UP) {
                kb_send_key_gamepad_control(2,true); //UP
            } else {
                kb_send_key_gamepad_control(2,false); //UP
            }
            lastDebounceTime = currentMillis;
        }
    }

    // SC_BTN_DOWN
    if ((currentState & SC_BTN_DOWN) != (lastState & SC_BTN_DOWN)) {
        if (currentMillis - lastDebounceTime > SLEEP_TIME) {
            if (currentState & SC_BTN_DOWN) {
                kb_send_key_gamepad_control(3,true); //DOWN
            } else {
                kb_send_key_gamepad_control(3,false); //DOWN
            }
            lastDebounceTime = currentMillis;
        }
    }

    // SC_BTN_LEFT
    if ((currentState & SC_BTN_LEFT) != (lastState & SC_BTN_LEFT)) {
        if (currentMillis - lastDebounceTime > SLEEP_TIME) {
            if (currentState & SC_BTN_LEFT) {
                kb_send_key_gamepad_control(0,true); //LEFT
            } else {
                kb_send_key_gamepad_control(0,false); //LEFT
            }
            lastDebounceTime = currentMillis;
        }
    }

    // SC_BTN_RIGHT
    if ((currentState & SC_BTN_RIGHT) != (lastState & SC_BTN_RIGHT)) {
        if (currentMillis - lastDebounceTime > SLEEP_TIME) {
            if (currentState & SC_BTN_RIGHT) {
                kb_send_key_gamepad_control(1,true); //RIGHT
            } else {
                kb_send_key_gamepad_control(1,false); //RIGHT
            }
            lastDebounceTime = currentMillis;
        }
    }

    // SC_BTN_START
    if ((currentState & SC_BTN_START) != (lastState & SC_BTN_START)) {
        if (currentMillis - lastDebounceTime > SLEEP_TIME) {
            if (currentState & SC_BTN_START) {
                kb_send_key_gamepad_control(4,true); //START
            } else {
                kb_send_key_gamepad_control(4,false); //START
            }
            lastDebounceTime = currentMillis;
        }
    }

    // SC_BTN_A
    if ((currentState & SC_BTN_A) != (lastState & SC_BTN_A)) {
        if (currentMillis - lastDebounceTime > SLEEP_TIME) {
            if (currentState & SC_BTN_A) {
                kb_send_key_gamepad_control(6,true); //FIRE
            } else {
                kb_send_key_gamepad_control(6,false); //FIRE
            }
            lastDebounceTime = currentMillis;
        }
    }

    // SC_BTN_B
    if ((currentState & SC_BTN_B) != (lastState & SC_BTN_B)) {
        if (currentMillis - lastDebounceTime > SLEEP_TIME) {
            if (currentState & SC_BTN_B) {
                kb_send_key_gamepad_control(7,true); //BUTTON_B
            } else {
                kb_send_key_gamepad_control(7,false); //BUTTON_B
            }
            lastDebounceTime = currentMillis;
        }
    }

    // SC_BTN_C
    if ((currentState & SC_BTN_C) != (lastState & SC_BTN_C)) {
        if (currentMillis - lastDebounceTime > SLEEP_TIME) {
            if (currentState & SC_BTN_C) {
                kb_send_key_gamepad_control(8,true); //BUTTON_C
            } else {
                kb_send_key_gamepad_control(8,false); //BUTTON_C
            }
            lastDebounceTime = currentMillis;
        }
    }

    // SC_BTN_X
    if ((currentState & SC_BTN_X) != (lastState & SC_BTN_X)) {
        if (currentMillis - lastDebounceTime > SLEEP_TIME) {
            if (currentState & SC_BTN_X) {
                kb_send_key_gamepad_control(9,true); //BUTTON_X
            } else {
                kb_send_key_gamepad_control(9,false); //BUTTON_X
            }
            lastDebounceTime = currentMillis;
        }
    }

    // SC_BTN_Y
    if ((currentState & SC_BTN_Y) != (lastState & SC_BTN_Y)) {
        if (currentMillis - lastDebounceTime > 50) {
            if (currentState & SC_BTN_Y) {
                kb_send_key_gamepad_control(10,true); //BUTTON_Y
            } else {
                kb_send_key_gamepad_control(10,false); //BUTTON_Y
            }
            lastDebounceTime = currentMillis;
        }
    }

    // SC_BTN_Z
    if ((currentState & SC_BTN_Z) != (lastState & SC_BTN_Z)) {
        if (currentMillis - lastDebounceTime > 25) {
            if (currentState & SC_BTN_Z) {
                kb_send_key_gamepad_control(11,true); //BUTTON_Z
            } else {
                kb_send_key_gamepad_control(11,false); //BUTTON_Z
            }
            lastDebounceTime = currentMillis;
        }
    }

    // SC_BTN_1
    if ((currentState & SC_BTN_1) != (lastState & SC_BTN_1)) {
        if (currentMillis - lastDebounceTime > 25) {
            if (currentState & SC_BTN_1) {
                kb_send_key_gamepad_control(6,true); //BUTTON_1
            } else {
                kb_send_key_gamepad_control(6,false); //BUTTON_C
            }
            lastDebounceTime = currentMillis;
        }
    }

    // SC_BTN_2
    if ((currentState & SC_BTN_2) != (lastState & SC_BTN_2)) {
        if (currentMillis - lastDebounceTime > 50) {
            if (currentState & SC_BTN_2) {
                kb_send_key_gamepad_control(11,true); //BUTTON_Z
            } else {
                kb_send_key_gamepad_control(11,false); //BUTTON_Z
            }
            lastDebounceTime = currentMillis;
        }
    }

    // SC_BTN_MODE
    if ((currentState & SC_BTN_MODE) != (lastState & SC_BTN_MODE)) {
        if (currentMillis - lastDebounceTime > 50) {
            if (currentState & SC_BTN_MODE) {
                kb_send_key_gamepad_control(5,true); //MODE
            } else {
                kb_send_key_gamepad_control(5,false); //FIRE
            }
            lastDebounceTime = currentMillis;
        }
    }

    lastState = currentState;  // Actualiza el estado anterior

}





//STAR GAMEPAD
uint32_t last_button_check = 0; 
//END GAMEPAD

void tuh_kb_set_leds(u8 leds) {
  if(kb_addr) {
    kb_leds = leds;
    printf("HID(%d,%d): LEDs = %d\n", kb_addr, kb_inst, kb_leds);
    tuh_hid_set_report(kb_addr, kb_inst, 0, HID_REPORT_TYPE_OUTPUT, &kb_leds, sizeof(kb_leds));
  }
}

#define LANGUAGE_ID 0x0409 // English

void tuh_hid_mount_cb(u8 dev_addr, u8 instance, u8 const* desc_report, u16 desc_len) {
  // This happens if report descriptor length > CFG_TUH_ENUMERATION_BUFSIZE.
  // Consider increasing #define CFG_TUH_ENUMERATION_BUFSIZE 256 in tusb_config.h
  if (desc_report == NULL && desc_len == 0) {
    printf("WARNING: HID(%d,%d) skipped!\n",dev_addr, instance);
    return;
  }

  hid_interface_protocol_enum_t hid_if_proto = tuh_hid_interface_protocol(dev_addr, instance);
  uint16_t vid, pid;
  tuh_vid_pid_get(dev_addr, &vid, &pid);

  char* hidprotostr;
  switch (hid_if_proto) {
    case HID_ITF_PROTOCOL_NONE:
      hidprotostr = "NONE";
      break;
    case HID_ITF_PROTOCOL_KEYBOARD:
      hidprotostr = "KEYBOARD";
      break;
    case HID_ITF_PROTOCOL_MOUSE:
      hidprotostr = "MOUSE";
      //tuh_hid_set_protocol(dev_addr, instance, HID_PROTOCOL_REPORT);
      break;
    default:
      hidprotostr = "UNKNOWN";
      break;
  };

  printf("HID(%d,%d,%s) mounted\n", dev_addr, instance, hidprotostr);
  printf(" ID: %04x:%04x\n", vid, pid);
 
  uint16_t temp_buf[128];

  printf(" Manufacturer: ");
  if (XFER_RESULT_SUCCESS == tuh_descriptor_get_manufacturer_string_sync(dev_addr, LANGUAGE_ID, temp_buf, sizeof(temp_buf)) )
  {
    print_utf16(temp_buf, TU_ARRAY_SIZE(temp_buf));
  }
  printf("\n");

  printf(" Product:      ");
  if (XFER_RESULT_SUCCESS == tuh_descriptor_get_product_string_sync(dev_addr, LANGUAGE_ID, temp_buf, sizeof(temp_buf)))
  {
    print_utf16(temp_buf, TU_ARRAY_SIZE(temp_buf));
  }
  printf("\n\n");

  if (hid_if_proto == HID_ITF_PROTOCOL_KEYBOARD || hid_if_proto == HID_ITF_PROTOCOL_MOUSE) {
    if (!tuh_hid_receive_report(dev_addr, instance)) {
      printf("ERROR: Could not register for HID(%d,%d,%s)!\n", dev_addr, instance, hidprotostr);
    } else {
      printf("HID(%d,%d,%s) registered for reports\n", dev_addr, instance, hidprotostr);
      if (hid_if_proto == HID_ITF_PROTOCOL_KEYBOARD) {
          // TODO: This needs to be addressed if we want to have multiple connected kbds working correctly! 
          // Only relevant for KB LEDS though.
          // Could be a list of all connected kbds, so we could set the LEDs on each.
          kb_addr = dev_addr;
          kb_inst = instance;
      }
      board_led_write(1);
    }
  }
}

void tuh_hid_umount_cb(u8 dev_addr, u8 instance) {
  printf("HID(%d,%d) unmounted\n", dev_addr, instance);
  board_led_write(0);
  
  if(dev_addr == kb_addr && instance == kb_inst) {
    kb_addr = 0;
    kb_inst = 0;
  }
}

void tuh_hid_report_received_cb(u8 dev_addr, u8 instance, u8 const* report, u16 len) {

  switch(tuh_hid_interface_protocol(dev_addr, instance)) {
    case HID_ITF_PROTOCOL_KEYBOARD:
      #ifdef TRACE
      printf("HID_KB(%d,%d): r[2..7]={0x%x,0x%x,0x%x,0x%x,0x%x,0x%x},r[0]=0x%x,l=%d\n",
       dev_addr, instance, 
       report[2], report[3], report[4], report[5], report[6], report[7], 
       report[0], len);
      #else
      #ifdef KB_DEBUG
      printf("HID_KB(%d,%d): r[2]=0x%x,r[0]=0x%x,l=%d\n", dev_addr, instance, report[2], report[0], len);
      #endif
      #endif
      kb_usb_receive(report, len);
      tuh_hid_receive_report(dev_addr, instance);
    break;
    
    case HID_ITF_PROTOCOL_MOUSE:
      #ifdef TRACE
      printf("HID_MS(%d,%d): r[2..7]={0x%x,0x%x,0x%x,0x%x,0x%x,0x%x},r[0]=0x%x,l=%d\n",
       dev_addr, instance, 
       report[2], report[3], report[4], report[5], report[6], report[7], 
       report[0], len);
      #else
      #ifdef MS_DEBUG
      printf("HID_MS(%d,%d)\n", dev_addr, instance);
      #endif
      #endif
      ms_usb_receive(report);
      tuh_hid_receive_report(dev_addr, instance);
    break;
  }
}

void setup() {
    gpio_init(GAMEPAD_LEFT);
    gpio_set_dir(GAMEPAD_LEFT, GPIO_IN);
    gpio_pull_up(GAMEPAD_LEFT);

    gpio_init(GAMEPAD_RIGHT);
    gpio_set_dir(GAMEPAD_RIGHT, GPIO_IN);
    gpio_pull_up(GAMEPAD_RIGHT);

    gpio_init(GAMEPAD_UP);
    gpio_set_dir(GAMEPAD_UP, GPIO_IN);
    gpio_pull_up(GAMEPAD_UP);

    gpio_init(GAMEPAD_DOWN);
    gpio_set_dir(GAMEPAD_DOWN, GPIO_IN);
    gpio_pull_up(GAMEPAD_DOWN);

    gpio_init(GAMEPAD_FIRE);
    gpio_set_dir(GAMEPAD_FIRE, GPIO_IN);
    gpio_pull_up(GAMEPAD_FIRE);
}


void main() {
    board_init();
    printf("\n%s-%s\n", PICO_PROGRAM_NAME, PICO_PROGRAM_VERSION_STRING);

    gpio_init(LVOUT);
    gpio_init(LVIN);
    gpio_set_dir(LVOUT, GPIO_OUT);
    gpio_set_dir(LVIN, GPIO_OUT);
    gpio_put(LVOUT, 1);
    gpio_put(LVIN, 1);
    
    tusb_init();
    kb_init(KBOUT, KBIN);
    ms_init(MSOUT, MSIN);

    // setup();
      // Inicializa el GamePad solo una vez
      // Inicializa el GamePad solo una vez
      if (!initialized) {
          GamePad_init(&gamepad, GAMEPAD_SELECT, GAMEPAD_UP, GAMEPAD_DOWN, GAMEPAD_LEFT, GAMEPAD_RIGHT, GAMEPAD_FIRE, 8);
          initialized = true;
      }

    while (1) {

        tuh_task();
        kb_task();
        ms_task();


        uint32_t current_time = time_us_32();  // Obtener tiempo actual en microsegundos
        if (current_time - last_button_check >= DEBOUNCE_TIME * 1000) {
            gamepad_controls();
            last_button_check = current_time;  // Actualiza la última verificación
        }
        
    }
    
}

void reset() {
  printf("\n\n *** PANIC via tinyusb: watchdog reset!\n\n");
  watchdog_enable(100, false);
}

//--------------------------------------------------------------------+
// String Descriptor Helper
//--------------------------------------------------------------------+

static void _convert_utf16le_to_utf8(const uint16_t *utf16, size_t utf16_len, uint8_t *utf8, size_t utf8_len) {
    // TODO: Check for runover.
    (void)utf8_len;
    // Get the UTF-16 length out of the data itself.

    for (size_t i = 0; i < utf16_len; i++) {
        uint16_t chr = utf16[i];
        if (chr < 0x80) {
            *utf8++ = chr & 0xffu;
        } else if (chr < 0x800) {
            *utf8++ = (uint8_t)(0xC0 | (chr >> 6 & 0x1F));
            *utf8++ = (uint8_t)(0x80 | (chr >> 0 & 0x3F));
        } else {
            // TODO: Verify surrogate.
            *utf8++ = (uint8_t)(0xE0 | (chr >> 12 & 0x0F));
            *utf8++ = (uint8_t)(0x80 | (chr >> 6 & 0x3F));
            *utf8++ = (uint8_t)(0x80 | (chr >> 0 & 0x3F));
        }
        // TODO: Handle UTF-16 code points that take two entries.
    }
}

// Count how many bytes a utf-16-le encoded string will take in utf-8.
static int _count_utf8_bytes(const uint16_t *buf, size_t len) {
    size_t total_bytes = 0;
    for (size_t i = 0; i < len; i++) {
        uint16_t chr = buf[i];
        if (chr < 0x80) {
            total_bytes += 1;
        } else if (chr < 0x800) {
            total_bytes += 2;
        } else {
            total_bytes += 3;
        }
        // TODO: Handle UTF-16 code points that take two entries.
    }
    return (int) total_bytes;
}
static void print_utf16(uint16_t *temp_buf, size_t buf_len) {
    if ((temp_buf[0] & 0xff) == 0) return;  // empty
    size_t utf16_len = ((temp_buf[0] & 0xff) - 2) / sizeof(uint16_t);
    size_t utf8_len = (size_t) _count_utf8_bytes(temp_buf + 1, utf16_len);
    _convert_utf16le_to_utf8(temp_buf + 1, utf16_len, (uint8_t *) temp_buf, sizeof(uint16_t) * buf_len);
    ((uint8_t*) temp_buf)[utf8_len] = '\0';

    printf("%s", (char*)temp_buf);
}

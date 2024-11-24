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

u8 kb_addr = 0;
u8 kb_inst = 0;
u8 kb_leds = 0;  
char device_str[50];
char manufacturer_str[50];


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

bool is_button_pressed(uint pin) {
    static uint64_t last_press_time[5] = {0};  // Marca de tiempo de los botones
    static bool last_state[5] = {true, true, true, true, true};  // Estado anterior de los botones (asumido como no presionado)
    uint64_t current_time = time_us_64();  // Tiempo actual en microsegundos
    int button_index = -1;

    // Mapeo de pines a índices
    switch(pin) {
        case GAMEPAD_UP:
            button_index = 0;
            break;
        case GAMEPAD_DOWN:
            button_index = 1;
            break;
        case GAMEPAD_LEFT:
            button_index = 2;
            break;
        case GAMEPAD_RIGHT:
            button_index = 3;
            break;
        case GAMEPAD_FIRE:
            button_index = 4;
            break;
        default:
            return false;  // Pin desconocido
    }

    // Verificar si el botón está presionado (activa bajo)
    bool current_state = gpio_get(pin) == 0;

    // Si el estado ha cambiado y ha pasado el tiempo de debounce
    if (current_state != last_state[button_index] && (current_time - last_press_time[button_index]) > 100000) {
        last_press_time[button_index] = current_time;
        last_state[button_index] = current_state;
        return true;  // El estado ha cambiado
    }
    return false;  // El estado no ha cambiado
}

int main() {
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


    gpio_init(GAMEPAD_UP);
    gpio_init(GAMEPAD_DOWN);
    gpio_init(GAMEPAD_LEFT); 
    gpio_init(GAMEPAD_RIGHT);
    gpio_init(GAMEPAD_FIRE); 
    gpio_set_dir(GAMEPAD_UP, GPIO_IN);
    gpio_set_dir(GAMEPAD_DOWN, GPIO_IN);
    gpio_set_dir(GAMEPAD_LEFT, GPIO_IN);
    gpio_set_dir(GAMEPAD_RIGHT, GPIO_IN);
    gpio_set_dir(GAMEPAD_FIRE, GPIO_IN);

    gpio_pull_up(GAMEPAD_UP);
    gpio_pull_up(GAMEPAD_DOWN);
    gpio_pull_up(GAMEPAD_LEFT);
    gpio_pull_up(GAMEPAD_RIGHT);
    gpio_pull_up(GAMEPAD_FIRE);

    while (1) {

        tuh_task();
        kb_task();

        // Verificar el estado de cada uno de los botones y realizar la acción correspondiente
        if (is_button_pressed(GAMEPAD_UP)) {
            if (gpio_get(GAMEPAD_UP) == 0) {
                printf("Botón 1 presionado\n");
            } else {
                printf("Botón 1 liberado\n");
            }
        }

        if (is_button_pressed(GAMEPAD_DOWN)) {
            if (gpio_get(GAMEPAD_DOWN) == 0) {
                printf("Botón 2 presionado\n");
            } else {
                printf("Botón 2 liberado\n");
            }
        }

        if (is_button_pressed(GAMEPAD_LEFT)) {
            if (gpio_get(GAMEPAD_LEFT) == 0) {
                printf("Botón 3 presionado\n");
            } else {
                printf("Botón 3 liberado\n");
            }
        }

        if (is_button_pressed(GAMEPAD_RIGHT)) {
            if (gpio_get(GAMEPAD_RIGHT) == 0) {
                printf("Botón 4 presionado\n");
            } else {
                printf("Botón 4 liberado\n");
            }
        }

        if (is_button_pressed(GAMEPAD_FIRE)) {
            if (gpio_get(GAMEPAD_FIRE) == 0) {
                printf("Botón 5 presionado\n");
            } else {
                printf("Botón 5 liberado\n");
            }
        }

        // Un pequeño retraso para evitar el uso excesivo de la CPU
        // sleep_ms(10);

        ms_task();


    }
    
    return 0;
}

// void main() {
//   board_init();
//   printf("\n%s-%s\n", PICO_PROGRAM_NAME, PICO_PROGRAM_VERSION_STRING);

//   gpio_init(LVOUT);
//   gpio_init(LVIN);

//   // // ADD - DESTROYER
//   printf("Waiting for ESPectrum...\n");
//     // Configurar pines como entradas con pull-ups
//     const uint BUTTON_PINS[] = {GAMEPAD_UP, GAMEPAD_DOWN, GAMEPAD_LEFT, GAMEPAD_RIGHT, GAMEPAD_FIRE};
//     for (int i = 0; i < 5; i++) {
//         gpio_init(BUTTON_PINS[i]);
//         gpio_set_dir(BUTTON_PINS[i], GPIO_IN);
//         gpio_pull_up(BUTTON_PINS[i]);
//     }
//   // // END - DESTROYER

//   gpio_set_dir(LVOUT, GPIO_OUT);
//   gpio_set_dir(LVIN, GPIO_OUT);
//   gpio_put(LVOUT, 1);
//   gpio_put(LVIN, 1);
  
//   tusb_init();
//   kb_init(KBOUT, KBIN);
//   ms_init(MSOUT, MSIN);
  

//     while (1) {


//         tuh_task();
//         kb_task();
//         ms_task();

//         // // ADD -> DESTROYER
//         // uint32_t current_time = time_us_32();  // Obtener tiempo actual en microsegundos
//         // if (current_time - last_button_check >= DEBOUNCE_TIME * 1000) {
            
//         //     check_button();  // Verifica el estado del botón
//         //     last_button_check = current_time;  // Actualiza la última verificación
            
//         // }
//         // // END - DESTROYER
//     }
    
// }

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

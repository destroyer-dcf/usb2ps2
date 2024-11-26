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


#include "tusb.h"
#include "ps2out.h"
#include "ps2in.h"
#include "hardware/watchdog.h"
#include "scancodesets.h"
#include "button.h"

ps2out kb_out;
ps2in kb_in;

// #define joy1Up 2 
// #define joy1Down 3
// #define joy1Left 4 
// #define joy1Right 5 
// #define joy1Fire 6 
// #define joy1Select 7 // Para joyStick SEGA
// #define joy1Start 8 // Para joyStick SEGA

#define GAMEPAD_SELECT 7
#define GAMEPAD_UP 2 
#define GAMEPAD_DOWN 3
#define GAMEPAD_LEFT 4 
#define GAMEPAD_RIGHT 5 
#define GAMEPAD_FIRE 6 
#define GAMEPAD_MODE 8 // Es mode???
#define SCAN_CODE_SET_F0 0xf0
#define SCAN_CODE_SET_E2 0xe2

// Estado de los botones
// uint8_t button_state = 0;     // Estado actual de los botones
uint8_t previous_state = 0;   // Estado anterior de los botones

// Máscaras para los botones
#define MASK_UP    (1 << 0)
#define MASK_DOWN  (1 << 1)
#define MASK_LEFT  (1 << 2)
#define MASK_RIGHT (1 << 3)
#define MASK_FIRE  (1 << 4)

#define KBHOSTCMD_RESET_FF 0xff
#define KBHOSTCMD_RESEND_FE 0xfe
#define KBHOSTCMD_SCS3_SET_KEY_MAKE_FD 0xfd
#define KBHOSTCMD_SCS3_SET_KEY_MAKE_BREAK_FC 0xfc
#define KBHOSTCMD_SCS3_SET_KEY_MAKE_TYPEMATIC_FB 0xfb
#define KBHOSTCMD_SCS3_SET_ALL_MAKE_BREAK_TYPEMATIC_FA 0xfa
#define KBHOSTCMD_SCS3_SET_ALL_MAKE_F9 0xf9
#define KBHOSTCMD_SCS3_SET_ALL_MAKE_BREAK_F8 0xf8
#define KBHOSTCMD_SCS3_SET_ALL_MAKE_TYPEMATIC_F7 0xf7
#define KBHOSTCMD_SET_DEFAULT_F6 0xf6
#define KBHOSTCMD_DISABLE_F5 0xf5
#define KBHOSTCMD_ENABLE_F4 0xf4
#define KBHOSTCMD_SET_TYPEMATIC_PARAMS_F3 0xf3
#define KBHOSTCMD_READ_ID_F2 0xf2
#define KBHOSTCMD_SET_SCAN_CODE_SET_F0 0xf0
#define KBHOSTCMD_SET_SCAN_CODE_SET_E2 0xe2
#define KBHOSTCMD_ECHO_EE 0xee
#define KBHOSTCMD_SET_LEDS_ED 0xed

#define KB_MSG_SELFTEST_PASSED_AA 0xaa
#define KB_MSG_ID1_AB 0xab
#define KB_MSG_ID2_83 0x83
#define KB_MSG_ECHO_EE 0xee
#define KB_MSG_ACK_FA 0xfa
#define KB_MSG_RESEND_FE 0xfe

// ADD -> DESTROYER
bool button_pressed = false;  // Estado del botón
bool button_pressed_up = false;  // Estado del botón
bool button_pressed_down = false;  // Estado del botón
bool button_pressed_left = false;  // Estado del botón
bool button_pressed_right = false;  // Estado del botón
bool button_pressed_fire = false;  // Estado del botón
// END - DESTROYER

#define ESP_JOY1LEFT 0x40
#define ESP_JOY1RIGHT 0x41
#define ESP_JOY1UP 0x42
#define ESP_JOY1DOWN 0x43
#define ESP_JOY1START 0x44
#define ESP_JOY1MODE 0x45
#define ESP_JOY1A 0x46
#define ESP_JOYFIRE 0x46
#define ESP_JOY1B 0x47
#define ESP_JOY1C 0x48
#define ESP_JOY1X 0x49
#define ESP_JOY1Y 0x4a
#define ESP_JOY1Z 0x4b

typedef enum {
  KBH_STATE_IDLE,
  KBH_STATE_SET_LEDS_ED,
  KBH_STATE_SET_SCAN_CODE_SET_F0,
  KBH_STATE_SET_TYPEMATIC_PARAMS_F3,
  KBH_STATE_SET_KEY_MAKE_FD,
  KBH_STATE_SET_KEY_MAKE_BREAK_FC,
  KBH_STATE_SET_KEY_MAKE_TYPEMATIC_FB
} kbhost_state_enum_t;

kbhost_state_enum_t kbhost_state = KBH_STATE_IDLE;

typedef enum {
  SCS3_MODE_MAKE,
  SCS3_MODE_MAKE_BREAK,
  SCS3_MODE_MAKE_TYPEMATIC,
  SCS3_MODE_MAKE_BREAK_TYPEMATIC,
} scs3_mode_enum_t;

scs3_mode_enum_t scs3_mode = SCS3_MODE_MAKE_BREAK_TYPEMATIC;

#define SCAN_CODE_SET_1 1
#define SCAN_CODE_SET_2 2
#define SCAN_CODE_SET_3 3
#define SCAN_CODE_SET_4 4

u8 scancodeset = SCAN_CODE_SET_2;

#define HOST_CMD_MIN 0xe0
#define KEYMODEMASK_BREAK 0b00000001
#define KEYMODEMASK_TYPEMATIC 0b00000010

u8 scs3keymodemap[HOST_CMD_MIN];

u8 const led2ps2[] = { 0, 4, 1, 5, 2, 6, 3, 7 };

u32 const repeats[] = {
  33333, 37453, 41667, 45872, 48309, 54054, 58480, 62500,
  66667, 75188, 83333, 91743, 100000, 108696, 116279, 125000,
  133333, 149254, 166667, 181818, 200000, 217391, 232558, 250000,
  270270, 303030, 333333, 370370, 400000, 434783, 476190, 500000
};
u16 const delays[] = { 250, 500, 750, 1000 };

bool kb_enabled = true;
bool blinking = false;
u32 repeat_us;
u16 delay_ms;
alarm_id_t repeater;

u8 prev_rpt[] = { 0, 0, 0, 0, 0, 0, 0, 0 };
u8 key2repeat = 0;

u8 last_byte_sent = 0;

void kb_send(u8 byte) {
  if (byte != KB_MSG_RESEND_FE){
    last_byte_sent = byte;
  } else {
    printf("NO TENGO NI PUTA IDEA");
  }
  printf("----> ENVIA SCANCODE: %02x\n", byte);
// printf("Dirección de kb_out.qbytes: %p\n", (void *)&kb_out.qbytes);
// printf("Dirección de last_byte_sent: %p\n", (void *)&last_byte_sent);
  queue_try_add(&kb_out.qbytes, &byte);
}

void kb_resend_last() {
  printf("r: k>h %x\n", last_byte_sent);
  queue_try_add(&kb_out.qbytes, &last_byte_sent);
}

void kb_maybe_send_prefix(u8 key) {
  u8 const *l = IS_MOD_KEY(key) ? ext_code_modifier_keys_1_2 : ext_code_keys_1_2;
  for (int i = 0; l[i]; i++) {
    if (key == l[i]) {
      printf("send_prefix",KB_EXT_PFX_E0);
      kb_send(KB_EXT_PFX_E0);
      break;
    }
  }
}

// sends out scan codes from a null byte terminated list
void kb_send_sc_list(const u8 *list) {
  key2repeat = 0;
  for (int i = 0; list[i]; i++) {
    kb_send(list[i]);
  }
}

void kb_set_leds(u8 byte) {
  if(byte > 7) byte = 0;
  tuh_kb_set_leds(led2ps2[byte]);
  ps2in_set(&kb_in, 0xed, byte);
}

s64 blink_callback() {
  if(blinking) {
    printf("Blinking keyboard LEDs\n");
    kb_set_leds(KEYBOARD_LED_NUMLOCK | KEYBOARD_LED_CAPSLOCK | KEYBOARD_LED_SCROLLLOCK);
    blinking = false;
    return 500000;
  }
  kb_set_leds(0);
  return 0;
}

void set_scancodeset(u8 scs) {
  // printf("----> SCANCODE SET: %u\n", scs);
  scancodeset = scs;
}

void kb_set_defaults() {
  printf("Setting defaults for keyboard\n");
  kbhost_state = KBH_STATE_IDLE;
  scs3_mode = SCS3_MODE_MAKE_BREAK_TYPEMATIC;
  set_scancodeset(2);
  kb_enabled = true;
  repeat_us = 91743;
  delay_ms = 500;
  blinking = true;
  memchr(prev_rpt, sizeof(prev_rpt), 0);
  add_alarm_in_ms(100, blink_callback, NULL, false);
  ps2in_reset(&kb_in);
}

s64 repeat_cb() {
  if(key2repeat) {
    switch (scancodeset) {
      case SCAN_CODE_SET_1:
        kb_maybe_send_prefix(key2repeat);
        IS_MOD_KEY(key2repeat) ? kb_send(mod2ps2_1[key2repeat - HID_KEY_CONTROL_LEFT]) : kb_send(hid2ps2_1[key2repeat]);
      break;
      case SCAN_CODE_SET_2:
        kb_maybe_send_prefix(key2repeat);
        IS_MOD_KEY(key2repeat) ? kb_send(mod2ps2_2[key2repeat - HID_KEY_CONTROL_LEFT]) : kb_send(hid2ps2_2[key2repeat]);
      break;
      case SCAN_CODE_SET_3:
        IS_MOD_KEY(key2repeat) ? kb_send(mod2ps2_3[key2repeat - HID_KEY_CONTROL_LEFT]) : kb_send(hid2ps2_3[key2repeat]);

      break;
      default:
        repeater = 0;
      return 0;
    }
    return repeat_us;
  }
  repeater = 0;
  return 0;
}

#define LOG_UNMAPPED_KEY printf("WARNING: Unmapped HID key 0x%x in set %d, ignoring it!\n",key,scancodeset);

void kb_send_key_scs1(u8 key, bool is_key_pressed, bool is_ctrl) {
  printf("***** KEYBOARD CONTROL\n");
  printf("----> KEY VALUE: %u\n", key);
  // PrintScreen and Pause have special sequences that must be sent.
  // Pause doesn't have a break code.
  if(key == HID_KEY_PAUSE || key == HID_KEY_PRINT_SCREEN) {
    if(is_key_pressed  && key == HID_KEY_PRINT_SCREEN)      kb_send_sc_list(prt_scn_make_1);
    if(!is_key_pressed && key == HID_KEY_PRINT_SCREEN)      kb_send_sc_list(prt_scn_break_1);
    if(is_key_pressed  && key == HID_KEY_PAUSE && is_ctrl)  kb_send_sc_list(break_make_1);
    if(is_key_pressed  && key == HID_KEY_PAUSE && !is_ctrl) kb_send_sc_list(pause_make_1);
    return;
  }

  u8 scan_code = IS_MOD_KEY(key) ? mod2ps2_1[key - HID_KEY_CONTROL_LEFT] : hid2ps2_1[key];

  if (!scan_code) {
    LOG_UNMAPPED_KEY
    return;
  }

  // Some keys require a prefix before the actual code
  kb_maybe_send_prefix(key);

  if (is_key_pressed) {
    // Take care of typematic repeat
    key2repeat = key;
    if(repeater) cancel_alarm(repeater);
    repeater = add_alarm_in_ms(delay_ms, repeat_cb, NULL, false);
  printf("2--> %d\n", scan_code);
    kb_send(scan_code);
  } else {
    // Cancel repeat
    if(key == key2repeat) key2repeat = 0;

    kb_send(scan_code | 0x80);
  }
  printf("***********************\n");
}

void kb_send_key_scs2(u8 key, bool is_key_pressed, bool is_ctrl) {

  printf("***** KEYBOARD CONTROL\n");
  printf("----> KEY VALUE: %u\n", key);

  // PrintScreen and Pause have special sequences that must be sent.
  // Pause doesn't have a break code.
  if(key == HID_KEY_PAUSE || key == HID_KEY_PRINT_SCREEN) {
    if(is_key_pressed  && key == HID_KEY_PRINT_SCREEN)      kb_send_sc_list(prt_scn_make_2);
    if(!is_key_pressed && key == HID_KEY_PRINT_SCREEN)      kb_send_sc_list(prt_scn_break_2);
    if(is_key_pressed  && key == HID_KEY_PAUSE && is_ctrl)  kb_send_sc_list(break_make_2);
    if(is_key_pressed  && key == HID_KEY_PAUSE && !is_ctrl) kb_send_sc_list(pause_make_2);
    return;
  }

  // u8 scan_code = IS_MOD_KEY(key) ? mod2ps2_2[key - HID_KEY_CONTROL_LEFT] : hid2ps2_2[key];
    u8 scan_code;
    if (IS_MOD_KEY(key)) {
        scan_code = mod2ps2_2[key - HID_KEY_CONTROL_LEFT];
    } else {
        scan_code = hid2ps2_2[key];
    }


  if (!scan_code) {
    LOG_UNMAPPED_KEY
    return;
  }

  // Some keys require a prefix before the actual code
  kb_maybe_send_prefix(key);

  if (is_key_pressed) {
  // Take care of typematic repeat
    printf("----> KEY PRESET: TRUE\n");
    key2repeat = key;
    if(repeater) cancel_alarm(repeater);
    repeater = add_alarm_in_ms(delay_ms, repeat_cb, NULL, false);
  } else {
    printf("----> KEY PRESET: FALSE\n");
    if(key == key2repeat) key2repeat = 0;
    kb_send(KB_BREAK_2_3);
  }
  kb_send(scan_code);
  printf("***********************\n");
}

void kb_send_key_scs3(u8 key, bool is_key_pressed) {
  printf("***** KEYBOARD CONTROL\n");
  printf("----> KEY VALUE: %u\n", key);
  u8 scan_code = IS_MOD_KEY(key) ? mod2ps2_3[key - HID_KEY_CONTROL_LEFT] : hid2ps2_3[key];

  if (!scan_code) {
    LOG_UNMAPPED_KEY
    return;
  }

  if (is_key_pressed) {
    // Take care of typematic repeat
    if (
      (scs3_mode == SCS3_MODE_MAKE_BREAK_TYPEMATIC || scs3_mode == SCS3_MODE_MAKE_TYPEMATIC)
      && !(scs3keymodemap[scan_code] & KEYMODEMASK_TYPEMATIC)
    ) {
      key2repeat = key;
      if(repeater) cancel_alarm(repeater);
      repeater = add_alarm_in_ms(delay_ms, repeat_cb, NULL, false);
    }
    kb_send(scan_code);
  } else {
    if(key == key2repeat) key2repeat = 0;

    if (
      (scs3_mode == SCS3_MODE_MAKE_BREAK || scs3_mode == SCS3_MODE_MAKE_BREAK_TYPEMATIC)
      && !(scs3keymodemap[scan_code] & KEYMODEMASK_BREAK)
    ) {
      kb_send(KB_BREAK_2_3);
      kb_send(scan_code);
    }
  }
}


// Sends a key state change to the host
// u8 keycode          - from hid.h HID_KEY_ definition
// bool is_key_pressed - state of key: true=pressed, false=released
void kb_send_key(u8 key, bool is_key_pressed, u8 modifiers) {
  printf("MODIFIERS ES: %u\n", modifiers);
  if (!kb_enabled) {
    printf("WARNING: Keyboard disabled, ignoring key press %u\n", key);
    return;
  }
  
  if(!IS_VALID_KEY(key)) {
    printf("INFO: Ignoring hid key 0x%x by design.\n", key);
    return;
  }

  bool is_ctrl = modifiers & KEYBOARD_MODIFIER_LEFTCTRL || modifiers & KEYBOARD_MODIFIER_RIGHTCTRL;
  switch (scancodeset) {
    case SCAN_CODE_SET_1:
      printf("----> SCANCODE SET 1\n");
      kb_send_key_scs1(key, is_key_pressed, is_ctrl);
      break;
    case SCAN_CODE_SET_2:
      printf("----> SCANCODE SET 2\n");
      kb_send_key_scs2(key, is_key_pressed, is_ctrl);
      break;
    case SCAN_CODE_SET_3:
      printf("----> SCANCODE SET 3\n");
      kb_send_key_scs3(key, is_key_pressed);
      break;
    case SCAN_CODE_SET_4:
      printf("----> SCANCODE SET 4\n");
      kb_send_key_gamepad_control(key, is_key_pressed);
      //kb_send_key_scs2(key, is_key_pressed, is_ctrl);
      break;
    default:
      printf("INTERNAL ERROR! SCAN CODE SET = %u\n", scancodeset);
      break;
  }
}

void kb_usb_receive(u8 const* report, u16 len) {

  // go over modifier keys which are in report[0]
  if(report[0] != prev_rpt[0]) {
    // modifiers have changed
    u8 rbits = report[0];
    u8 pbits = prev_rpt[0];
    
    for(u8 j = 0; j <= MOD2PS2_IDX_MAX; j++) {
      if((rbits & 1) != (pbits & 1)) {
        kb_send_key(HID_KEY_CONTROL_LEFT + j, rbits & 1, report[0]);
      }
      
      rbits = rbits >> 1;
      pbits = pbits >> 1;
    }
  }

  // NOTE: report[1] is completely ignored.
  //       It is usually 0x00. Log if otherwise.
  if (report[1]) {
    printf("INFO: report[1]=0x%x\n",report[1]); // just curious... ;)
  }
  
  // go over activated non-modifier keys in prev_rpt and
  // check if they are still in the current report
  for(u8 i = 2; i < sizeof(prev_rpt); i++) {
    if(prev_rpt[i]) {
      bool brk = true;
      
      for(u8 j = 2; j < len; j++) {
        if(prev_rpt[i] == report[j]) {
          brk = false;
          break;
        }
      }
      
      if(brk) {
        // send break if key not pressed anymore
        kb_send_key(prev_rpt[i], false, report[0]);
      }
    }
  }
  
  // go over activated non-modifier keys in report and check if they were
  // already in prev_rpt.
  for(u8 i = 2; i < len; i++) {
    if(report[i]) {
      bool make = true;
      
      for(u8 j = 2; j < sizeof(prev_rpt); j++) {
        if(report[i] == prev_rpt[j]) {
          make = false;
          break;
        }
      }
      
      // send make if key was in the current report the first time
      if(make) {
        kb_send_key(report[i], true, report[0]);
      }
    }
  }
  
  // Only remember the first sizeof(prv_rpt) from the current report at most
  memchr(prev_rpt, sizeof(prev_rpt), 0);
  memcpy(prev_rpt, report, len < sizeof(prev_rpt) ? len : sizeof(prev_rpt));
}

const char* notinscs3_str = "WARNING: Scan code set 3 not set. Ignoring command 0x%x\n";

void kb_receive(u8 byte, u8 prev_byte) {
  printf("host > kb %02x\n", byte);
  switch (kbhost_state) {
    case KBH_STATE_SET_KEY_MAKE_FD:
    case KBH_STATE_SET_KEY_MAKE_BREAK_FC:
    case KBH_STATE_SET_KEY_MAKE_TYPEMATIC_FB:
      // Scan code set 3 only
      if (byte < sizeof(scs3keymodemap)) {
        switch (kbhost_state) {
          case KBH_STATE_SET_KEY_MAKE_FD: scs3keymodemap[byte] = KEYMODEMASK_BREAK | KEYMODEMASK_TYPEMATIC; break;
          case KBH_STATE_SET_KEY_MAKE_BREAK_FC: scs3keymodemap[byte]= KEYMODEMASK_TYPEMATIC; break;
          case KBH_STATE_SET_KEY_MAKE_TYPEMATIC_FB: scs3keymodemap[byte]= KEYMODEMASK_BREAK; break;
        }
        // we stay in KBH_STATE_SET_KEY.. to be ready to receive the next scancode
      } else {
        // we received a host command, we must deal with the actual command
        kbhost_state = KBH_STATE_IDLE;
        kb_receive(byte, prev_byte);
        return;
      }
    break;

    case KBH_STATE_SET_LEDS_ED:
      kb_set_leds(byte);
      kbhost_state = KBH_STATE_IDLE;
    break;
    
    case KBH_STATE_SET_TYPEMATIC_PARAMS_F3:
      repeat_us = repeats[byte & 0x1f];
      delay_ms = delays[(byte & 0x60) >> 5];
      ps2in_set(&kb_in, 0xf3, byte);
      kbhost_state = KBH_STATE_IDLE;
    break;

    case KBH_STATE_SET_SCAN_CODE_SET_F0:
      switch((u8)byte) {
        case 0:
          kb_send(scancodeset);
          break;
        case SCAN_CODE_SET_1:
        case SCAN_CODE_SET_2:
        case SCAN_CODE_SET_3:
          memchr(scs3keymodemap,0,sizeof(scs3keymodemap));
          set_scancodeset(byte);
          break;
        case SCAN_CODE_SET_4:
          printf("Estoy en el 4-1");
          set_scancodeset(4);
          break;
        default:
          printf("WARNING: scancodeset requested to set to unknown value %u by host, defaulting to 2\n",byte);
          set_scancodeset(2);
        break;
      }
      kbhost_state = KBH_STATE_IDLE;
    break;

    case KBH_STATE_IDLE:
    default:
      switch ((u8)byte) {
        case KBHOSTCMD_RESET_FF:
          printf("KBHOSTCMD_RESET_FF\n");
          // We only set defaults, we do not actually reset ourselves.
          kb_set_defaults();
          kb_send(KB_MSG_ACK_FA);
          kb_send(KB_MSG_SELFTEST_PASSED_AA);
        return;

        case KBHOSTCMD_RESEND_FE:
          printf("KBHOSTCMD_RESEND_FE\n");
          kb_resend_last();
          kbhost_state = KBH_STATE_IDLE;
        return;

        case KBHOSTCMD_SCS3_SET_KEY_MAKE_FD:
          printf("KBHOSTCMD_SCS3_SET_KEY_MAKE_FD\n");
          if (scancodeset == SCAN_CODE_SET_3) {
            kbhost_state = KBH_STATE_SET_KEY_MAKE_FD;
          } else {
            printf(notinscs3_str,byte);
            kbhost_state = KBH_STATE_IDLE;
          }
        break;

        case KBHOSTCMD_SCS3_SET_KEY_MAKE_BREAK_FC:
          printf("KBHOSTCMD_SCS3_SET_KEY_MAKE_BREAK_FC\n");
          if (scancodeset == SCAN_CODE_SET_3) {
            kbhost_state = KBH_STATE_SET_KEY_MAKE_BREAK_FC;
          } else {
            printf(notinscs3_str,byte);
            kbhost_state = KBH_STATE_IDLE;
          }
        break;

        case KBHOSTCMD_SCS3_SET_KEY_MAKE_TYPEMATIC_FB:
          printf("KBHOSTCMD_SCS3_SET_KEY_MAKE_TYPEMATIC_FB\n");
          if (scancodeset == SCAN_CODE_SET_3) {
            kbhost_state = KBH_STATE_SET_KEY_MAKE_TYPEMATIC_FB;
          } else {
            printf(notinscs3_str,byte);
            kbhost_state = KBH_STATE_IDLE;
          }
        break;


        case KBHOSTCMD_SCS3_SET_ALL_MAKE_BREAK_TYPEMATIC_FA: 
          printf("KBHOSTCMD_SCS3_SET_ALL_MAKE_BREAK_TYPEMATIC_FA\n");
          if (scancodeset == SCAN_CODE_SET_3) {
            scs3_mode = SCS3_MODE_MAKE_BREAK_TYPEMATIC;
            memchr(scs3keymodemap,0,sizeof(scs3keymodemap));
          } else {
            printf(notinscs3_str,byte);
          }
          kbhost_state = KBH_STATE_IDLE;
        break;

        case KBHOSTCMD_SCS3_SET_ALL_MAKE_F9: 
          printf("KBHOSTCMD_SCS3_SET_ALL_MAKE_F9\n");
          if (scancodeset == SCAN_CODE_SET_3) {
            scs3_mode = SCS3_MODE_MAKE;
            memchr(scs3keymodemap,0,sizeof(scs3keymodemap));
          } else {
            printf(notinscs3_str,byte);
          }
          kbhost_state = KBH_STATE_IDLE;
        break;

        case KBHOSTCMD_SCS3_SET_ALL_MAKE_BREAK_F8: 
          // utilized by SGI O2
          if (scancodeset == SCAN_CODE_SET_3) {
            printf("KBHOSTCMD_SCS3_SET_ALL_MAKE_BREAK_F8\n");
            scs3_mode = SCS3_MODE_MAKE_BREAK;
            memchr(scs3keymodemap,0,sizeof(scs3keymodemap));
          } else {
            printf(notinscs3_str,byte);
          }
          kbhost_state = KBH_STATE_IDLE;
        break;

        case KBHOSTCMD_SCS3_SET_ALL_MAKE_TYPEMATIC_F7:
          if (scancodeset == SCAN_CODE_SET_3) {
            printf("KBHOSTCMD_SCS3_SET_ALL_MAKE_TYPEMATIC_F7\n");
            scs3_mode = SCS3_MODE_MAKE_TYPEMATIC;
            memchr(scs3keymodemap,0,sizeof(scs3keymodemap));
          } else {
            printf(notinscs3_str,byte);
          }
          kbhost_state = KBH_STATE_IDLE;
        break;

        case KBHOSTCMD_SET_DEFAULT_F6:
          printf("KBHOSTCMD_SET_DEFAULT_F6\n");
          kb_set_defaults();
        break;
        
        case KBHOSTCMD_DISABLE_F5:
          printf("KBHOSTCMD_DISABLE_F5\n");
          // Documentation says this command might also set defaults.
          // In the case of a SGI O2 and generic PS/2 Cherry KB this not true
          // and would prevent the O2 from working.
          // The O2 sets scan code set 3 and then disables the keyboard with F5.
          // It still expects the KB to be in scan code set 3 mode though
          // when it enables it afterwards with F4.
          //
          // kb_set_defaults();
          //
          kb_enabled = false;
        break;
        
        case KBHOSTCMD_ENABLE_F4:
          printf("KBHOSTCMD_ENABLE_F4\n");
          kb_enabled = true;
          kbhost_state = KBH_STATE_IDLE;
        break;
    
        case KBHOSTCMD_SET_TYPEMATIC_PARAMS_F3:
          printf("KBHOSTCMD_SET_TYPEMATIC_PARAMS_F3\n");
          kbhost_state = KBH_STATE_SET_TYPEMATIC_PARAMS_F3;
        break;
        
        case KBHOSTCMD_READ_ID_F2:
          printf("KBHOSTCMD_READ_ID_F2\n");
          kb_send(KB_MSG_ACK_FA);
          kb_send(KB_MSG_ID1_AB);
          kb_send(KB_MSG_ID2_83);
        return; // ACK already sent

        case KBHOSTCMD_SET_SCAN_CODE_SET_F0:
          printf("KBHOSTCMD_SET_SCAN_CODE_SET_F0\n");
          kbhost_state = KBH_STATE_SET_SCAN_CODE_SET_F0;
        break;
        
        case KBHOSTCMD_ECHO_EE:
          printf("KBHOSTCMD_ECHO_EE\n");
          kb_send(KB_MSG_ECHO_EE);
          kbhost_state = KBH_STATE_IDLE;
        return;

        case KBHOSTCMD_SET_LEDS_ED:
          printf("KBHOSTCMD_SET_LEDS_ED\n");
          kbhost_state = KBH_STATE_SET_LEDS_ED;
        break;

        default:
          printf("WARNING: Unknown host cmd: 0x%x, requesting resend from host!\n",byte);
          kb_send(KB_MSG_RESEND_FE);
          kbhost_state = KBH_STATE_IDLE;
        return;
      }
    break;
  }
  kb_send(KB_MSG_ACK_FA);
}

// Estados de los botones
bool gamepad_left_state = false;
bool gamepad_left_prev_state = false;
bool gamepad_right_state = false;
bool gamepad_right_prev_state = false;
bool gamepad_up_state = false;
bool gamepad_up_prev_state = false;
bool gamepad_down_state = false;
bool gamepad_down_prev_state = false;
bool gamepad_fire_state = false;
bool gamepad_fire_prev_state = false;

// ************************
// GAMEPAD CONTROL
// ************************

#define SCAN_CODE_SET_F0 0xf0
#define SCAN_CODE_SET_E2 0xe2

void kb_send_key_gamepad_control(u8 key, bool is_key_pressed) {
  printf("***** GAMEPAD CONTROL\n");
  printf("----> KEY VALUE: %u\n", key);
  u8 scan_code = gamepad_scancodes[key];
  kb_send(SCAN_CODE_SET_E2);   
  if (is_key_pressed) {
    printf("----> KEY PRESET: TRUE\n");
    // key2repeat = key;
    // if(repeater) cancel_alarm(repeater);
    // repeater = add_alarm_in_ms(delay_ms, repeat_cb, NULL, false);
    kb_send(scan_code);
  } else {
    printf("----> KEY PRESET: FALSE\n");
    // if(key == key2repeat) key2repeat = 0;
    kb_send(KB_BREAK_2_3);
  }
  kb_send(scan_code);
}
#define SLEEP_TIME 25

void sendGamePad(u8 scancode, bool press) {
    printf("***** KEYBOARD CONTROL\n");
    kb_send(SCAN_CODE_SET_E2);
    sleep_ms(SLEEP_TIME);
    printf("----> KEY PRESSED: %s\n", press ? "TRUE" : "FALSE");
    if (!press) {
        kb_send(KB_BREAK_2_3);
        sleep_ms(SLEEP_TIME);
    }
    kb_send(scancode);
    sleep_ms(SLEEP_TIME);
    printf("**********************\n");
}

void debounce_delay() {
    sleep_ms(1);
}

// #define ESP_JOY1LEFT 0x40
// #define ESP_JOY1RIGHT 0x41

// #define ESP_JOY1UP 0x42
// #define ESP_JOY1DOWN 0x43
// #define ESP_JOY1START 0x44
// #define ESP_JOY1MODE 0x45
// #define ESP_JOY1FIRE 0x46

void check_buttons() {
    // Leer los estados actuales
    gamepad_left_state  = !gpio_get(GAMEPAD_LEFT);
    gamepad_right_state = !gpio_get(GAMEPAD_RIGHT);
    gamepad_up_state    = !gpio_get(GAMEPAD_UP);
    gamepad_down_state  = !gpio_get(GAMEPAD_DOWN);
    gamepad_fire_state  = !gpio_get(GAMEPAD_FIRE);

    // LEFT CONTROL ******
    if (gamepad_left_state && !gamepad_left_prev_state) {
        sendGamePad(ESP_JOY1LEFT,true);
    }
    if (!gamepad_left_state && gamepad_left_prev_state) {
        sendGamePad(ESP_JOY1LEFT,false);
    }

    // RIGHT CONTROL ******
    if (gamepad_right_state && !gamepad_right_prev_state) {
        sendGamePad(ESP_JOY1RIGHT,true);
    }
    if (!gamepad_right_state && gamepad_right_prev_state) {
        sendGamePad(ESP_JOY1RIGHT,false);
    }

    // UP CONTROL ******
    if (gamepad_up_state && !gamepad_up_prev_state) {
        sendGamePad(ESP_JOY1UP,true);
    }
    if (!gamepad_up_state && gamepad_up_prev_state) {
        sendGamePad(ESP_JOY1UP,false);
    }

    // DOWN CONTROL ******
    if (gamepad_down_state && !gamepad_down_prev_state) {
        sendGamePad(ESP_JOY1DOWN,true);
    }
    if (!gamepad_down_state && gamepad_down_prev_state) {
        sendGamePad(ESP_JOY1DOWN,false);
    }

    // FIRE CONTROL ******
    if (gamepad_fire_state && !gamepad_fire_prev_state) {
        sendGamePad(ESP_JOYFIRE,true);
    }
    if (!gamepad_fire_state && gamepad_fire_prev_state) {
        sendGamePad(ESP_JOYFIRE,false);
    }

    gamepad_left_prev_state = gamepad_left_state;
    gamepad_right_prev_state = gamepad_right_state;
    gamepad_up_prev_state = gamepad_up_state;
    gamepad_down_prev_state = gamepad_down_state;
    gamepad_fire_prev_state = gamepad_fire_state;
}


bool kb_task() {
  ps2out_task(&kb_out);
  ps2in_task(&kb_in, &kb_out);
  
  return kb_enabled && !kb_out.busy;// TODO: return value can probably be void
}

void kb_init(u8 gpio_out, u8 gpio_in) {
  printf("GPIO OUT %02x\n", gpio_out);
  printf("GPIO IN %02x\n", gpio_in);
  ps2out_init(&kb_out, pio0, gpio_out, &kb_receive);
  ps2in_init(&kb_in, pio1, gpio_in);
  kb_set_defaults();
  kb_send(KB_MSG_SELFTEST_PASSED_AA);
}

// // Función para verificar el estado del botón
// void check_button() {
//     if (gpio_get(BUTTON_TESTING) == 0) {
//         if (!button_pressed) {  // Si no estaba presionado antes
//             kb_send_key_scs2(13, 0, 0);
//             // printf("Hola\n");
//             button_pressed = true;  // Marca el botón como presionado
            
//         }
//     } else {
//         button_pressed = false;  // Resetea el estado cuando el botón se suelta
//     }
// }
void sendPadScancode(int scancode, bool press) {
  kb_send(SCAN_CODE_SET_E2);
  sleep_ms(50);
  if (!press) {
    kb_send(SCAN_CODE_SET_F0);
    sleep_ms(50);
  }
  kb_send(scancode);
  sleep_ms(50);
}


// // Rutina control jopystick's
// void onchange(button_t *button_p) {
//   button_t *button = (button_t*)button_p;
//   //printf("Button on pin %d changed its state to %d\n", button->pin, button->state);

// //de momento los 2 puertos de joystcik actuan igual, queda pendiente reasignar las teclas correspondientes a cada uno

// //Cuando soltamos un boton
//   if(button->state) {printf ("el state del pin %d es %d ¿Hemos SOLTADO el boton?\n",button->pin,button->state);
//   switch(button->pin){

//   //Comprobamos el joy1 si hemos soltado un boton 
//     case joy1Up:
//         //kb_send_key(0x52, 0, 0); // 0x52 cursor arriba
//         kb_send_key_gamepad_control(2,false);
//         //send_joy_action(ESP_JOY1UP,false);
//     break;
//     case joy1Down:
//         //kb_send_key(0x51, 0, 0); // 0x51 cursor abajo
//         // send_joy_action(ESP_JOY1DOWN,false);
//         kb_send_key_gamepad_control(3,false);
//     break;
//     case joy1Left:
//         //kb_send_key(0x50, 0, 0); //0x50 cursor izquierda
//         // send_joy_action(ESP_JOY1LEFT,false);
//         kb_send_key_gamepad_control(0,false);
//     break;
//     case joy1Right:
//         //kb_send_key(0x4f, 0, 0); //0x4f cursor derecha
//         //send_joy_action(ESP_JOY1RIGHT,false);
//         kb_send_key_gamepad_control(1,false);
//     break;
//     case joy1Fire: 
//         //kb_send_key(0x2b, 0, 0); // tabulador
//         // send_joy_action(ESP_JOY1A,false);
//         kb_send_key_gamepad_control(6,false);
//     break; 
//   }
  
//   //kb_send_key(0x52, 0, 0);
//    return;}
   
//    //cuando pulsamos un boton
//    if(!button->state) {;
//    switch(button->pin){

//    //comprobamos el joy1 si hemos pulsado un boton 
//     case joy1Up:
//         //kb_send_key(ESP_JOY1UP, 1, 0); // 0x52 cursor arriba 
//         kb_send_key_gamepad_control(2,true);
//     break;
//     case joy1Down:
//         kb_send_key_gamepad_control(3,true);
//         //kb_send_key(0x51, 1, 0); // 0x51 cursor abajo 
//     break;
//     case joy1Left:
//         //kb_send_key(0x50, 1, 0); // 0x50 cursor izquierda
//         kb_send_key_gamepad_control(0,true);
//     break;
//     case joy1Right:
//         kb_send_key_gamepad_control(1,true);
//         //kb_send_key(0x4f, 1, 0);// 0x4f cursor derecha
//     break;
//     case joy1Fire:
//         //kb_send_key(0x2b, 1, 0);// 0x2b tabulador
//         kb_send_key_gamepad_control(6,true);
//     break;
//   }
   
   
//    return; }
// }

void check_button() {

    set_scancodeset(4);

    bool current_state_up = !gpio_get(GAMEPAD_UP);
    if (current_state_up && !button_pressed_up) {
        button_pressed_up = true;
        kb_send_key_gamepad_control(2, button_pressed_up);
    } 
    else if (!current_state_up && button_pressed_up) {
        button_pressed_up = false;
        kb_send_key_gamepad_control(2, button_pressed_up);
    }

    bool current_state_down = !gpio_get(GAMEPAD_DOWN); 
    if (current_state_down && !button_pressed_down) {
        button_pressed_down = true;
        kb_send_key_gamepad_control(3, button_pressed_down);
    } 
    else if (!current_state_down && button_pressed_down) {
        button_pressed_down = false;
        kb_send_key_gamepad_control(3, button_pressed_down);
    }

    bool current_state_left = !gpio_get(GAMEPAD_LEFT); 
    if (current_state_left && !button_pressed_left) {
        button_pressed_left = true;
        kb_send_key_gamepad_control(0, button_pressed_left);
    } 
    else if (!current_state_left && button_pressed_left) {
        button_pressed_left = false;
        kb_send_key_gamepad_control(0, button_pressed_left);
    }

    bool current_state_right = !gpio_get(GAMEPAD_RIGHT);
    if (current_state_right && !button_pressed_right) {
        button_pressed_right = true;
        kb_send_key_gamepad_control(1, button_pressed_right);
    } 
    else if (!current_state_right && button_pressed_right) {
        button_pressed_right = false;
        kb_send_key_gamepad_control(1, button_pressed_right);
    }

    bool current_state_fire = !gpio_get(GAMEPAD_FIRE);
    if (current_state_fire && !button_pressed_fire) {
        button_pressed_fire = true;
        kb_send_key_gamepad_control(6, button_pressed_fire);
    } 
    else if (!current_state_fire && button_pressed_fire) {
        button_pressed_fire = false;
        kb_send_key_gamepad_control(6, button_pressed_fire);
    }

    set_scancodeset(2);

}
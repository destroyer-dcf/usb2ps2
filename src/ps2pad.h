#include <stdbool.h>
#include "pico/stdlib.h"
#include "button.h"

void padController();
typedef uint8_t u8;
void kb_send_key_gamepad_control(u8 key, bool is_key_pressed);
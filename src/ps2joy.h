// ps2joy.h
#ifndef PS2JOY_H
#define PS2JOY_H

#include <stdbool.h>
typedef uint8_t u8;
extern void send_joy_action(u8 scancode, bool press); // Actualiza la declaración
extern void check_joystick();
extern bool button_pressed;
void kb_send(u8 scancode); // Declaración de kb_send

#endif // PS2JOY_H
#include "button.h"
#include <stdbool.h>
#include "pico/stdlib.h"
#include <stdio.h>

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
//

#define SCAN_CODE_SET_F0 0xf0
#define SCAN_CODE_SET_E2 0xe2
#define SLEEP_TIME 25

#define joy1Up 2 
#define joy1Down 3
#define joy1Left 4 
#define joy1Right 5 
#define joy1Fire 6 
#define joy1Select 7 // Para joyStick SEGA
#define joy1Start 8 // Para joyStick SEGA

//
#define joy2Up 29 
#define joy2Down 28
#define joy2Left 27
#define joy2Right 26 
#define joy2Fire 10  
#define joy2Select 9  // Para joyStick SEGA
#define joy2Start 1  // Para joyStick SEGA

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

// uint32_t last_button_check = 0;  // Última vez que se verificó el botón

// Rutina control jopystick's
void onchange(button_t *button_p) {
  button_t *button = (button_t*)button_p;
  //printf("Button on pin %d changed its state to %d\n", button->pin, button->state);

//de momento los 2 puertos de joystcik actuan igual, queda pendiente reasignar las teclas correspondientes a cada uno

//Cuando soltamos un boton
  if(button->state) {printf ("el state del pin %d es %d ¿Hemos SOLTADO el boton?\n",button->pin,button->state);
  switch(button->pin){

  //Comprobamos el joy1 si hemos soltado un boton 
    case joy1Up:
        //kb_send_key(0x52, 0, 0); // 0x52 cursor arriba
        kb_send_key_gamepad_control(2,false);
        //send_joy_action(ESP_JOY1UP,false);
    break;
    case joy1Down:
        //kb_send_key(0x51, 0, 0); // 0x51 cursor abajo
        // send_joy_action(ESP_JOY1DOWN,false);
        kb_send_key_gamepad_control(3,false);
    break;
    case joy1Left:
        //kb_send_key(0x50, 0, 0); //0x50 cursor izquierda
        // send_joy_action(ESP_JOY1LEFT,false);
        kb_send_key_gamepad_control(0,false);
    break;
    case joy1Right:
        //kb_send_key(0x4f, 0, 0); //0x4f cursor derecha
        //send_joy_action(ESP_JOY1RIGHT,false);
        kb_send_key_gamepad_control(1,false);
    break;
    case joy1Fire: 
        //kb_send_key(0x2b, 0, 0); // tabulador
        // send_joy_action(ESP_JOY1A,false);
        kb_send_key_gamepad_control(6,false);
    break; 
  }
  
  //kb_send_key(0x52, 0, 0);
   return;}
   
   //cuando pulsamos un boton
   if(!button->state) {;
   switch(button->pin){

   //comprobamos el joy1 si hemos pulsado un boton 
    case joy1Up:
        //kb_send_key(ESP_JOY1UP, 1, 0); // 0x52 cursor arriba 
        kb_send_key_gamepad_control(2,true);
    break;
    case joy1Down:
        kb_send_key_gamepad_control(3,true);
        //kb_send_key(0x51, 1, 0); // 0x51 cursor abajo 
    break;
    case joy1Left:
        //kb_send_key(0x50, 1, 0); // 0x50 cursor izquierda
        kb_send_key_gamepad_control(0,true);
    break;
    case joy1Right:
        kb_send_key_gamepad_control(1,true);
        //kb_send_key(0x4f, 1, 0);// 0x4f cursor derecha
    break;
    case joy1Fire:
        //kb_send_key(0x2b, 1, 0);// 0x2b tabulador
        kb_send_key_gamepad_control(6,true);
    break;
  }
   
   
   return; }
}

void padController(){
  button_t *Joy1Up = create_button(joy1Up, onchange);
  button_t *Joy1Down = create_button(joy1Down, onchange);
  button_t *Joy1Left = create_button(joy1Left, onchange);
  button_t *Joy1Right = create_button(joy1Right, onchange);
  button_t *Joy1Fire = create_button(joy1Fire, onchange);

  //Se usa en el mando sega para comprobrar botones extra, hay que ponerla a 0 o 1 de modo manual y hacer a continuacion la lectura
  //de momento la desactivamos
  //button_t *Joy1Select = create_button(joy1Select, onchange); 

  button_t *Joy1Start = create_button(joy1Start, onchange); // o tambien conocido como fire2
}
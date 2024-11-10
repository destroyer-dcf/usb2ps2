#include <Arduino.h>
#include <GamePad.h>
#include <ps2dev.h> 
// Controller DB9 pins (looking face-on to the end of the plug):
//
// 5 4 3 2 1
//  9 8 7 6

// DB9 Pin 1 --> Arduino Pin 2
// DB9 Pin 2 --> Arduino Pin 3
// DB9 Pin 3 --> Arduino Pin 4
// DB9 Pin 4 --> Arduino Pin 5
// DB9 Pin 5 --> Arduino +5V
// DB9 Pin 6 --> Arduino Pin 6
// DB9 Pin 7 --> Arduino Pin 7 Select
// DB9 Pin 8 --> Arduino GND
// DB9 Pin 9 --> Arduino Pin 8

GamePad controller(7, 2, 3, 4, 5, 6, 8);

#define PS2_CLK_PIN 1
#define PS2_DATA_PIN 0

// Crear una instancia de PS2dev para el teclado PS2pn
PS2dev keyboard(PS2_CLK_PIN, PS2_DATA_PIN);

// Scancodes para Kempston Joystick
const int ESP_JOY1LEFT = 0x40;
const int ESP_JOY1RIGHT = 0x41;
const int ESP_JOY1UP = 0x42;
const int ESP_JOY1DOWN = 0x43;
const int ESP_JOY1START = 0x44;
const int ESP_JOY1MODE = 0x45;
const int ESP_JOY1A = 0x46;
const int ESP_JOY1B = 0x47;
const int ESP_JOY1C = 0x48;
const int ESP_JOY1X = 0x49;
const int ESP_JOY1Y = 0x4a;
const int ESP_JOY1Z = 0x4b;
// const int ESP_JOY2LEFT = 0x4c;
// const int ESP_JOY2RIGHT = 0x4d;
// const int ESP_JOY2UP = 0x4e;
// const int ESP_JOY2DOWN = 0x4f;
// const int ESP_JOY2START = 0x50;
// const int ESP_JOY2MODE = 0x51;
// const int ESP_JOY2A = 0x52;
// const int ESP_JOY2B = 0x53;
// const int ESP_JOY2C = 0x54;
// const int ESP_JOY2X = 0x55;
// const int ESP_JOY2Y = 0x56;
// const int ESP_JOY2Z = 0x57;

void sendKeyAction(int scancode, bool press) {
  keyboard.write(0xE2);
  delay(15);
  if (!press) {
    keyboard.write(0xF0);
    delay(15);
  }
  keyboard.write(scancode);
  delay(15);
}

// Controller states
word currentState = 0;
word lastState = 0;

void sendState()
{
    // Verificar el cambio de estado para cada botón
    if ((currentState & SC_BTN_UP) != (lastState & SC_BTN_UP)) {
        if (currentState & SC_BTN_UP) {
            Serial.println("Botón UP presionado");
            sendKeyAction(ESP_JOY1UP, true);
        } else {
            Serial.println("Botón UP soltado");
            sendKeyAction(ESP_JOY1UP, false);
        }
    }

    if ((currentState & SC_BTN_DOWN) != (lastState & SC_BTN_DOWN)) {
        if (currentState & SC_BTN_DOWN) {
            sendKeyAction(ESP_JOY1DOWN, true);
            Serial.println("Botón DOWN presionado");
        } else {
            sendKeyAction(ESP_JOY1DOWN, false);
            Serial.println("Botón DOWN soltado");
        }
    }

    if ((currentState & SC_BTN_LEFT) != (lastState & SC_BTN_LEFT)) {
        if (currentState & SC_BTN_LEFT) {
            sendKeyAction(ESP_JOY1LEFT, true);
            Serial.println("Botón LEFT presionado");
        } else {
          sendKeyAction(ESP_JOY1LEFT, false);
            Serial.println("Botón LEFT soltado");
        }
    }

    if ((currentState & SC_BTN_RIGHT) != (lastState & SC_BTN_RIGHT)) {
        if (currentState & SC_BTN_RIGHT) {
            sendKeyAction(ESP_JOY1RIGHT, true);
            Serial.println("Botón RIGHT presionado");
        } else {
            sendKeyAction(ESP_JOY1RIGHT, false);
            Serial.println("Botón RIGHT soltado");
        }
    }

    if ((currentState & SC_BTN_START) != (lastState & SC_BTN_START)) {
        if (currentState & SC_BTN_START) {
            sendKeyAction(ESP_JOY1START, true);
            Serial.println("Botón START presionado");
        } else {
            sendKeyAction(ESP_JOY1START, false);
            Serial.println("Botón START soltado");
        }
    }

    if ((currentState & SC_BTN_A) != (lastState & SC_BTN_A)) {
        if (currentState & SC_BTN_A) {
            sendKeyAction(ESP_JOY1A, true);
            Serial.println("Botón A presionado");
        } else {
            sendKeyAction(ESP_JOY1A, false);
            Serial.println("Botón A soltado");
        }
    }

    if ((currentState & SC_BTN_B) != (lastState & SC_BTN_B)) {
        if (currentState & SC_BTN_B) {
            sendKeyAction(ESP_JOY1B, true);
            Serial.println("Botón B presionado");
        } else {
            sendKeyAction(ESP_JOY1B, false);
            Serial.println("Botón B soltado");
        }
    }

    if ((currentState & SC_BTN_C) != (lastState & SC_BTN_C)) {
        if (currentState & SC_BTN_C) {
            sendKeyAction(ESP_JOY1C, true);
            Serial.println("Botón C presionado");
        } else {
            sendKeyAction(ESP_JOY1C, false);
            Serial.println("Botón C soltado");
        }
    }

    if ((currentState & SC_BTN_X) != (lastState & SC_BTN_X)) {
        if (currentState & SC_BTN_X) {
            sendKeyAction(ESP_JOY1X, true);
            Serial.println("Botón X presionado");
        } else {
            sendKeyAction(ESP_JOY1X, false);
            Serial.println("Botón X soltado");
        }
    }

    if ((currentState & SC_BTN_Y) != (lastState & SC_BTN_Y)) {
        if (currentState & SC_BTN_Y) {
            sendKeyAction(ESP_JOY1Y, true);
            Serial.println("Botón Y presionado");
        } else {
            sendKeyAction(ESP_JOY1Y, false);
            Serial.println("Botón Y soltado");
        }
    }

    if ((currentState & SC_BTN_Z) != (lastState & SC_BTN_Z)) {
        if (currentState & SC_BTN_Z) {
            sendKeyAction(ESP_JOY1Z, true);
            Serial.println("Botón Z presionado");
        } else {
            sendKeyAction(ESP_JOY1Z, false);
            Serial.println("Botón Z soltado");
        }
    }

    if ((currentState & SC_BTN_1) != (lastState & SC_BTN_1)) {
        if (currentState & SC_BTN_1) {
            sendKeyAction(ESP_JOY1A, true);
            Serial.println("Botón 1 presionado");
        } else {
            sendKeyAction(ESP_JOY1A, false);
            Serial.println("Botón 1 soltado");
        }
    }

    if ((currentState & SC_BTN_2) != (lastState & SC_BTN_2)) {
        if (currentState & SC_BTN_2) {
            sendKeyAction(ESP_JOY1Z, true);
            Serial.println("Botón 2 presionado");
        } else {
            sendKeyAction(ESP_JOY1Z, false);
            Serial.println("Botón 2 soltado");
        }
    }

    if ((currentState & SC_BTN_MODE) != (lastState & SC_BTN_MODE)) {
        if (currentState & SC_BTN_MODE) {
            sendKeyAction(ESP_JOY1MODE, true);
            Serial.println("Botón MODE presionado");
        } else {
            sendKeyAction(ESP_JOY1MODE, false);
            Serial.println("Botón MODE soltado");
        }
    }

    // Actualizar el último estado
    lastState = currentState;
}

void setup() {
    Serial.begin(9600);

    int joystickType = controller.detectJoystickType();
    if (joystickType == 1) {
        Serial.println("Sega Joystick detectado");
        // Configura para Sega Joystick
    } else if (joystickType == 2) {
        Serial.println("Amstrad Joystick detectado");
        // Configura para Amstrad Joystick
    } else {
        Serial.println("Joystick no detectado o desconocido");
    }
}

void loop()
{
    currentState = controller.getState();
    sendState();
}



// #include <Arduino.h>
// #include <GamePad.h>

// // Controller DB9 pins (looking face-on to the end of the plug):
// //
// // 5 4 3 2 1
// //  9 8 7 6
// //
// // Connect pin 5 to +5V and pin 8 to GND
// // Connect the remaining pins to digital I/O pins (see below)

// // Specify the Arduino pins that are connected to
// // DB9 Pin 7, DB9 Pin 1, DB9 Pin 2, DB9 Pin 3, DB9 Pin 4, DB9 Pin 6, DB9 Pin 9
// GamePad controller(8, 2, 3, 4, 5, 6, 7);

// // Controller states
// word currentState = 0;
// word lastState = 0;

// void sendState()
// {
//     // Solo reportar el estado del controlador si ha cambiado
//     if (currentState != lastState)
//     {
//         // Verificar el cambio de estado para cada botón
//         if (currentState & SC_BTN_UP) {
//             Serial.println("Botón UP presionado");
//         } else {
//             Serial.println("Botón UP no presionado");
//         }

//         if (currentState & SC_BTN_DOWN) {
//             Serial.println("Botón DOWN presionado");
//         } else {
//             Serial.println("Botón DOWN no presionado");
//         }

//         if (currentState & SC_BTN_LEFT) {
//             Serial.println("Botón LEFT presionado");
//         } else {
//             Serial.println("Botón LEFT no presionado");
//         }

//         if (currentState & SC_BTN_RIGHT) {
//             Serial.println("Botón RIGHT presionado");
//         } else {
//             Serial.println("Botón RIGHT no presionado");
//         }

//         if (currentState & SC_BTN_START) {
//             Serial.println("Botón START presionado");
//         } else {
//             Serial.println("Botón START no presionado");
//         }

//         if (currentState & SC_BTN_A) {
//             Serial.println("Botón A presionado");
//         } else {
//             Serial.println("Botón A no presionado");
//         }

//         if (currentState & SC_BTN_B) {
//             Serial.println("Botón B presionado");
//         } else {
//             Serial.println("Botón B no presionado");
//         }

//         if (currentState & SC_BTN_C) {
//             Serial.println("Botón C presionado");
//         } else {
//             Serial.println("Botón C no presionado");
//         }

//         if (currentState & SC_BTN_X) {
//             Serial.println("Botón X presionado");
//         } else {
//             Serial.println("Botón X no presionado");
//         }

//         if (currentState & SC_BTN_Y) {
//             Serial.println("Botón Y presionado");
//         } else {
//             Serial.println("Botón Y no presionado");
//         }

//         if (currentState & SC_BTN_Z) {
//             Serial.println("Botón Z presionado");
//         } else {
//             Serial.println("Botón Z no presionado");
//         }

//         if (currentState & SC_BTN_MODE) {
//             Serial.println("Botón MODE presionado");
//         } else {
//             Serial.println("Botón MODE no presionado");
//         }

//         // Actualizar el último estado
//         lastState = currentState;
//     }
// }

// void setup()
// {
//     Serial.begin(9600);
// }

// void loop()
// {
//     currentState = controller.getState();
//     sendState();
// }
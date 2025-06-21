# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 1.5.1)
set(toolchainVersion 13_2_Rel1)
set(picotoolVersion 2.0.0)
set(picoVscode ${USERHOME}/.pico-sdk/sdk/2.1.0/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Cambia esta línea para seleccionar la placa deseada
set(PICO_BOARD waveshare_rp2040_zero CACHE STRING "Board type")
#set(PICO_BOARD pico CACHE STRING "Board type")

set(MS_RATE_DEFAULT 100 CACHE STRING "Default mouse sample rate")
set(MS_RATE_HOST_CONTROL ON CACHE BOOL "Allow the host to configure the mouse sample rate")

# Pull in Raspberry Pi Pico SDK
include(pico_sdk_import.cmake)

if (PICO_SDK_VERSION_STRING VERSION_LESS "1.5.1")
  message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.5.1 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

project(usb2ps2 C CXX ASM)

pico_sdk_init()

# add_compile_definitions(PICO_DEFAULT_LED_PIN=10)
add_compile_definitions(PICO_DEFAULT_LED_PIN=25)
add_executable(usb2ps2 src/usb2ps2.c src/scancodesets.c src/ps2kb.c src/ps2ms.c src/ps2out.c src/ps2in.c src/cassette.c)

pico_generate_pio_header(usb2ps2 ${CMAKE_CURRENT_LIST_DIR}/src/ps2out.pio)
pico_generate_pio_header(usb2ps2 ${CMAKE_CURRENT_LIST_DIR}/src/ps2in.pio)

### BUTTON GAMEPAD
add_compile_definitions(BUTTON_TESTING=22) # Button testing
add_compile_definitions(CASSETTE_REC=28) 
add_compile_definitions(CASSETTE_PLAY=27) 
add_compile_definitions(CASSETTE_REW=26) 
add_compile_definitions(CASSETTE_FF=9) 
add_compile_definitions(CASSETTE_STOP=8) 
add_compile_definitions(CASSETTE_PAUSE=2)

add_compile_definitions(DEBOUNCE_TIME=100) # tiempo de rebote

### PS/2 output
add_compile_definitions(LVOUT=13) # Level shifter pull-up voltage
add_compile_definitions(KBOUT=11) # Keyboard data pin, clock pin es 12
add_compile_definitions(MSOUT=14) # Mouse data pin, clock pin es 15

### PS/2 input
add_compile_definitions(LVIN=5) # Level shifter pull-up voltage
add_compile_definitions(KBIN=3) # Keyboard data pin, clock pin es 4
add_compile_definitions(MSIN=6) # Mouse data pin, clock pin es 7

add_compile_definitions(PICO_PANIC_FUNCTION=reset)

add_compile_definitions(MS_RATE_DEFAULT=${MS_RATE_DEFAULT})
if (MS_RATE_HOST_CONTROL)
    add_compile_definitions(MS_RATE_HOST_CONTROL)
endif()

pico_set_program_name(usb2ps2 "usb2ps2")
pico_set_program_version(usb2ps2 "1.1")

pico_enable_stdio_uart(usb2ps2 1)
pico_enable_stdio_usb(usb2ps2 0)

target_include_directories(usb2ps2 PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src)
target_link_libraries(usb2ps2 pico_stdlib hardware_pio tinyusb_host tinyusb_board)

# Set the output name of the executable
set_target_properties(usb2ps2 PROPERTIES OUTPUT_NAME usb2ps2)

pico_add_extra_outputs(usb2ps2)

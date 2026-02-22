# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.2.0)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.2.0-a4)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico CACHE STRING "Board type")

# Set minimum required version of CMake
cmake_minimum_required(VERSION 3.12)

# generate compile_commands.json for IntelliSense / tools
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Include build functions from Pico SDK
include(pico_sdk_import.cmake)

# Set name of project (as PROJECT_NAME) and C/C++ standards
project(blink C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Creates a pico-sdk subdirectory in our project for the libraries
pico_sdk_init()

# Tell CMake where to find the executable source files
add_executable(${PROJECT_NAME} 
    main.c
    motor_control.c
    lcd_i2c.c
    button.c
    rgb_led.c
    keypad.c
)

# Make local headers visible to the target (optional but helps IntelliSense)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_LIST_DIR})

# Create map/bin/hex/uf2 files
pico_add_extra_outputs(${PROJECT_NAME})

# Link to pico_stdlib (gpio, time, etc. functions) and hardware libs used by drivers
target_link_libraries(${PROJECT_NAME} 
    pico_stdlib
    hardware_i2c
    hardware_pwm
)

# Enable usb output, disable uart output
pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)
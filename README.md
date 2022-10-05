# TURAG-Feldbus
Simple yet feature-rich serial bus protocol for embedded devices.

This repository contains the reference implementation for bus devices.

The protocol specification can be found in the [wiki](https://github.com/turag-ev/TURAG-Feldbus/wiki).

Code reference: https://turag-ev.github.io/TURAG-Feldbus/modules.html

Currently, host implementations can be found within the [Tina library](https://github.com/turag-ev/Tina). 

The [TURAG-Console](https://github.com/turag-ev/TURAG-Console) makes use of the host implementations and allows it to communicate with bus devices through a serial port.

## Usage
To enable your project to act as a TURAG-Feldbus device, you need
* clone the repository or add it to your project as a git submodule
* configure the contents of the directory _TURAG-Feldbus/src/feldbus_ to be compiled into your project. C++ is required.
* make the directory _TURAG-Feldbus/src_ visible to the compiler as an include path
* copy the file _TURAG-Feldbus/src/feldbus/device/feldbus_config.h_ into to a different directory (e.g. next to your main.c file), remove the compiler error at the top of the file and adjust it to your needs
* make sure the directory containing the new feldbus_config.h file is visible to the compiler as an include path.

Note for users of STM32CubeIDE: keep in mind that include paths are set separately for languages and configurations.

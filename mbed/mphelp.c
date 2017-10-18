/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2016 Colin Hogben
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
 */
#include "py/runtime.h"
#if MBED_CONF_MICROPYTHON_WITH_HELP

const char mbed_mpy_help_text[] =
    "Welcome to the (unofficial) mbed port of MicroPython!\n"
    "For online help please visit http://micropython.org/help/.\n"
    "For further help on a specific object, type help(obj)\n"
    "\n"
    "The following modules are available:\n"
#if MICROPY_PY_SYS
    "  sys\n"
#endif
#if MICROPY_PY_STRUCT
    "  ustruct -- cut-down version of struct\n"
#endif
    "  machine -- direct memory access and reset function\n"
    "  mbed    -- equivalents to mbed API classes:\n"
#if MICROPY_MBED_ANALOGIN
    "    AnalogIn\n"
#endif
#if MICROPY_MBED_ANALOGOUT
    "    AnalogOut\n"
#endif
#if MICROPY_MBED_DIGITALIN
    "    DigitalIn\n"
#endif
#if MICROPY_MBED_DIGITALOUT
    "    DigitalOut\n"
#endif
#if MICROPY_MBED_I2C
    "    I2C\n"
#endif
#if MICROPY_MBED_PWMOUT
    "    PwmOut\n"
#endif
#if MICROPY_MBED_SERIAL
    "    Serial\n"
#endif
#if MICROPY_MBED_SPI
    "    SPI\n"
#endif
#if MICROPY_PY_NET
    "  net     -- equivalents to mbed network classes:\n"
    "    EthernetInterface\n"
    "    SocketAddress\n"
    "    TCPServer\n"
    "    TCPSocket\n"
    "    UDPSocket\n"
#endif
    "  pins    -- board-specific and generic named pins e.g. USBTX\n"
    ;

#endif // MBED_CONF_MICROPYTHON_WITH_HELP

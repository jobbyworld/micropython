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

/**
 * A read-eval-print-loop for micropython on the mbed platform.
 *
 * The USB serial link is used.
 */
#include "mbed.h"
extern "C" {
#include "py/runtime.h"
#include "py/mphal.h"
#include "lib/utils/pyexec.h"
#include "mpc.h"
}
#include "mprepl.h"

// Serial communication to host PC via USB
Serial pc(USBTX, USBRX);

MPC_VOIDFUNC_3(printf, STRING, INT, STRING);

// Now the main program - run the REPL.
int main() {
    mpc_add_func("printf", mpc_f_printf);
    mp_init();
    mpc_populate_globals();
    mprepl_add_Serial(pc);
    mprepl_run();
    mp_deinit();
    return 0;
}

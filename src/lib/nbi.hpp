
/* License:
 *
 * MIT License
 *
 * Copyright (c) 2020 magistermaks
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* Main feature of this library is the nbi_get_char(),
 * which is platform independent non-blocking input function. It returns key from input stream (stdin) or
 * -1 if the stream is empty. It removes the returned char from the stream.
 *
 * Other functions:
 * void nbi_clear()               - Removes all characters from input stream, so that nbi_get_char() will return -1, and nbi_get_flag() false.
 * char nbi_std_input()           - Waits for key press and returns it, acts like windows' getch() but is platform independent.
 * void nbi_wait()                - Waits for key press, then continues execution.
 * void nbi_set_echo( bool echo ) - Sets current echo mode. When set to true nbi_get_char(), nbi_std_input() will display pressed char, the default value is false.
 * bool nbi_get_flag()            - Returns true if there is any key in the stream, and false otherwise. (platform independent version of _kbhit())
 *
 * Functions avaible only on linux-like systems:
 * void nbi_termios_update()      - Must be called after using tcsetattr() from "termios.h".
 */

/* Key enums:
 * NBI_KEY_ESC
 * NBI_KEY_TAB
 * NBI_KEY_BACKSPACE
 * NBI_KEY_ENTER
 * NBI_KEY_SPACE
 *
 * No key enum: (-1)
 * NBI_KEY_NONE
 */

/* Options: (define to enable)
 * NBI_LIB_WINDOWS        - Don't check build environment, assume windows.
 * NBI_LIB_LINUX          - Don't check build environment, assume linux-like.
 * NBI_LIB_ASSUME_STDIO   - Don't include stdio.h, assume already included.
 * NBI_LIB_ASSUME_TERMIOS - Don't include termios.h, assume already included. (If you are using termios you may want to read more about nbi_termios_update() and NBI_LIB_NO_TERMIOS_POP)
 * NBI_LIB_NO_TERMIOS_POP - Is ignored if not compiled on linux, makes nbi_get_char() calls (technically) faster. Breaks input line buffering.
 * NBI_LIB_IMPLEMENTATION - This file will act as .c not .h
 */

/* Limitations:
 * This library implements platform independent non-blocking input, but it doesn't
 * supply any way to interpret that input. Different systems encode (some) keys in different ways,
 * e.g. Arrows on MacOS are encoded as 3 chars: 27, 91, [65, 66, 67, 68] (UP, DOWN, RIGHT, LEFT)
 * while Windows use only 2 chars: -32, [72, 80, 77, 75].
 */

/* Versions:
 * 1.0 - Initial version.
 * 1.1 - Bug fix.
 * 1.2 - Added enums.
 * 1.3 - Fixed typos.
 * 1.4 - Fixed typo.
 * 1.5 - Fixed bugs, renamed functions.
 */

#ifdef NBI_LIB_WINDOWS
#define __NBI_LIB_WINDOWS
#elif defined NBI_LIB_LINUX
#define __NBI_LIB_LINUX
#else
#ifdef _WIN32
#define __NBI_LIB_WINDOWS
#elif defined __APPLE__
#define __NBI_LIB_LINUX
#elif defined __linux__
#define __NBI_LIB_LINUX
#elif defined __unix__
#define __NBI_LIB_LINUX
#else
#error "Build environment test failed. Define NBI_LIB_WINDOWS or NBI_LIB_LINUX."
#endif
#endif

#ifndef __NBI_HEADER
#define __NBI_HEADER

#ifndef NBI_LIB_ASSUME_STDIO
#include <stdio.h>
#endif

#ifdef __NBI_LIB_WINDOWS
#include <conio.h>
#endif

#ifdef __NBI_LIB_LINUX
#include <sys/types.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#ifndef NBI_LIB_ASSUME_TERMIOS
#include <termios.h>
#endif
#endif

#ifdef __NBI_LIB_WINDOWS
#define NBI_KEY_ESC 27
#define NBI_KEY_TAB 9
#define NBI_KEY_BACKSPACE 8
#define NBI_KEY_ENTER 13
#define NBI_KEY_SPACE 32
#endif

#ifdef __NBI_LIB_LINUX
#define NBI_KEY_ESC 27
#define NBI_KEY_TAB 9
#define NBI_KEY_BACKSPACE 8
#define NBI_KEY_ENTER 10
#define NBI_KEY_SPACE 32
#endif

#define NBI_KEY_NONE -1

#ifdef __cplusplus
extern "C" {
#endif

char nbi_get_char();
bool nbi_get_flag();
char nbi_std_input();
void nbi_set_echo( bool echo );
void nbi_clear();
void nbi_wait();

#ifdef __NBI_LIB_LINUX
void __nbi_termios_init();
void __nbi_termios_push();
void __nbi_termios_pop();

void nbi_termios_update();
#endif

extern bool __nbi_echo;
#ifdef __NBI_LIB_LINUX
extern struct termios __nbi_termios_state_required;
extern struct termios __nbi_termios_state_current;
extern bool __nbi_termios_initialized;
extern bool __nbi_termios_state;
#endif

#ifdef __cplusplus
}
#endif

#endif

#ifdef NBI_LIB_IMPLEMENTATION
#undef NBI_LIB_IMPLEMENTATION

#ifdef __cplusplus
extern "C" {
#endif

bool __nbi_echo = false;

#ifdef __NBI_LIB_LINUX
struct termios __nbi_termios_state_required;
struct termios __nbi_termios_state_current;
bool __nbi_termios_initialized = false;
bool __nbi_termios_state = false;

void __nbi_termios_init() {
    tcgetattr(0, &__nbi_termios_state_required);
    __nbi_termios_state_current = __nbi_termios_state_required;
    __nbi_termios_state_required.c_lflag &= ~ICANON;
    nbi_set_echo( __nbi_echo );
    __nbi_termios_push();
}

void __nbi_termios_push() {
    if( __nbi_termios_state ) return;
    __nbi_termios_state = true;
    tcsetattr(0, TCSANOW, &__nbi_termios_state_required);
}

void __nbi_termios_pop() {
    __nbi_termios_state = false;
    tcsetattr(0, TCSANOW, &__nbi_termios_state_current);
}

void nbi_termios_update() {
    tcgetattr(0, &__nbi_termios_state_current);
}
#endif

char nbi_get_char() {
    if( nbi_get_flag() )
        return nbi_std_input();
    else
        return -1;
}

bool nbi_get_flag() {
#ifdef __NBI_LIB_WINDOWS
    return (bool)_kbhit();
#elif defined __NBI_LIB_LINUX
    if( !__nbi_termios_initialized ){
        __nbi_termios_init();
    }
    __nbi_termios_push();

    timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;

    fd_set fd;
    FD_ZERO(&fd);
    FD_SET(0, &fd);

    bool flag = (bool)select(1, &fd, NULL, NULL, &timeout);
#ifndef NBI_LIB_NO_TERMIOS_POP
    __nbi_termios_pop();
#endif
    return flag;
#endif
}

char nbi_std_input() {
#ifdef __NBI_LIB_WINDOWS
    if( __nbi_echo )
        return _getche();
    else
        return _getch();
#elif defined __NBI_LIB_LINUX
    if( !__nbi_termios_initialized ){
        __nbi_termios_init();
    }
    __nbi_termios_push();
    char chr = getchar();
#ifndef NBI_LIB_NO_TERMIOS_POP
    __nbi_termios_pop();
#endif
    return chr;
#endif
}

void nbi_set_echo( bool echo ) {
    __nbi_echo = echo;
#ifdef __NBI_LIB_LINUX
    __nbi_termios_state = false;
    if( echo )
        __nbi_termios_state_required.c_lflag |= ECHO;
    else
        __nbi_termios_state_required.c_lflag &= ~ECHO;
#endif
}

void nbi_clear() {
    while( nbi_get_flag() ) nbi_get_char();
}

void nbi_wait() {
    nbi_std_input();
}

#ifdef __cplusplus
}
#endif

#endif

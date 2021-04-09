
/*
 * MIT License
 *
 * Copyright (c) 2020, 2021 magistermaks
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

#pragma once

#ifdef __linux__
#	define __SEQ_USE_LINUX
#endif

#ifdef __unix__
#	define __SEQ_USE_LINUX
#endif

// linux only headers
#ifdef __SEQ_USE_LINUX
#	define __SEQ_USE_POSIX
#	include <linux/limits.h>
#	define SEQ_LIB_NAME "native.so"
#	define SQ_TARGET "linux"
#endif

// apple only headers
#ifdef __APPLE__
#	define __SEQ_USE_POSIX
#	include <sys/syslimits.h>
#	define SEQ_LIB_NAME "native.so"
#	define SQ_TARGET "darwin"
#endif

// generic posix
#ifdef __SEQ_USE_POSIX
#	include <unistd.h>
#	include <sys/stat.h>
#	define LIBLOAD_LINUX
#	define CWD_MAX_PATH PATH_MAX
#	define POSIX_GETCWD getcwd
#	define POSIX_MKDIR(dir) mkdir( dir, ACCESSPERMS )
#endif

// windows only headers
#ifdef _WIN32
#	include <windows.h>
#	include <direct.h>
#	define SEQ_LIB_NAME "native.dll"
#	define LIBLOAD_WINDOWS
#	define CWD_MAX_PATH MAX_PATH
#	define POSIX_GETCWD _getcwd
#	define SQ_TARGET "windows"
#	define POSIX_MKDIR(dir) _mkdir( dir )
#endif

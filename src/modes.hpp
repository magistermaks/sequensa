
/*
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

#ifndef MODES_HPP_
#define MODES_HPP_

#define SQ_VER "1.0"

#include <iostream>
#include <fstream>
#include <vector>

// SeqAPI is re-included in every module
// because my IDE was complaining about "undefined symbols"
#include "api/SeqAPI.hpp"
#include "lib/argparse.hpp"
#include "lib/whereami.h"
#include "lib/detector.hpp"

// linux only headers
#ifdef SEQ_LINUX
#	include <unistd.h>
#	include <linux/limits.h>
#	define SEQ_LIB_NAME "native.so"
#	define LIBLOAD_LINUX
#	define CWD_MAX_PATH PATH_MAX
#	define POSIX_GETCWD getcwd
#	define SQ_TARGET "linux"
#endif

// windows only headers
#ifdef SEQ_WINDOWS
#	include <windows.h>
#	include <direct.h>
#	define SEQ_LIB_NAME "native.dll"
#	define LIBLOAD_WINDOWS
#	define CWD_MAX_PATH MAX_PATH
#	define POSIX_GETCWD _getcwd
#	define SQ_TARGET "windows"
#endif

typedef int (*DynLibInit) (seq::Executor*,seq::FileHeader*);

// command line flags
struct Options {
	bool verbose: 1;
	bool force_execution: 1;
	bool print_all: 1;
	bool print_none: 1;
	bool strict_math: 1;
	bool multi_error: 1;
};

void help( ArgParse& argp, Options opt );
void build( ArgParse& argp, Options opt );
void run( ArgParse& argp, Options opt );

// utils - implemented in utils.cpp
bool file_exist( const char *path );
std::string get_exe_path();
std::string get_cwd_path();
std::string get_base_name( std::string path );
size_t get_path_hash( std::string path );
std::string get_absolute_path( std::string relative, std::string base );
std::string get_directory( std::string& path );

#endif /* MODES_HPP_ */

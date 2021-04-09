
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

#ifndef MODULES_HPP_
#define MODULES_HPP_

#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>

// SeqAPI is re-included in every module
// because my IDE was complaining about "undefined symbols"
#include "api/SeqAPI.hpp"
#include "lib/argparse.hpp"
#include "lib/whereami.h"
#include "lib/system.hpp"

// dynamic library entry point
typedef int (*DynLibInit) (seq::Executor*,seq::FileHeader*);

// command line flags
struct Options {
	bool verbose: 1;
	bool force_execution: 1;
	bool print_exit: 1;
	bool strict_math: 1;
	bool no_multi_error: 1;
	bool optimize: 1;
};

#define USAGE_HELP( error, mode ) std::cout << error << "\nUse '--help " << mode << "' for usage help." << std::endl;

void help( ArgParse& argp );
void build( ArgParse& argp, Options opt );
void run( ArgParse& argp, Options opt );
void info( ArgParse& argp, Options opt );
void decompile( ArgParse& argp, Options opt );
void shell( ArgParse& argp, Options opt );

// utils - implemented in utils.cpp
std::map<std::string, std::string> build_header_map( std::vector<std::string>& natives, std::vector<std::string>& strings );
void unload_native_libs();
bool load_native_libs( seq::Executor& exe, seq::FileHeader& header, bool verbose );
bool load_header( seq::FileHeader* header, seq::BufferReader& br );
std::string posix_time_to_date( time_t rawtime );
bool validate_version( seq::FileHeader& header, bool force, bool verbose );
bool file_exist( const char *path );
std::string get_exe_path();
std::string get_cwd_path();
std::string get_base_name( std::string path );
size_t get_path_hash( std::string path );
std::string get_absolute_path( std::string relative, std::string base );
std::string get_directory( std::string& path );

#endif /* MODULES_HPP_ */

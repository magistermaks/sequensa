
#ifndef MODES_HPP_
#define MODES_HPP_

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
#endif

// windows only headers
#ifdef SEQ_WINDOWS
#	include <windows.h>
#	include <direct.h>
#	define SEQ_LIB_NAME "native.dll"
#	define LIBLOAD_WINDOWS
#	define CWD_MAX_PATH MAX_PATH
#	define POSIX_GETCWD _getcwd
#endif

typedef int (*DynLibInit) (seq::Executor*,seq::FileHeader*);

// command line flags
struct Options {
	bool verbose: 1;
	bool force_execution: 1;
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

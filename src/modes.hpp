
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

// linux only headers
#include <unistd.h>
#include <linux/limits.h>

typedef int (*DynLibInit) (seq::Executor*,seq::FileHeader*);

#define SEQ_LIB_NAME "native.so"

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

#endif /* MODES_HPP_ */

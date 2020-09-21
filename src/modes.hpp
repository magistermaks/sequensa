
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

typedef int (*DynLibInit) (seq::Executor*,seq::FileHeader*);

#define SEQ_LIB_NAME "native.so"

void help( ArgParse& argp );
void build( ArgParse& argp );
void run( ArgParse& argp );

// utils - implemented in ./utils.cpp
bool file_exist( const char *path );
std::string get_exe_path();

#endif /* MODES_HPP_ */

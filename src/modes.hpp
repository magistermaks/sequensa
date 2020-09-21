
#ifndef MODES_HPP_
#define MODES_HPP_

#include <iostream>
#include <fstream>
#include <vector>

#include "SeqAPI.hpp"
#include "argparse.hpp"
#include "lib/whereami.h"

typedef bool (*DynLibInit) (seq::Executor*,seq::FileHeader*);

#define SEQ_LIB_NAME "native.so"

void help( ArgParse& argp );
void build( ArgParse& argp );
void run( ArgParse& argp );

#endif /* MODES_HPP_ */

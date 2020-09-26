
#ifndef STD_COMMON_HPP_
#define STD_COMMON_HPP_

#define SEQ_IMPLEMENT
#define SEQ_EXCLUDE_COMPILER
#include "../api/SeqAPI.hpp"

#define INIT_SUCCESS 0
#define INIT_ERROR 1
#define INIT extern "C" int init
#define EMPTY seq::Stream()

#undef SEQ_EXCLUDE_COMPILER

#endif /* STD_COMMON_HPP_ */

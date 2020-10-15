
#ifndef STD_COMMON_HPP_
#define STD_COMMON_HPP_

#define SEQ_IMPLEMENT
#define SEQ_EXCLUDE_COMPILER
#include "../api/SeqAPI.hpp"

#ifdef _WIN32
#	define __SEQ_DECLSPEC __declspec(dllexport)
#	define SQ_SYSTEM "windows"
#else
#	define __SEQ_DECLSPEC
#	define SQ_SYSTEM "linux"
#endif

#define INIT_SUCCESS 0
#define INIT_ERROR 1
#define INIT extern "C" __SEQ_DECLSPEC int init
#define EMPTY seq::Stream()

#undef SEQ_EXCLUDE_COMPILER

#endif /* STD_COMMON_HPP_ */

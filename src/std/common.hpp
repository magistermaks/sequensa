
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

#ifndef STD_COMMON_HPP_
#define STD_COMMON_HPP_

#ifndef NO_EXCLUDE_COMPILER
#	define SEQ_EXCLUDE_COMPILER
#endif

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

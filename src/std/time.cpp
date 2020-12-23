
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

#include "common.hpp"
#include "../lib/system.hpp"

void ms_sleep( long miliseconds ) {

#ifdef __SEQ_USE_POSIX
	struct timespec req, rem;

	if(miliseconds > 999) {
		req.tv_sec = (int)(miliseconds / 1000);
		req.tv_nsec = (miliseconds - ((long)req.tv_sec * 1000)) * 1000000;
	} else {
		req.tv_sec = 0;
		req.tv_nsec = miliseconds * 1000000;
	}

	nanosleep(&req , &rem);
#endif

#ifdef _WIN32
	Sleep( miliseconds );
#endif

}

#define MILLIS std::chrono::milliseconds
#define THREAD std::this_thread

seq::Stream seq_std_time( seq::Stream& input ) {
	seq::Stream output;

	for( int i = input.size(); i > 0; i -- ) {

		output.push_back( seq::util::newNumber( std::time(0) ) );

	}

	return output;
}

seq::Stream seq_std_sleep( seq::Stream& input ) {
	seq::Stream output;

	for( auto& arg : input ) {

		ms_sleep( (unsigned long) seq::util::numberCast(arg).Number().getLong() );

	}

	return output;
}

INIT( seq::Executor* exe, seq::FileHeader* head ) {

	exe->inject( "std:time", seq_std_time );
	exe->inject( "std:sleep", seq_std_sleep );

	return INIT_SUCCESS;
}


#include "common.hpp"
#include "../lib/detector.hpp"

#ifdef SEQ_WINDOWS
#	include <windows.h>
#endif

void ms_sleep( long miliseconds ) {

#ifdef SEQ_LINUX
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

#ifdef SEQ_WINDOWS
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

	exe->inject( "std:time"_b, seq_std_time );
	exe->inject( "std:sleep"_b, seq_std_sleep );

	return INIT_SUCCESS;
}

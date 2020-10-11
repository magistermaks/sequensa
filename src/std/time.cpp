
#include "common.hpp"
#include <chrono>
#include <thread>

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

		// Eclipse has some weird problems with parsing std::chrono
		// But it compiles anyway
		MILLIS ms { (unsigned long) seq::util::numberCast(arg).Number().getLong() };
		THREAD::sleep_for( ms );

	}

	return output;
}

INIT( seq::Executor* exe, seq::FileHeader* head ) {

	exe->inject( "std:time"_b, seq_std_time );
	exe->inject( "std:sleep"_b, seq_std_sleep );

	return INIT_SUCCESS;
}

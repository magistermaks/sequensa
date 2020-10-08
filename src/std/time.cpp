
#include "common.hpp"

seq::Stream seq_std_time( seq::Stream& input ) {
	seq::Stream output;

	for( int i = input.size(); i > 0; i -- ) {

		output.push_back( seq::Generic( new seq::type::Number( false, std::time(0) ) ) );

	}

	return output;
}


INIT( seq::Executor* exe, seq::FileHeader* head ) {

	exe->inject( "std:time"_b, seq_std_time );

	return INIT_SUCCESS;
}

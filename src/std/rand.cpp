
#include "common.hpp"

seq::Stream seq_std_rand( seq::Stream& input ) {
	seq::Stream output;

	for( int i = input.size(); i > 0; i -- ) {

		output.push_back( seq::util::newNumber( rand() ) );

	}

	return output;
}

seq::Stream seq_std_srand( seq::Stream& input ) {
	seq::Stream output;

	for( auto& arg : input ) {

		srand( seq::util::numberCast( arg ).Number().getLong() );

	}

	return output;
}

INIT( seq::Executor* exe, seq::FileHeader* head ) {

	exe->inject( "std:rand"_b, seq_std_rand );
	exe->inject( "std:srand"_b, seq_std_srand );

	return INIT_SUCCESS;
}


#include "common.hpp"

seq::Stream seq_std_uppercase( seq::Stream& input ) {

	seq::Stream output;

	for( auto& arg : input ) {

		std::string str = (char*) seq::util::stringCast( arg ).String().getString().c_str();
		std::transform(str.begin(), str.end(), str.begin(), toupper);
		output.push_back( seq::util::newString( (seq::byte*) str.c_str() ) );

	}

	return output;
}

seq::Stream seq_std_lowercase( seq::Stream& input ) {

	seq::Stream output;

	for( auto& arg : input ) {

		std::string str = (char*) seq::util::stringCast( arg ).String().getString().c_str();
		std::transform(str.begin(), str.end(), str.begin(), tolower);
		output.push_back( seq::util::newString( (seq::byte*) str.c_str() ) );

	}

	return output;
}

seq::Stream seq_std_concat( seq::Stream& input ) {

	seq::string str;

	for( auto& arg : input ) {

		str += seq::util::stringCast( arg ).String().getString();

	}

	return { seq::util::newString( str.c_str() ) };
}

seq::Stream seq_std_split( seq::Stream& input ) {

	seq::Stream output;
	seq::string delim = seq::util::stringCast( input[0] ).String().getString();

	for( long i = 1; i < (long) input.size(); i ++ ) {

		seq::string str2 = seq::util::stringCast( input[i] ).String().getString();
		size_t prev = 0, pos = 0;

		do {

			pos = str2.find(delim, prev);
			if(pos == std::string::npos) pos = str2.length();
			output.push_back( seq::util::newString( str2.substr(prev, pos-prev).c_str() ) );
			prev = pos + delim.length();

		} while( pos < str2.size() && prev < str2.size() );

	}

	return output;
}

INIT( seq::Executor* exe, seq::FileHeader* head ) {

	exe->inject( "std:uppercase"_b, seq_std_uppercase );
	exe->inject( "std:lowercase"_b, seq_std_lowercase );
	exe->inject( "std:concat"_b, seq_std_concat );
	exe->inject( "std:split"_b, seq_std_split );

	return INIT_SUCCESS;
}

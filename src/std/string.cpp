
#include "common.hpp"

seq::Stream seq_std_uppercase( seq::Stream& input ) {

	seq::Stream output;

	for( auto& arg : input ) {

		std::string str = (char*) seq::util::stringCast( arg ).String().getString().c_str();
		std::transform(str.begin(), str.end(), str.begin(), toupper);
		output.push_back( seq::Generic( new seq::type::String( false, (seq::byte*) str.c_str() ) ) );

	}

	return output;
}

seq::Stream seq_std_lowercase( seq::Stream& input ) {

	seq::Stream output;

	for( auto& arg : input ) {

		std::string str = (char*) seq::util::stringCast( arg ).String().getString().c_str();
		std::transform(str.begin(), str.end(), str.begin(), tolower);
		output.push_back( seq::Generic( new seq::type::String( false, (seq::byte*) str.c_str() ) ) );

	}

	return output;
}

seq::Stream seq_std_concat( seq::Stream& input ) {

	seq::string str;

	for( auto& arg : input ) {

		str += seq::util::stringCast( arg ).String().getString();

	}

	return { seq::Generic( new seq::type::String( false, str.c_str() ) ) };
}

INIT( seq::Executor* exe, seq::FileHeader* head ) {

	exe->inject( "std:uppercase"_b, seq_std_uppercase );
	exe->inject( "std:lowercase"_b, seq_std_lowercase );
	exe->inject( "std:concat"_b, seq_std_concat );

	return INIT_SUCCESS;
}

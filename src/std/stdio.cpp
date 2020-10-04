
#include "common.hpp"
#include "iostream"

seq::Stream seq_std_out( seq::Stream& input ) {

	for( auto& element : input ) {

		std::cout << (byte*) seq::util::stringCast( element ).String().getString().c_str();

	}

	return EMPTY;
}

seq::Stream seq_std_outln( seq::Stream& input ) {

	for( auto& element : input ) {

		std::cout << (byte*) seq::util::stringCast( element ).String().getString().c_str() << '\n';

	}

	return EMPTY;
}

seq::Stream seq_std_in( seq::Stream& input ) {

	seq::Stream output;

	for( int i = input.size(); i > 0; i -- ) {

		std::string str;
		std::cin >> str;
		output.push_back( seq::Generic( new seq::type::String( false, (seq::byte*) str.c_str() ) ) );

	}

	return output;
}

seq::Stream seq_std_flush( seq::Stream& input ) {

	std::cout.flags();

	return EMPTY;
}

INIT( seq::Executor* exe, seq::FileHeader* head ) {

	exe->inject( "std:out"_b, seq_std_out );
	exe->inject( "std:outln"_b, seq_std_outln );
	exe->inject( "std:in"_b, seq_std_out );
	exe->inject( "std:flush"_b, seq_std_flush );
	exe->define( "std:br"_b, { seq::Generic( new seq::type::String( false, "\n"_b ) ) } );

	return INIT_SUCCESS;
}

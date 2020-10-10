
#include "common.hpp"
#include "iostream"
#include "../lib/nbi.hpp"

#define NBI_LIB_IMPLEMENTATION
#include "../lib/nbi.hpp"
#undef NBI_LIB_IMPLEMENTATION

seq::Stream seq_std_out( seq::Stream& input ) {

	for( auto& element : input ) {

		std::cout << (seq::byte*) seq::util::stringCast( element ).String().getString().c_str();

	}

	return EMPTY;
}

seq::Stream seq_std_outln( seq::Stream& input ) {

	for( auto& element : input ) {

		std::cout << (seq::byte*) seq::util::stringCast( element ).String().getString().c_str() << '\n';

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

seq::Stream seq_std_inchr( seq::Stream& input ) {

	seq::Stream output;

	for( int i = input.size(); i > 0; i -- ) {

		std::string str;
		str += nbi_std_input();

		output.push_back( seq::Generic( new seq::type::String( false, (seq::byte*) str.c_str() ) ) );

	}

	return output;
}

seq::Stream seq_std_nbinchr( seq::Stream& input ) {

	seq::Stream output;

	for( int i = input.size(); i > 0; i -- ) {

		std::string str;
		char chr = nbi_get_char();

		if( chr == -1 ) {

			str += chr;
			output.push_back( seq::Generic( new seq::type::String( false, (seq::byte*) str.c_str() ) ) );

		}else{

			output.push_back( seq::Generic( new seq::type::Null( false ) ) );

		}

	}

	return output;
}

INIT( seq::Executor* exe, seq::FileHeader* head ) {

	exe->inject( "std:out"_b, seq_std_out );
	exe->inject( "std:outln"_b, seq_std_outln );
	exe->inject( "std:in"_b, seq_std_in );
	exe->inject( "std:flush"_b, seq_std_flush );
	exe->inject( "std:inchr"_b, seq_std_inchr );
	exe->inject( "std:nbinchr"_b, seq_std_nbinchr );

	exe->define( "std:br"_b, { seq::Generic( new seq::type::String( false, "\n"_b ) ) } );

	return INIT_SUCCESS;
}

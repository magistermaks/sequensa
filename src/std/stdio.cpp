
/*
 * MIT License
 *
 * Copyright (c) 2020, 2021 magistermaks
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
#include "iostream"
#include "../lib/nbi.hpp"

#define NBI_LIB_IMPLEMENTATION
#include "../lib/nbi.hpp"
#undef NBI_LIB_IMPLEMENTATION

seq::Stream* seq_std_out( seq::Stream* input ) {

	for( auto& element : *input ) {

		std::cout << (seq::byte*) seq::util::stringCast( element ).String().getString().c_str();

	}

	input->clear();
	return nullptr;
}

seq::Stream* seq_std_outln( seq::Stream* input ) {

	for( auto& element : *input ) {

		std::cout << (seq::byte*) seq::util::stringCast( element ).String().getString().c_str() << std::endl;

	}

	input->clear();
	return nullptr;
}

seq::Stream* seq_std_in( seq::Stream* input ) {

	seq::Stream* output = new seq::Stream();

	for( int i = input->size(); i > 0; i -- ) {

		std::string str;
		std::cin >> str;
		output->push_back( seq::util::newString( str.c_str() ) );

	}

	return output;
}

seq::Stream* seq_std_flush( seq::Stream* input ) {

	std::cout << std::flush;

	input->clear();
	return nullptr;
}

seq::Stream* seq_std_inchr( seq::Stream* input ) {

	seq::Stream* output = new seq::Stream();

	for( int i = input->size(); i > 0; i -- ) {

		std::string str;
		str += nbi_std_input();

		output->push_back( seq::util::newString( str.c_str() ) );

	}

	return output;
}

seq::Stream* seq_std_nbinchr( seq::Stream* input ) {

	seq::Stream* output = new seq::Stream();

	for( int i = input->size(); i > 0; i -- ) {

		std::string str;
		char chr = nbi_get_char();

		if( chr != -1 ) {

			str += chr;
			output->push_back( seq::util::newString( str.c_str() ) );

		}else{

			output->push_back( seq::util::newNull() );

		}

	}

	return output;
}

INIT( seq::Executor* exe, seq::FileHeader* head ) {

	exe->inject( "std:out", seq_std_out );
	exe->inject( "std:outln", seq_std_outln );
	exe->inject( "std:in", seq_std_in );
	exe->inject( "std:flush", seq_std_flush );
	exe->inject( "std:inchr", seq_std_inchr );
	exe->inject( "std:nbinchr", seq_std_nbinchr );

	exe->define( "std:br", { seq::Generic( new seq::type::String( false, "\n" ) ) } );

	return INIT_SUCCESS;
}

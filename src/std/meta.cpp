
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

std::map<std::string, std::string> values;
seq::byte major;
seq::byte minor;
seq::byte patch;

seq::Stream seq_std_meta_major( seq::Stream& input ) {
	seq::Stream output;

	for( int i = input.size(); i > 0; i -- ) {

		output.push_back( seq::util::newNumber( (int) major ) );

	}

	return output;
}

seq::Stream seq_std_meta_minor( seq::Stream& input ) {
	seq::Stream output;

	for( int i = input.size(); i > 0; i -- ) {

		output.push_back( seq::util::newNumber( (int) minor ) );

	}

	return output;
}

seq::Stream seq_std_meta_patch( seq::Stream& input ) {
	seq::Stream output;

	for( int i = input.size(); i > 0; i -- ) {

		output.push_back( seq::util::newNumber( (int) patch ) );

	}

	return output;
}

seq::Stream seq_std_meta_value( seq::Stream& input ) {
	seq::Stream output;

	for( auto& arg : input ) {

		if( arg.getDataType() != seq::DataType::String ) {

			output.push_back( seq::util::newNull() );
			continue;

		}

		try{

			output.push_back( seq::util::newString(  values.at( arg.String().getString().c_str() ).c_str() ) );

		}catch( std::out_of_range& err ) {

			output.push_back( seq::util::newNull() );

		}

	}

	return output;
}

seq::Stream seq_std_meta_build_time( seq::Stream& input ) {
	seq::Stream output;

	for( int i = input.size(); i > 0; i -- ) {

		try{

			output.push_back( seq::util::newNumber( std::stoi( (char*) values.at( "time" ).c_str() ) ) );

		}catch( std::out_of_range& err ) {

			output.push_back( seq::util::newNull() );

		}

	}

	return output;
}

INIT( seq::Executor* exe, seq::FileHeader* head ) {

	if( head == nullptr ) {
		return INIT_ERROR;
	}

	major = head->getVersionMajor();
	minor = head->getVersionMinor();
	patch = head->getVersionPatch();
	values = head->getValueMap();

	exe->inject( "std:meta:major", seq_std_meta_major );
	exe->inject( "std:meta:minor", seq_std_meta_minor );
	exe->inject( "std:meta:patch", seq_std_meta_patch );
	exe->inject( "std:meta:value", seq_std_meta_patch );
	exe->inject( "std:meta:build_time", seq_std_meta_build_time );

	return INIT_SUCCESS;
}

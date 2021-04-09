
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

seq::Stream seq_std_uppercase( seq::Stream& input ) {

	seq::Stream output;

	for( auto& arg : input ) {

		std::string str = (char*) seq::util::stringCast( arg ).String().getString().c_str();
		std::transform(str.begin(), str.end(), str.begin(), toupper);
		output.push_back( seq::util::newString( str.c_str() ) );

	}

	return output;
}

seq::Stream seq_std_lowercase( seq::Stream& input ) {

	seq::Stream output;

	for( auto& arg : input ) {

		std::string str = (char*) seq::util::stringCast( arg ).String().getString().c_str();
		std::transform(str.begin(), str.end(), str.begin(), tolower);
		output.push_back( seq::util::newString( str.c_str() ) );

	}

	return output;
}

seq::Stream seq_std_concat( seq::Stream& input ) {

	std::string str;

	for( auto& arg : input ) {

		str += seq::util::stringCast( arg ).String().getString();

	}

	return { seq::util::newString( str.c_str() ) };
}

seq::Stream seq_std_join( seq::Stream& input ) {

	std::string str;
	std::string separator = seq::util::stringCast( input[0] ).String().getString();

	for( long i = 1; i < (long) input.size(); i ++ ) {

		str += seq::util::stringCast( input[i] ).String().getString();
		str += separator;

	}

	str.erase( str.size() - separator.size() );

	return { seq::util::newString( str.c_str() ) };
}

seq::Stream seq_std_split( seq::Stream& input ) {

	seq::Stream output;
	std::string delim = seq::util::stringCast( input[0] ).String().getString();

	if( delim.empty() ) {
		return input;
	}

	for( long i = 1; i < (long) input.size(); i ++ ) {

		std::string str2 = seq::util::stringCast( input[i] ).String().getString();
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

seq::Stream seq_std_explode( seq::Stream& input ) {

	seq::Stream output;

	for( long i = 0; i < (long) input.size(); i ++ ) {

		std::string str = seq::util::stringCast( input[i] ).String().getString();

		for( long j = 0; j < (long) str.size(); j ++ ) {

			std::string chr = "";
			chr += (seq::byte) str[j];
			output.push_back( seq::util::newString( chr.c_str() ) );

		}

	}

	return output;
}

seq::Stream seq_std_from_code( seq::Stream& input ) {

	seq::Stream output;

	for( long i = 0; i < (long) input.size(); i ++ ) {

		seq::byte code = (seq::byte) seq::util::numberCast( input[i] ).Number().getLong();
		std::string str = "";
		str += (seq::byte) code;
		output.push_back( seq::util::newString( str.c_str() ) );

	}

	return output;
}

seq::Stream seq_std_to_code( seq::Stream& input ) {

	seq::Stream output;

	for( long i = 0; i < (long) input.size(); i ++ ) {

		seq::byte code = seq::util::stringCast( input[i] ).String().getString()[0];
		output.push_back( seq::util::newNumber( code ) );

	}

	return output;
}

seq::Stream seq_std_substr( seq::Stream& input ) {

	if( input.size() < 2 ) {
		return { seq::util::newBool(false) };
	}

	const int start = seq::util::numberCast( input[0] ).Number().getLong();
	const int length = seq::util::numberCast( input[1] ).Number().getLong() - start;

	seq::Stream output;

	for( long i = 2; i < (long) input.size(); i ++ ) {

		try{
			output.push_back( seq::util::newString( seq::util::stringCast( input[i] ).String().getString().substr(start, length).c_str() ) );
		}catch(std::exception& err){
			return { seq::util::newString( "" ) };
		}

	}

	return output;
}

seq::Stream seq_std_findstr( seq::Stream& input ) {

	seq::Stream output;
	std::string delim = seq::util::stringCast( input[0] ).String().getString();

	if( delim.empty() ) {
		return { seq::util::newNumber(-1) };
	}

	for( long i = 1; i < (long) input.size(); i ++ ) {

		std::string str2 = seq::util::stringCast( input[i] ).String().getString();
		output.push_back( seq::util::newNumber( str2.find( delim ) ) );

	}

	return output;
}

INIT( seq::Executor* exe, seq::FileHeader* head ) {

	exe->inject( "std:uppercase", seq_std_uppercase );
	exe->inject( "std:lowercase", seq_std_lowercase );
	exe->inject( "std:concat", seq_std_concat );
	exe->inject( "std:join", seq_std_join );
	exe->inject( "std:split", seq_std_split );
	exe->inject( "std:explode", seq_std_explode );
	exe->inject( "std:from_code", seq_std_from_code );
	exe->inject( "std:to_code", seq_std_to_code );
	exe->inject( "std:substr", seq_std_substr );
	exe->inject( "std:findstr", seq_std_findstr );

	return INIT_SUCCESS;
}


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
#include <fstream>

#ifdef _WIN32
#	include <windows.h>
#	include <direct.h>
#	define CWD_MAX_PATH MAX_PATH
#	define POSIX_GETCWD _getcwd
#	define POSIX_MKDIR(dir) _mkdir( dir )
#else
#	include <unistd.h>
#	include <linux/limits.h>
#	include <sys/stat.h>
#	define CWD_MAX_PATH PATH_MAX
#	define POSIX_GETCWD getcwd
#	define POSIX_MKDIR(dir) mkdir( dir, ACCESSPERMS )
#endif

seq::Stream seq_std_read( seq::Stream& input ) {

	seq::Stream output;

	for( auto& arg : input ) {

		std::string path = (char*) seq::util::stringCast( arg ).String().getString().c_str();
		std::ifstream file( path );
		std::stringstream buffer;
		buffer << file.rdbuf();
		output.push_back( seq::util::newString( (seq::byte*) buffer.str().c_str() ) );
		file.close();

	}

	return output;
}

seq::Stream seq_std_write( seq::Stream& input ) {

	std::string path = (char*) seq::util::stringCast( input[0] ).String().getString().c_str();
	std::ofstream file( path, std::ios::out | std::ios::trunc );

	if( file.good() ) {

		for( int i = 1; i < (int) input.size(); i ++ ) {
			file << (char*) seq::util::stringCast( input[i] ).String().getString().c_str();
		}

		file.close();

		return { seq::util::newBool(true) };
	}

	return { seq::util::newBool(false) };
}

seq::Stream seq_std_append( seq::Stream& input ) {

	std::string path = (char*) seq::util::stringCast( input[0] ).String().getString().c_str();
	std::ofstream file( path, std::ios::out | std::ios::app );

	if( file.good() ) {

		for( int i = 1; i < (int) input.size(); i ++ ) {
			file << (char*) seq::util::stringCast( input[i] ).String().getString().c_str();
		}

		file.close();

		return { seq::util::newBool(true) };
	}

	return { seq::util::newBool(false) };
}

seq::Stream seq_std_mkdir( seq::Stream& input ) {

	seq::Stream output;

	for( auto& arg : input ) {

		std::string path = (char*) seq::util::stringCast( arg ).String().getString().c_str();
		bool status = ( POSIX_MKDIR( path.c_str() ) != -1 );
		output.push_back( seq::util::newBool( status ) );

	}

	return output;
}

seq::Stream seq_std_mkfile( seq::Stream& input ) {

	seq::Stream output;

	for( auto& arg : input ) {

		std::string path = (char*) seq::util::stringCast( arg ).String().getString().c_str();
		std::ofstream file( path );
		output.push_back( seq::util::newBool( file.good() ) );
		file.close();

	}

	return output;
}

seq::Stream seq_std_remove( seq::Stream& input ) {

	seq::Stream output;

	for( auto& arg : input ) {

		std::string path = (char*) seq::util::stringCast( arg ).String().getString().c_str();
		output.push_back( seq::util::newBool( remove( path.c_str() ) != 0 ) );

	}

	return output;
}

seq::Stream seq_std_exists( seq::Stream& input ) {

	seq::Stream output;

	for( auto& arg : input ) {

		std::string path = (char*) seq::util::stringCast( arg ).String().getString().c_str();
		output.push_back( seq::util::newBool( std::ifstream( path ).good() ) );

	}

	return output;
}

seq::Stream seq_std_cwd( seq::Stream& input ) {

	seq::Stream output;

	for( int i = input.size() - 1; i >= 0; i -- ) {

		char tmp[ CWD_MAX_PATH ];
		if( POSIX_GETCWD(tmp, sizeof(tmp)) ) {
			output.push_back( seq::util::newString( (seq::byte*) tmp ) );
		}else{
			output.push_back( seq::util::newNull() );
		}

	}

	return output;
}

seq::Stream seq_std_system_name( seq::Stream& input ) {

	seq::Stream output;

	for( int i = input.size(); i > 0; i -- ) {

		output.push_back( seq::util::newString( (seq::byte*) SQ_SYSTEM ) );

	}

	return output;
}

seq::Stream seq_std_invoke( seq::Stream& input ) {

	seq::Stream output;

	for( auto& arg : input ) {

		int s = system( (char*) seq::util::stringCast( arg ).String().getString().c_str() );
		output.push_back( seq::util::newNumber( s ) );

	}

	return output;
}

INIT( seq::Executor* exe, seq::FileHeader* head ) {

	exe->inject( "std:read"_b, seq_std_read );
	exe->inject( "std:write"_b, seq_std_write );
	exe->inject( "std:append"_b, seq_std_append );
	exe->inject( "std:mkfile"_b, seq_std_mkfile );
	exe->inject( "std:mkdir"_b, seq_std_mkdir );
	exe->inject( "std:remove"_b, seq_std_remove );
	exe->inject( "std:exists"_b, seq_std_exists );
	exe->inject( "std:cwd"_b, seq_std_cwd );
	exe->inject( "std:system_name"_b, seq_std_system_name );
	exe->inject( "std:invoke"_b, seq_std_invoke );

	return INIT_SUCCESS;
}

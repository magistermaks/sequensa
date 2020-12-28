
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
#include "cmath"

#define PI 3.14159265

seq::Stream seq_std_rand( seq::Stream& input ) {
	seq::Stream output;

	for( int i = input.size(); i > 0; i -- ) {

		output.push_back( seq::util::newNumber( rand() ) );

	}

	return output;
}

seq::Stream seq_std_srand( seq::Stream& input ) {

	for( auto& arg : input ) {

		srand( seq::util::numberCast( arg ).Number().getLong() );

	}

	return EMPTY;
}

seq::Stream seq_std_sin( seq::Stream& input ) {
	seq::Stream output;

	for( auto& arg : input ) {

		output.push_back( seq::util::newNumber( std::sin( seq::util::numberCast( arg ).Number().getDouble() ) ) );

	}

	return output;
}

seq::Stream seq_std_cos( seq::Stream& input ) {
	seq::Stream output;

	for( auto& arg : input ) {

		output.push_back( seq::util::newNumber( std::cos( seq::util::numberCast( arg ).Number().getDouble() ) ) );

	}

	return output;
}

seq::Stream seq_std_tan( seq::Stream& input ) {
	seq::Stream output;

	for( auto& arg : input ) {

		output.push_back( seq::util::newNumber( std::tan( seq::util::numberCast( arg ).Number().getDouble() ) ) );

	}

	return output;
}

seq::Stream seq_std_sum( seq::Stream& input ) {
	double sum = 0;

	for( auto& arg : input ) {

		sum += seq::util::numberCast( arg ).Number().getDouble();

	}

	seq::Stream output {
		seq::util::newNumber( sum )
	};

	return output;
}

seq::Stream seq_std_abs( seq::Stream& input ) {
	seq::Stream output;

	for( auto& arg : input ) {

		output.push_back( seq::util::newNumber( std::abs( seq::util::numberCast( arg ).Number().getDouble() ) ) );

	}

	return output;
}

seq::Stream seq_std_min( seq::Stream& input ) {
	const int s = input.size();

	double min = seq::util::numberCast( input.at(0) ).Number().getDouble();

	for( int i = 1; i < s; i ++ ) {

		double tmp = seq::util::numberCast( input.at(i) ).Number().getDouble();
		if( tmp < min ) min = tmp;

	}

	seq::Stream output {
		 seq::util::newNumber( min )
	};

	return output;
}

seq::Stream seq_std_max( seq::Stream& input ) {
	const int s = input.size();

	double max = seq::util::numberCast( input.at(0) ).Number().getDouble();

	for( int i = 1; i < s; i ++ ) {

		double tmp = seq::util::numberCast( input.at(i) ).Number().getDouble();
		if( tmp > max ) max = tmp;

	}

	seq::Stream output {
		seq::util::newNumber( max )
	};

	return output;
}

seq::Stream seq_std_sqrt( seq::Stream& input ) {
	seq::Stream output;

	for( auto& arg : input ) {

		double num = seq::util::numberCast( arg ).Number().getDouble();

		if( num < 0 ) {
			output.push_back( seq::util::newNull() );
		}else{
			output.push_back( seq::util::newNumber( std::sqrt( num ) ) );
		}

	}

	return output;
}

seq::Stream seq_std_round( seq::Stream& input ) {
	seq::Stream output;

	for( auto& arg : input ) {

		output.push_back( seq::util::newNumber( std::round( seq::util::numberCast( arg ).Number().getDouble() ) ) );

	}

	return output;
}

seq::Stream seq_std_floor( seq::Stream& input ) {
	seq::Stream output;

	for( auto& arg : input ) {

		output.push_back( seq::util::newNumber( std::floor( seq::util::numberCast( arg ).Number().getDouble() ) ) );

	}

	return output;
}

seq::Stream seq_std_ceil( seq::Stream& input ) {
	seq::Stream output;

	for( auto& arg : input ) {

		output.push_back( seq::util::newNumber( std::ceil( seq::util::numberCast( arg ).Number().getDouble() ) ) );

	}

	return output;
}

seq::Stream seq_std_deg( seq::Stream& input ) {
	seq::Stream output;

	for( auto& arg : input ) {

		output.push_back( seq::util::newNumber( (PI / 180.0) * seq::util::numberCast( arg ).Number().getDouble() ) );

	}

	return output;
}

INIT( seq::Executor* exe, seq::FileHeader* head ) {

	exe->inject( "std:sin", seq_std_sin );
	exe->inject( "std:cos", seq_std_cos );
	exe->inject( "std:tan", seq_std_tan );
	exe->inject( "std:abs", seq_std_abs );
	exe->inject( "std:sum", seq_std_sum );
	exe->inject( "std:min", seq_std_min );
	exe->inject( "std:max", seq_std_max );
	exe->inject( "std:sqrt", seq_std_sqrt );
	exe->inject( "std:round", seq_std_round );
	exe->inject( "std:floor", seq_std_floor );
	exe->inject( "std:ceil", seq_std_ceil );
	exe->inject( "std:deg", seq_std_deg );
	exe->inject( "std:rand", seq_std_rand );
	exe->inject( "std:srand", seq_std_srand );

	exe->define( "std:pi", seq::Stream {
		seq::util::newNumber( PI )
	} );

	return INIT_SUCCESS;
}

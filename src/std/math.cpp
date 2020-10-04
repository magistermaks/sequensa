
#include "common.hpp"
#include "cmath"

#define PI 3.14159265

seq::Stream seq_std_sin( seq::Stream& input ) {
	seq::Stream output;

	for( auto& arg : input ) {

		output.push_back( seq::Generic( new seq::type::Number( false, std::sin( seq::util::numberCast( arg ).Number().getDouble() ) ) ) );

	}

	return output;
}

seq::Stream seq_std_cos( seq::Stream& input ) {
	seq::Stream output;

	for( auto& arg : input ) {

		output.push_back( seq::Generic( new seq::type::Number( false, std::cos( seq::util::numberCast( arg ).Number().getDouble() ) ) ) );

	}

	return output;
}

seq::Stream seq_std_tan( seq::Stream& input ) {
	seq::Stream output;

	for( auto& arg : input ) {

		output.push_back( seq::Generic( new seq::type::Number( false, std::tan( seq::util::numberCast( arg ).Number().getDouble() ) ) ) );

	}

	return output;
}

seq::Stream seq_std_sum( seq::Stream& input ) {
	double sum = 0;

	for( auto& arg : input ) {

		sum += seq::util::numberCast( arg ).Number().getDouble();

	}

	seq::Stream output {
		 seq::Generic( new seq::type::Number( false, sum ) )
	};

	return output;
}

seq::Stream seq_std_abs( seq::Stream& input ) {
	seq::Stream output;

	for( auto& arg : input ) {

		output.push_back( seq::Generic( new seq::type::Number( false, std::abs( seq::util::numberCast( arg ).Number().getDouble() ) ) ) );

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
		 seq::Generic( new seq::type::Number( false, min ) )
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
		seq::Generic( new seq::type::Number( false, max ) )
	};

	return output;
}

seq::Stream seq_std_sqrt( seq::Stream& input ) {
	seq::Stream output;

	for( auto& arg : input ) {

		double num = seq::util::numberCast( arg ).Number().getDouble();

		if( num < 0 ) {
			output.push_back( seq::Generic( new seq::type::Null( false ) ) );
		}else{
			output.push_back( seq::Generic( new seq::type::Number( false, std::sqrt( num ) ) ) );
		}

	}

	return output;
}

seq::Stream seq_std_round( seq::Stream& input ) {
	seq::Stream output;

	for( auto& arg : input ) {

		output.push_back( seq::Generic( new seq::type::Number( false, std::round( seq::util::numberCast( arg ).Number().getDouble() ) ) ) );

	}

	return output;
}

seq::Stream seq_std_floor( seq::Stream& input ) {
	seq::Stream output;

	for( auto& arg : input ) {

		output.push_back( seq::Generic( new seq::type::Number( false, std::floor( seq::util::numberCast( arg ).Number().getDouble() ) ) ) );

	}

	return output;
}

seq::Stream seq_std_ceil( seq::Stream& input ) {
	seq::Stream output;

	for( auto& arg : input ) {

		output.push_back( seq::Generic( new seq::type::Number( false, std::ceil( seq::util::numberCast( arg ).Number().getDouble() ) ) ) );

	}

	return output;
}

seq::Stream seq_std_deg( seq::Stream& input ) {
	seq::Stream output;

	for( auto& arg : input ) {

		output.push_back( seq::Generic( new seq::type::Number( false, (PI / 180.0) * seq::util::numberCast( arg ).Number().getDouble() ) ) );

	}

	return output;
}

INIT( seq::Executor* exe, seq::FileHeader* head ) {

	exe->inject( "std:sin"_b, seq_std_sin );
	exe->inject( "std:cos"_b, seq_std_cos );
	exe->inject( "std:tan"_b, seq_std_tan );
	exe->inject( "std:abs"_b, seq_std_abs );
	exe->inject( "std:sum"_b, seq_std_sum );
	exe->inject( "std:min"_b, seq_std_min );
	exe->inject( "std:max"_b, seq_std_max );
	exe->inject( "std:sqrt"_b, seq_std_sqrt );
	exe->inject( "std:round"_b, seq_std_round );
	exe->inject( "std:floor"_b, seq_std_floor );
	exe->inject( "std:ceil"_b, seq_std_ceil );
	exe->inject( "std:deg"_b, seq_std_deg );

	return INIT_SUCCESS;
}

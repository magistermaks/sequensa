
#include "common.hpp"

std::map<seq::string, seq::string> values;
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

			output.push_back( seq::util::newNumber( std::stoi( (char*) values.at( "time"_b ).c_str() ) ) );

		}catch( std::out_of_range& err ) {

			output.push_back( seq::util::newNull() );

		}

	}

	return output;
}

seq::Stream seq_std_meta_system( seq::Stream& input ) {
	seq::Stream output;

	for( int i = input.size(); i > 0; i -- ) {

		output.push_back( seq::util::newString( (seq::byte*) SQ_SYSTEM ) );

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

	exe->inject( "std:meta:major"_b, seq_std_meta_major );
	exe->inject( "std:meta:minor"_b, seq_std_meta_minor );
	exe->inject( "std:meta:patch"_b, seq_std_meta_patch );
	exe->inject( "std:meta:value"_b, seq_std_meta_patch );
	exe->inject( "std:meta:build_time"_b, seq_std_meta_build_time );
	exe->inject( "std:meta:system"_b, seq_std_meta_system );

	return INIT_SUCCESS;
}

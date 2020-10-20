
#include "common.hpp"

seq::Stream seq_std_call( seq::Stream& input ) {
	seq::Stream output;

	for( auto& arg : input ) {

		long val = seq::util::numberCast(arg).Number().getLong();

		while( val --> 0 ) {
			output.push_back( seq::util::newNull() );
		}

	}

	return output;
}

seq::Stream seq_std_length( seq::Stream& input ) {
	return { seq::util::newNumber( input.size() ) };
}

INIT( seq::Executor* exe, seq::FileHeader* head ) {

	exe->inject( "std:call"_b, seq_std_call );
	exe->inject( "std:length"_b, seq_std_length );

	const seq::Generic value_true = seq::util::newBool( true );

	exe->define( "std:if"_b, {
			seq::util::newFlowc( {
					new seq::FlowCondition( seq::FlowCondition::Type::Value, value_true, value_true )
			} )
	} );

	return INIT_SUCCESS;
}


#include "common.hpp"

seq::Stream seq_std_call( seq::Stream& input ) {
	seq::Stream output;

	for( auto& arg : input ) {

		long val = seq::util::numberCast(arg).Number().getLong();

		while( val --> 0 ) {
			output.push_back( seq::Generic( new seq::type::Null( false ) ) );
		}

	}

	return output;
}

INIT( seq::Executor* exe, seq::FileHeader* head ) {

	exe->inject( "std:call"_b, seq_std_call );

	const seq::Generic value_true = seq::Generic( new seq::type::Bool( false, true ) );
	const seq::Generic value_false = seq::Generic( new seq::type::Bool( false, false ) );

	exe->define( "std:if"_b, {
			seq::Generic( new seq::type::Flowc( false, {
					new seq::FlowCondition( seq::FlowCondition::Type::Value, value_true, value_true )
			} ) )
	} );

	return INIT_SUCCESS;
}

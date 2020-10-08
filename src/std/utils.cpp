
#include "common.hpp"
#include <chrono>
#include <thread>

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

seq::Stream seq_std_sleep( seq::Stream& input ) {
	seq::Stream output;

	for( auto& arg : input ) {

		std::chrono::milliseconds ms { seq::util::numberCast(arg).Number().getLong() };
		std::this_thread::sleep_for( ms );

	}

	return output;
}

INIT( seq::Executor* exe, seq::FileHeader* head ) {

	exe->inject( "std:call"_b, seq_std_call );
	exe->inject( "std:sleep"_b, seq_std_sleep );

	const seq::Generic value_true = seq::Generic( new seq::type::Bool( false, true ) );
	const seq::Generic value_false = seq::Generic( new seq::type::Bool( false, false ) );

	exe->define( "std:if"_b, {
			seq::Generic( new seq::type::Flowc( false, {
					new seq::FlowCondition( seq::FlowCondition::Type::Value, value_true, value_true )
			} ) )
	} );

	exe->define( "std:ifn"_b, {
			seq::Generic( new seq::type::Flowc( false, {
					new seq::FlowCondition( seq::FlowCondition::Type::Value, value_false, value_false )
			} ) )
	} );

	return INIT_SUCCESS;
}

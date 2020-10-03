
#include "common.hpp"

INIT( seq::Executor* exe, seq::FileHeader* head ) {

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

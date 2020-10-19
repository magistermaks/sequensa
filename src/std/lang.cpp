
#define NO_EXCLUDE_COMPILER
#include "common.hpp"

seq::Stream seq_std_eval( seq::Stream& input ) {
	seq::Stream output;

	for( auto& arg : input ) {

		try{

			seq::string code = seq::util::stringCast(arg).String().getString();
			auto buf = seq::Compiler::compile( code );
			seq::ByteBuffer bb( buf.data(), buf.size() );

			seq::Executor exe;
			exe.execute( bb );

			output.push_back( exe.getResult() );

		}catch( std::exception& error ) {
			output.push_back( seq::util::newNull() );
		}

	}

	return output;
}

INIT( seq::Executor* exe, seq::FileHeader* head ) {

	exe->inject( "std:eval"_b, seq_std_eval );

	return INIT_SUCCESS;
}

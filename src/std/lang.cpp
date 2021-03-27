
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

#define NO_EXCLUDE_COMPILER
#include "common.hpp"

seq::Executor* executor;

seq::Stream seq_std_eval( seq::Stream& input ) {
	seq::Stream output;

	for( auto& arg : input ) {

		try{

			std::string code = seq::util::stringCast(arg).String().getString();
			auto buf = seq::Compiler::compileStatic( code );
			seq::ByteBuffer bb( buf.data(), buf.size() );

			seq::Executor exe;
			exe.execute( bb );

			output.insert(output.end(), exe.getResults().begin(), exe.getResults().end());

		}catch( std::exception& error ) {
			output.push_back( seq::util::newNull() );
		}

	}

	return output;
}

seq::Stream seq_std_mixin( seq::Stream& input ) {
	seq::Stream output;

	for( auto& arg : input ) {

		try{

			std::string code = seq::util::stringCast(arg).String().getString();
			auto buf = seq::Compiler::compileStatic( code );
			seq::ByteBuffer bb( buf.data(), buf.size() );

			seq::Executor exe(executor);
			exe.execute( bb );

			output.insert(output.end(), exe.getResults().begin(), exe.getResults().end());

		}catch( std::exception& error ) {
			output.push_back( seq::util::newNull() );
		}

	}

	return output;
}

INIT( seq::Executor* exe, seq::FileHeader* head ) {

	executor = exe;

	exe->inject( "std:mixin", seq_std_mixin );
	exe->inject( "std:eval", seq_std_eval );

	return INIT_SUCCESS;
}

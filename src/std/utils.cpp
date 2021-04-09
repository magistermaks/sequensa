
/*
 * MIT License
 *
 * Copyright (c) 2020, 2021 magistermaks
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

	exe->inject( "std:call", seq_std_call );
	exe->inject( "std:length", seq_std_length );

	const seq::Generic value_true = seq::util::newBool( true );

	exe->define( "std:if", {
			seq::util::newFlowc( {
					new seq::FlowCondition( seq::FlowCondition::Type::Value, value_true, value_true )
			} )
	} );

	return INIT_SUCCESS;
}

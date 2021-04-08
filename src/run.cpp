
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

#include "api/SeqAPI.hpp"
#include "modules.hpp"

void run( std::string input, Options opt ) {

	std::ifstream infile( input, std::ios::binary );
	if( infile.good() ) {

		std::vector<seq::byte> buffer( (std::istreambuf_iterator<char>(infile) ), (std::istreambuf_iterator<char>() ));
		seq::ByteBuffer bb( buffer.data(), buffer.size() );
		seq::BufferReader br = bb.getReader();
		seq::FileHeader header;

		if( !load_header( &header, br ) ) return;
		if( !validate_version( header, opt.force_execution, opt.verbose ) ) return;

		seq::ByteBuffer bytecode = br.getSubBuffer();

		// load string table if one is present
		seq::StringTable table = header.getValueTable("str");
		if( table.size() > 0 ) {
			bytecode.setStringTable( &table );
		}

		try{

			seq::Executor exe;

			if( !load_native_libs( exe, header, opt.verbose ) ) {
				std::cout << "Failed to create virtual environment, start aborted!" << std::endl;
				return;
			}

			exe.setStrictMath( opt.strict_math );
			exe.execute( bytecode );

			if( !opt.print_none ) {

				std::cout << "Exit value: ";

				if( opt.print_all ) {

					for( auto& res : exe.getResults() ) {

						std::cout << seq::util::stringCast( res ).String().getString() << " ";

					}

					std::cout << std::endl;

				}else{

					std::cout << exe.getResultString() << std::endl;

				}

			}


		}catch( seq::RuntimeError& err ) {

			std::cout << "Runtime error!" << std::endl;
			std::cout << err.what() << (opt.verbose ? " (" + input + ")" : "") << std::endl;
			return;

		}catch( seq::InternalError& err ) {

			std::cout << "Internal error!" << std::endl;
			std::cout << err.what() << (opt.verbose ? " (" + input + ")" : "") << std::endl;
			return;

		}

	}else{

		std::cout << "No such file '" << input << "' found!" << std::endl;
		return;

	}

	unload_native_libs();
	infile.close();

}

void run( ArgParse& argp, Options opt ) {

	auto vars = argp.getArgs("--run", "-r");

	if( vars.size() == 1 ) {

		run( vars.at(0), opt );

	}else{

		std::cout << "Expected one filename!" << std::endl;
		std::cout << "Use '--help' for usage help." << std::endl;

	}

}

#undef TRY_LOADING


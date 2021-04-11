
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

#define ARGPARSE_IMPLEMENT
#include "lib/argparse.hpp"

#include "modules.hpp"

int main( int argc, char **argv ) {

	ArgParse argp( argc, argv );
	short mode = 0;

	if( argp.hasFlag("--help", "-h") ) {
		help(argp);
		return 0;
	}

	mode |= argp.hasFlag("--build", "-b") ? 1 : 0;
	mode |= argp.hasFlag("--run", "-r") ? 2 : 0;
	mode |= argp.hasFlag("--decompile", "-d") ? 4 : 0;
	mode |= argp.hasFlag("--info", "-i") ? 8 : 0;
	mode |= argp.hasFlag("--shell", "-s") ? 16 : 0;

	Options options = {0};
	options.verbose = argp.hasFlag("-v", "--verbose");
	options.force_execution = argp.hasFlag("-f", "--force");
	options.print_exit = argp.hasFlag("-e");
	options.strict_math = argp.hasFlag("-S");
	options.no_multi_error = argp.hasFlag("-xm");
	options.optimize = argp.hasFlag("-o");
	options.no_warn = argp.hasFlag("-xw");

	try{

		switch( mode ) {

			case 1:
				build( argp, options );
				break;

			case 2:
				run( argp, options );
				break;

			case 4:
				decompile( argp, options );
				break;

			case 8:
				info( argp, options );
				break;

			case 16:
				shell( argp, options );
				break;

			default:
				std::cout << "Invalid arguments!" << std::endl;
				std::cout << "Use '--help' for usage help." << std::endl;
				break;

		}

	}catch(...){

		std::cout << "Unknown internal error occurred, process terminated!" << std::endl;

	}

	return 0;

}

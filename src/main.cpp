
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

#include "api/SeqAPI.hpp"

#define ARGPARSE_IMPLEMENT
#include "lib/argparse.hpp"

#include "modes.hpp"

int main( int argc, char **argv ) {

	ArgParse argp( argc, argv );
	short mode = 0;

	mode |= argp.hasFlag("--help") || argp.hasFlag("-h") ? 1 : 0;
	mode |= argp.hasFlag("--build") || argp.hasFlag("-b") ? 2 : 0;
	mode |= argp.hasFlag("--run") || argp.hasFlag("-r") ? 4 : 0;

	Options options = {0};
	options.verbose = argp.hasFlag("-v");
	options.force_execution = argp.hasFlag("-f");
	options.print_all = argp.hasFlag("-a");
	options.print_none = argp.hasFlag("-n");
	options.strict_math = argp.hasFlag("-s");
	options.multi_error = argp.hasFlag("-m");
	options.optimize = argp.hasFlag("-o");

	if( options.print_all && options.print_none ) {

		std::cout << "Unable to combine '-a' and '-n'!" << std::endl;
		return 0;

	}

	try{

		switch( mode ) {

			case 1:
				help( argp, options );
				break;

			case 2:
				build( argp, options );
				break;

			case 4:
				run( argp, options );
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


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

#include "modes.hpp"
#include "api/SeqAPI.hpp"

void help( ArgParse& argp, Options opt ) {

	std::cout << "Options:" << std::endl;
	std::cout << "  Use --help  to see this help page." << std::endl;
	std::cout << "  Use --build to compile Sequensa program." << std::endl;
	std::cout << "  Use --run   to execute compiled Sequensa program." << std::endl;
	std::cout << "  Use -v      to enable verbose mode." << std::endl;
	std::cout << "  Use -f      to force execution regardless of version mismatch." << std::endl;
	std::cout << "  Use -a      to print entire exit stream." << std::endl;
	std::cout << "  Use -n      to not print exit value." << std::endl;
	std::cout << "  Use -s      to enable strict math - throw on invalid operands." << std::endl;
	std::cout << "  Use -m      to enable multi-error mode." << std::endl;
	std::cout << std::endl;
	std::cout << "Example:" << std::endl;
	std::cout << "  sequensa --build ./main.sq ./main.sqc" << std::endl;
	std::cout << "  sequensa --run ./main.sqc" << std::endl;
	std::cout << std::endl;
	std::cout << "Version:" << std::endl;
	std::cout << "  Sequensa API " << '"' << SEQ_API_NAME << "\" " << SEQ_API_VERSION_MAJOR << '.' << SEQ_API_VERSION_MINOR << '.' << SEQ_API_VERSION_PATCH << std::endl;
	std::cout << "  Standard " << SEQ_API_STANDARD << std::endl;
	std::cout << "  Version " << SQ_VER << " (" << SQ_TARGET << ')' << std::endl;

}

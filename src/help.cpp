
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

void help() {

	std::cout << "Usage: sequensa [MODE] [OPTION...]" << std::endl;

	std::cout << std::endl;
	std::cout << "Main modes of operation:" << std::endl;
	std::cout << "  -h, --help       Print this help page and exit." << std::endl;
	std::cout << "  -b, --build      Build specified file." << std::endl;
	std::cout << "  -r, --run        Execute specified file." << std::endl;
	std::cout << "  -d, --decompile  Decompile specified file." << std::endl;
	std::cout << "  -i, --info       Print general information about specified file." << std::endl;
	std::cout << "  -s, --shell      Start Sequensa Shell." << std::endl;

	std::cout << std::endl;
	std::cout << "Options:" << std::endl;
	std::cout << "  -v, --verbose    Enable verbose mode." << std::endl;
	std::cout << "  -f, --force      Force Sequensa to continue." << std::endl;
	std::cout << "  -xm              Disable multi-error mode." << std::endl;
	std::cout << "  -e               Print exit stream." << std::endl;
	std::cout << "  -S               Enable strict math." << std::endl;
	std::cout << "  -o               Enable compiler optimizations." << std::endl;

	std::cout << std::endl;
	std::cout << "Example:" << std::endl;
	std::cout << "  sequensa --build main.sq main.sqc" << std::endl;
	std::cout << "  sequensa --run main.sqc" << std::endl;

	std::cout << std::endl;
	std::cout << "Version:" << std::endl;
	std::cout << "  Version " << '"' << SEQ_API_NAME << "\" " << SEQ_API_VERSION_MAJOR << '.' << SEQ_API_VERSION_MINOR << '.' << SEQ_API_VERSION_PATCH << " (" << SQ_TARGET << ')' << std::endl;
	std::cout << "  Standard " << SEQ_API_STANDARD << std::endl;

}

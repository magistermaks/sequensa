
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

std::string get_loads( seq::FileHeader& header ) {
	std::string str;

	for( std::string& load : header.getValueTable("load") ) {
		str += "load \"";
		str += load;
		str += "\"\n";
	}

	return str + "\n";
}

std::string decompile( std::string input, Options opt ) {

	std::ifstream infile( input, std::ios::binary );
	if( infile.good() ) {

		std::vector<seq::byte> buffer( (std::istreambuf_iterator<char>(infile) ), (std::istreambuf_iterator<char>() ));
		seq::ByteBuffer bb( buffer.data(), buffer.size() );
		seq::BufferReader br = bb.getReader();
		seq::FileHeader header;

		if( !load_header( &header, br ) ) throw 0;
		if( !validate_version( header, opt.force_execution, opt.verbose ) ) throw 0;

		seq::ByteBuffer bytecode = br.getSubBuffer();

		// load string table if one is present
		seq::StringTable table = header.getValueTable("str");
		if( table.size() > 0 ) {
			bytecode.setStringTable( &table );
		}

		seq::SourceDecompiler decompiler;
		seq::BufferReader bytecodeReader = bytecode.getReader();

		time_t rawtime;
		time (&rawtime);

		std::string str;
		str.append( "// Decompiled using Sequensa Source Decompiler \n" );
		str.append( "// " );
		str.append( posix_time_to_date(rawtime) );
		str.append( "\n\n" );
		str.append( get_loads( header ) );
		str.append( decompiler.decompile( bytecodeReader ) );

		infile.close();
		return str;

	}else{
		std::cout << "Failed to open input file!" << std::endl;
	}

	infile.close();
	throw 0;

}

void decompile( ArgParse& argp, Options opt ) {

	auto file = argp.getArgs("--decompile", "-d");

	try{

		if( file.size() == 1 ) {

			std::cout << decompile( file[0], opt );

		}else if( file.size() == 2 ){

			std::ofstream outfile( file[1], std::ios::out );
			if( outfile.good() ) {

				outfile << decompile( file[0], opt );

			}else{
				std::cout << "Failed to open output file!" << std::endl;
			}

		}else{
			USAGE_HELP("Expected one or two filenames!", "decompile");
		}

	}catch(...) {
		// ignore
	}

}


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

void info( std::string input, Options opt ) {

	std::ifstream infile( input, std::ios::binary );
	if( infile.good() ) {

		std::vector<seq::byte> buffer( (std::istreambuf_iterator<char>(infile) ), (std::istreambuf_iterator<char>() ));
		seq::ByteBuffer bb( buffer.data(), buffer.size() );
		seq::BufferReader br = bb.getReader();
		seq::FileHeader header;

		if( !load_header( &header, br ) ) return;

		try{

			seq::StringTable natives = header.getValueTable("load");
			seq::StringTable strings = header.getValueTable("str");
			const time_t rawtime = std::stol( header.getValue("time") );
			const seq::byte major = SEQ_API_VERSION_MAJOR;
			const seq::byte minor = SEQ_API_VERSION_MINOR;

			std::cout << "Compiled for: " << header.getVersionString() << " " << header.getValue("std") << (header.checkVersion(major, minor) ? "" : " (unaligned)") << std::endl;
			std::cout << "Natives: " << seq::util::tableToString(natives, ", ") << std::endl;
			std::cout << "Size: " << buffer.size() << " bytes (without header: " << br.getSubBuffer().size() << " bytes)" << std::endl;
			std::cout << "Build on: " << header.getValue("sys") << ", at: " << posix_time_to_date(rawtime) << std::endl;
			std::cout << "Uses string table: " << (strings.size() != 0 ? "yes" : "no") << std::endl;

		}catch(...){
			std::cout << "Failed to create summary, error occurred!" << std::endl;
			std::cout << "This file may be corrupted!" << std::endl;
		}

	}

}

void info( ArgParse& argp, Options opt ) {

	auto file = argp.getArgs("--info", "-i");

	if( file.size() == 1 ) {

		info( file[0], opt );

	}else{
		USAGE_HELP("Expected one filename!", "info");
	}

}


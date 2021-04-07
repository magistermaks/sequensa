
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

		std::cout << "Expected one filename!" << std::endl;
		std::cout << "Use '--help' for usage help." << std::endl;

	}

}


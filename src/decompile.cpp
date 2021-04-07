
#include "api/SeqAPI.hpp"
#include "modules.hpp"

std::string decompile( std::string input, Options opt ) {

	std::ifstream infile( input, std::ios::binary );
	if( infile.good() ) {

		std::vector<seq::byte> buffer( (std::istreambuf_iterator<char>(infile) ), (std::istreambuf_iterator<char>() ));
		seq::ByteBuffer bb( buffer.data(), buffer.size() );
		seq::BufferReader br = bb.getReader();
		seq::FileHeader header;

		try {
			header = br.getHeader();
		} catch( seq::InternalError& err ) {
			std::cout << "Error! Failed to parse file header, invalid signature!" << std::endl;
			throw 0;
		}

		// validate versions
		if( !header.checkVersion(SEQ_API_VERSION_MAJOR, SEQ_API_VERSION_MINOR) ) {

			std::cout << "Error! Invalid Sequensa Virtual Machine version!" << std::endl;
			std::cout << "Program expected: " << header.getVersionString() << std::endl;

			// force program execution regardless of version mismatch
			if( opt.force_execution ) {
				std::cout << "Execution forced, this may cause errors!" << std::endl;
			}else{
				throw 0;
			}

		}else{

			if( opt.verbose && !header.checkPatch(SEQ_API_VERSION_PATCH) ) {

				std::cout << "Warning! Invalid Sequensa Virtual Machine version!" << std::endl;
				std::cout << "Program expected: " << header.getVersionString() << std::endl;

			}

		}

		seq::ByteBuffer bytecode = br.getSubBuffer();

		// load string table if one is present
		seq::StringTable table = header.getValueTable("str");
		if( table.size() > 0 ) {
			bytecode.setStringTable( &table );
		}

		seq::SourceDecompiler decompiler;
		seq::BufferReader bytecodeReader = bytecode.getReader();

		time_t rawtime;
		struct tm ts;
		char buf[255];

		time (&rawtime);
		ts = *gmtime(&rawtime);
		strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S %Z", &ts);

		std::string str;
		str.append("// Decompiled using Sequensa Source Decompiler \n");
		str.append("// ");
		str.append(buf);
		str.append("\n\n");
		str.append(decompiler.decompile( bytecodeReader ));

		return std::move(str);

	}else{
		std::cout << "Failed to open input file!" << std::endl;
	}

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

			std::cout << "Expected one or two filenames!" << std::endl;
			std::cout << "Use '--help' for usage help." << std::endl;

		}
	}catch(...) {
		// ignore
	}

}

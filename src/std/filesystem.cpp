
#include "common.hpp"
#include <fstream>

#ifdef _WIN32
#	include <windows.h>
#	include <direct.h>
#	define CWD_MAX_PATH MAX_PATH
#	define POSIX_GETCWD _getcwd
#	define POSIX_MKDIR(dir) _mkdir( dir )
#else
#	include <unistd.h>
#	include <linux/limits.h>
#	include <sys/stat.h>
#	define CWD_MAX_PATH PATH_MAX
#	define POSIX_GETCWD getcwd
#	define POSIX_MKDIR(dir) mkdir( dir, ACCESSPERMS )
#endif

//class FileHandle: public seq::type::Blob {
//
//	public:
//		FileHandle( bool _anchor, std::fstream file ): seq::type::Blob( _anchor ) {
//			this->file = file;
//		}
//
//		seq::type::Blob* copy() {
//			return new FileHandle( this->anchor, this->file );
//		}
//
//		seq::string toString() {
//			return "file_handle"_b;
//		}
//
//		std::fstream file;
//
//};
//
//seq::Stream seq_std_fs_open( seq::Stream& input ) {
//
//	seq::Stream output;
//
//	for( auto& arg : input ) {
//		std::string path = reinterpret_cast<std::string>(seq::util::stringCast( arg ).String().getString());
//		output.push_back( seq::Generic( new FileHandle( false, std::fstream(path) ) ) );
//	}
//
//	return output;
//}
//
//seq::Stream seq_std_fs_open_binary( seq::Stream& input ) {
//
//	seq::Stream output;
//
//	for( auto& arg : input ) {
//		std::string path = reinterpret_cast<std::string>(seq::util::stringCast( arg ).String().getString());
//		output.push_back( seq::Generic( new FileHandle( false, std::fstream(path, std::ios::binary) ) ) );
//	}
//
//	return output;
//}
//
//seq::Stream seq_std_fs_is_good( seq::Stream& input ) {
//
//	seq::Stream output;
//
//	for( auto& arg : input ) {
//		if( arg.getDataType() == seq::DataType::Blob && arg.Blob().toString() == "file_handle"_b ) {
//			output.push_back( seq::util::newBool( ((FileHandle&) arg.Blob()).file.is_open() ) );
//		}
//
//		output.push_back( seq::util::newNull() );
//	}
//
//	return output;
//}
//
//seq::Stream seq_std_fs_close( seq::Stream& input ) {
//
//	for( auto& arg : input ) {
//		if( arg.getDataType() == seq::DataType::Blob && arg.Blob().toString() == "file_handle"_b ) {
//			((FileHandle&) arg.Blob()).file.close();
//		}
//	}
//
//	return EMPTY;
//}
//
//seq::Stream seq_std_fs_read_all( seq::Stream& input ) {
//
//	seq::Stream output;
//
//	for( auto& arg : input ) {
//		if( arg.getDataType() == seq::DataType::Blob && arg.Blob().toString() == "file_handle"_b ) {
//
//			std::stringstream buffer;
//			buffer << ((FileHandle&) arg.Blob()).file.rdbuf();
//			std::string str(buffer.str());
//			output.push_back( seq::util::newString( (seq::byte*) str.c_str() ) );
//
//		}
//
//		output.push_back( seq::util::newNull() );
//	}
//
//	return output;
//}
//
//seq::Stream seq_std_fs_read_line( seq::Stream& input ) {
//	return EMPTY;
//}
//
//seq::Stream seq_std_fs_write( seq::Stream& input ) {
//	return EMPTY;
//}
//
//seq::Stream seq_std_fs_append( seq::Stream& input ) {
//	return EMPTY;
//}

seq::Stream seq_std_mkdir( seq::Stream& input ) {

	seq::Stream output;

	for( auto& arg : input ) {

		std::string path = (char*) seq::util::stringCast( arg ).String().getString().c_str();
		bool status = ( POSIX_MKDIR( path.c_str() ) != -1 );
		output.push_back( seq::util::newBool( status ) );

	}

	return output;
}

seq::Stream seq_std_mkfile( seq::Stream& input ) {

	seq::Stream output;

	for( auto& arg : input ) {

		std::string path = (char*) seq::util::stringCast( arg ).String().getString().c_str();
		std::ofstream file { path };
		output.push_back( seq::util::newBool( file.good() ) );
		file.close();

	}

	return output;
}

seq::Stream seq_std_remove( seq::Stream& input ) {

	seq::Stream output;

	for( auto& arg : input ) {

		std::string path = (char*) seq::util::stringCast( arg ).String().getString().c_str();
		output.push_back( seq::util::newBool( remove( path.c_str() ) != 0 ) );

	}

	return output;
}

seq::Stream seq_std_exists( seq::Stream& input ) {

	seq::Stream output;

	for( auto& arg : input ) {

		std::string path = (char*) seq::util::stringCast( arg ).String().getString().c_str();
		output.push_back( seq::util::newBool( std::ifstream( path ).good() ) );

	}

	return output;
}

seq::Stream seq_std_cwd( seq::Stream& input ) {

	seq::Stream output;

	for( int i = input.size() - 1; i >= 0; i -- ) {

		char tmp[ CWD_MAX_PATH ];
		if( POSIX_GETCWD(tmp, sizeof(tmp)) ) {
			output.push_back( seq::util::newString( (seq::byte*) tmp ) );
		}else{
			output.push_back( seq::util::newNull() );
		}

	}

	return output;
}

INIT( seq::Executor* exe, seq::FileHeader* head ) {

	//exe->inject( "std:fopen"_b, seq_std_fs_open );
	//exe->inject( "std:fopen_binary"_b, seq_std_fs_open );
	//exe->inject( "std:is_good"_b, seq_std_fs_is_good );
	//exe->inject( "std:close"_b, seq_std_fs_close );
	//exe->inject( "std:read_all"_b, seq_std_fs_read_all );
	//exe->inject( "std:read_line"_b, seq_std_fs_read_line );
	//exe->inject( "std:write"_b, seq_std_fs_write );
	//exe->inject( "std:append"_b, seq_std_fs_append );
	exe->inject( "std:mkfile"_b, seq_std_mkfile );
	exe->inject( "std:mkdir"_b, seq_std_mkdir );
	exe->inject( "std:remove"_b, seq_std_remove );
	exe->inject( "std:exists"_b, seq_std_exists );
	exe->inject( "std:cwd"_b, seq_std_cwd );

	return INIT_SUCCESS;
}

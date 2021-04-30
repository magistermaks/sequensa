
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

/*
 * Sequensa Dynamic C API:
 *
 * 		Sequensa Dynamic C API is a layer of abstraction build on top of Sequensa API that allows Sequensa to be
 * 		used in Languages compatible with Dynamic Libraries such as C, Java, Python, and C#.
 * 		For the Sequensa API itself see: SeqAPI.hpp
 *
 * Known issues:
 *
 * 		- No String tables support
 * 		- No buffer writer/reader interface
 * 		- No file header interface
 * 		- Limited error handling
 * 		- Limited generic types
 *
 */

#include "SeqAPI.hpp"

#ifdef _WIN32
#	define FUNC extern "C" __declspec(dllexport)
#else
#	define FUNC extern "C"
#endif

using Native = seq::type::Native;
using ErrorHandle = bool (*) (void*);

/// Dummy function
FUNC int seq_verify() {
	return 0x534551;
}

/// Null callback
FUNC bool seq_null_error_handle( void* error ) {
	return false;
}

/// Generic free()
FUNC void seq_free( void* ptr ) {
	free(ptr);
}

/// Get major version component
FUNC int seq_version_major() {
	return SEQ_API_VERSION_MAJOR;
}

/// Get minor version component
FUNC int seq_version_minor() {
	return SEQ_API_VERSION_MINOR;
}

/// Get path version component
FUNC int seq_version_patch() {
	return SEQ_API_VERSION_PATCH;
}

/// Get standard name
FUNC const char* seq_standard() {
	return SEQ_API_STANDARD;
}

/// Free Compiler object allocated using seq_compiler_new
FUNC void seq_compiler_free( void* ptr ) {
	delete (seq::Compiler*) ptr;
}

/// Create new Compiler object
FUNC void* seq_compiler_new() {
	return new seq::Compiler();
}

/// Free program buffer
FUNC void seq_compiler_build_free( void* ptr ) {
	free(ptr);
}

/// Create and populate new program buffer
FUNC void* seq_compiler_build_new( void* compiler, const char* str, int* size ) {
	try{
		auto data = ((seq::Compiler*) compiler)->compile( std::string(str) );
		*size = data.size();
		seq::byte* buffer = (seq::byte*) malloc(*size);
		memcpy(buffer, data.data(), *size);
		return buffer;
	}catch(...) {
		*size = 0;
		return nullptr;
	}
}

/// create new buffer, it needs to be later freed
FUNC void* seq_buffer_new( void* data, int size ) {
	seq::byte* buffer = (seq::byte*) malloc(size);
	memcpy(buffer, data, size);
	return buffer;
}

/// Set compiler optimization flags
FUNC void seq_compiler_optimizations( void* compiler, int flags ) {
	((seq::Compiler*) compiler)->setOptimizationFlags(flags);
}

/// Set Compiler error handle
FUNC void seq_compiler_error_handle( void* compiler, ErrorHandle func ) {
	((seq::Compiler*) compiler)->setErrorHandle((seq::Compiler::ErrorHandle) func);
}

/// Query error message from any exception
FUNC const char* seq_exception_message( void* exception ) {
	return ((std::exception*) exception)->what();
}

/// Query error level from compiler exception
FUNC int seq_compiler_error_level( void* exception ) {
	return ((seq::CompilerError*) exception)->getLevel();
}

/// Decompile given bytecode buffer
FUNC void* seq_decompile( void* buffer, int size, const char* indentation, int base ) {
	try{
		seq::ByteBuffer bb( (seq::byte*) buffer, size );
		seq::SourceDecompiler decompiler;
		seq::BufferReader br = bb.getReader();

		decompiler.setIndentation(indentation, base);
		std::string code = decompiler.decompile( br );

		char* str = (char*) malloc( code.size() + 1 );
		strcpy( str, code.c_str() );
		return (void*) str;
	}catch(...) {
		return nullptr;
	}
}

/// Free Executor object allocated using seq_executor_new
FUNC void seq_executor_free( void* ptr ) {
	delete (seq::Executor*) ptr;
}

/// Create new Executor object
FUNC void* seq_executor_new() {
	return new seq::Executor();
}

/// Set Executor's strict math flag
FUNC void seq_executor_strict_math( void* executor, bool flag ) {
	((seq::Executor*) executor)->setStrictMath(flag);
}

/// Execute given program
FUNC void seq_executor_execute( void* executor, void* buffer, int size, ErrorHandle func ) {
	try{
		seq::ByteBuffer bb( (seq::byte*) buffer, size );
		((seq::Executor*) executor)->execute( bb );
	}catch(std::exception& err) {
		func((void*) &err);
	}
}

/// Get pointer to the results stream
FUNC void* seq_executor_results_stream_ptr( void* executor ) {
	return (void*) &(((seq::Executor*) executor)->getResults());
}

/// Add native function to executor
FUNC void seq_executor_add_native( void* executor, const char* name, Native func ) {
	((seq::Executor*) executor)->inject(std::string(name), func);
}

/// Get stream size
FUNC int seq_stream_size( void* stream ) {
	return ((seq::Stream*) stream)->size();
}

/// Get a new stream object, for use in native functions
FUNC void* seq_stream_create() {
	return new seq::Stream();
}

/// Free Stream object allocated using seq_stream_create
FUNC void seq_stream_free( void* stream ) {
	delete (seq::Stream*) stream;
}

/// Get generic from stream
FUNC void* seq_stream_generic_ptr( void* stream, int index ) {
	return (void*) (*((seq::Stream*) stream))[index].getRaw();
}

/// Clear the stream
FUNC void seq_stream_clear( void* stream ) {
	((seq::Stream*) stream)->clear();
}

/// Append to stream
FUNC void seq_stream_add( void* stream, void* generic ) {
	((seq::Stream*) stream)->push_back( seq::Generic( (seq::type::Generic*) generic ) );
}

/// Get data type from generic
FUNC int seq_generic_type( void* generic ) {
	return (int) ((seq::type::Generic*) generic)->getDataType();
}

/// Get anchor from generic
FUNC int seq_generic_anchor( void* generic ) {
	return (int) ((seq::type::Generic*) generic)->getAnchor();
}

/// Query long from number generic
FUNC long seq_generic_number_long( void* generic ) {
	return ((seq::type::Number*) generic)->getLong();
}

/// Query double from number generic
FUNC double seq_generic_number_double( void* generic ) {
	return ((seq::type::Number*) generic)->getDouble();
}

/// Query string from string generic
FUNC const char* seq_generic_string_string( void* generic ) {
	return ((seq::type::String*) generic)->getString().c_str();
}

/// Query bool from string generic
FUNC bool seq_generic_bool_bool( void* generic ) {
	return ((seq::type::Bool*) generic)->getBool();
}

/// Create new generic number object
FUNC void* seq_generic_number_create( bool anchor, double value ) {
	return (void*) new seq::type::Number( anchor, value );
}

/// Create new generic bool object
FUNC void* seq_generic_bool_create( bool anchor, bool value ) {
	return (void*) new seq::type::Bool( anchor, value );
}

/// Create new generic string object
FUNC void* seq_generic_string_create( bool anchor, const char* value ) {
	return (void*) new seq::type::String( anchor, value );
}

/// Create new generic null object
FUNC void* seq_generic_null_create( bool anchor ) {
	return (void*) new seq::type::Null( anchor );
}




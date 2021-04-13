
#include <iostream>

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
 * Copyright Appendix:
 * The following Software is licensed under the MIT license written above,
 * but the Sequensa Language with all its specification is copyrighted by darktree.net
 * Thus you may only redistribute the Software if your copy adheres to the Sequensa specification.
 */

/*
 * General API overview:
 *
 * 		For more informations about Sequensa Language itself see:
 * 		http://darktree.net/projects/sequensa/
 *
 * 		Sections:
 * 			1. Compiling and executing
 * 			2. Execution arguments
 * 			3. Injecting (and removing) functions (and variables) to/from Sequensa
 * 			4. Obtaining results
 * 			5. Using Sequensa streams and data types
 * 			6. Exceptions and their meaning
 * 			7. Compiler error handle
 * 			8. Optimization
 * 			9. Decompiler
 * 			10. Preprocessor
 *
 * 1. Compiling and executing
 *
 *		seq::Compiler::compileStatic( std::string code, seq::StringTable* headerData = nullptr, oflag_t oflag = 0 )
 *		The seq::Compiler::compileStatic method can be used to generate executable Sequesa binary buffer.
 *		If provided with the `headerData` pointer it will store an array of found dependences names in it.
 * 		The returned buffer is of type `std::vector<seq::byte>`
 *
 * 		To execute returned buffer it first must be encased in ByteBuffer object, the first expected constructor
 * 		argument is the pointer to the buffer, and the second one is the buffer size:
 * 		seq::ByteBuffer bb( buf.data(), buf.size() )
 * 		where `buf` is the std::vector returned by seq::Compiler::compileStatic.
 *
 * 		Then the ByteBuffer can be passed to the seq::Executor's execute method.
 * 		Full example:
 *
 * 			// Compile the code and save bytecode in the ByteBuffer
 * 			auto buf = seq::Compiler::compileStatic( code_to_excecute );
 * 			seq::ByteBuffer bb( buf.data(), buf.size() );
 *
 * 			// Execute the bytecode
 * 			seq::Executor exe;
 * 			exe.execute( bb );
 *
 * 		To gain access to more advanced compiler settings a `Compiler` object must be created first,
 * 		then specific options can be set using the provided methods. The bytecode can be obtained
 * 		from this compiler using the `compile( std::string code )` method.
 *
 * 			// Create the Compiler object
 * 			seq::Compiler compiler;
 *
 * 			// Extended compiler API
 * 			compiler.setLoadTable( ... ); // Optional: an replacement for the second argument of `compileStatic()`.
 * 			compiler.setNameTable( ... ); // Optional: sets the name table, more on that in section 8.
 * 			compiler.setErrorHandle( ... ); // Optional: sets error handle, more on that in section 7.
 * 			compiler.setOptimizationFlags( ... ); // Optional: sets optimizations flags, more on that in section 8.
 *
 * 			// Compile the program using the created compiler
 * 			auto buf = compiler.compile( code_to_excecute );
 * 			seq::ByteBuffer bb( buf.data(), buf.size() );
 *
 * 			// Execute the bytecode
 * 			seq::Executor exe;
 * 			exe.execute( bb );
 *
 * 2. Execution arguments
 *
 *		The seq::Excecutor's execute method can also accept additional argument 'args'
 *		which contains the stream of top-level arguments (using more than one argument is
 *		unadvised, as it will cause the whole Sequensa program to execute multiple times - ones per argument)
 *		The default value is an one-element stream containing the value 'null'
 *
 *			// Create args stream containing the number '2'
 *			seq::Stream args = {
 *				seq::util::newNumber( 2 )
 * 			};
 *
 * 			// Execute the bytecode with given args
 * 			seq::Executor exe;
 * 			exe.execute( bb, args );
 *
 * 3. Injecting (and removing) functions (and variables) to/from Sequensa
 *
 * 		By default nothing outside of the Sequnsa program can be manipulated by SVM
 * 		(Sequensa Virtual Machine) to allow function to be invoked from inside of the
 * 		Sequensa program it must first be injected into the executor using the 'inject' method.
 *
 * 		`inject` method takes two parameters: name of the function inside of the Sequensa and the
 * 		function pointer (seq::type::Native). the given function must take seq::Stream as an argument and return
 * 		seq::Stream.
 *
 * 			exe.inject( "myfunc", [] (seq::Stream args) -> seq::Stream {
 * 				// ...
 * 			} );
 *
 * 		`define` method takes two parameters: name of the variable and stream to be stored as the variable's value.
 *
 * 			exe.define( "myval", {
 * 				seq::util::newNull()
 * 			} );
 *
 * 		All injected function can be removed from the Executor object by
 * 		calling `executor.reset()` (this does NOT remove defined variables)
 *
 * 4. Obtaining results
 *
 * 		After successful execution (exe.execute) of the program the returned value(s) can be
 * 		obtained by calling `exe.getResult()` for single result or `exe.getResults()` for stream
 * 		of returned values. Additional helper method `exe.getResultString()` can be used to obtain result
 * 		directly as C++ string.
 *
 * 		Note: `exe.getResult()` returns first element of the result stream (according to
 * 		Sequensa Language Specification this is the program's exit code) if the exit stream is
 * 		empty this method will return a stream with a single null value.
 *
 * 5. Using Sequensa streams and data types
 *
 * 		seq::Stream is a std::vector of seq::Generic's used to represent Sequensa streams (for both input and output)
 * 		seq::Generic is an object used to represent Sequensa's data types and provide a simple interface to manipulate them.
 *
 * 			seq::Generic val = exe.getResult();
 *
 * 			// use this to get the Generic's type
 * 			seq::DataType type = val.getDataType();
 *
 * 			// If you know the type of the value you can access it,
 * 			// Never directly access generics without checking the type - that would cause an
 * 			// undefined behavior and most probably crash your program.
 * 			if( type == seq::DataType::Number ) {
 *
 * 				std::cout << val.Number().getLong();
 *
 * 			}
 *
 * 6. Exceptions and their meaning
 *
 * 		Sequnesa API can generate 3 types of exceptions:
 *
 * 		seq::CompilerError
 * 			Error generated only by the seq::Compile set of functions,
 * 			it signals that the compilation was unsuccessful and provides a standard error message.
 * 			If you are interested in seeing more than one error at a time see section 7.
 *
 * 		seq::RuntimeError
 * 			Generated by executor.execute method (and a set of private method located in the Executor class),
 * 			it signals that the program execution was unsuccessful, return value of `exe.getResultString()`, `exe.getResults()` and
 * 			`exe.getResult()` are undefined after this exception.
 *
 * 		seq::InternalError
 * 			Can be caused by many Sequensa API methods. It can signal one of the following problems:
 * 				1. Invalid Sequensa bytecode (file corruption etc)
 * 				2. Programmer error - either on the side of the API itself or the user.
 * 				3. Some other completely unexpected thing.
 * 			It is advised to report those exceptions as bugs.
 *
 * 7. Compiler error handle
 *
 *		Error handle its a function invoked when compiler encounters a error, to change the default one
 *		(which simply throws the given error) you need to create the `Compiler` object and set it using the
 *		`setErrorHandle` method.
 *
 *			seq::Compiler compiler;
 *
 *			compiler.setErrorHandle( [] (seq::CompilerError err) {
 *
 *				// your code to handle the exception, e.g. print it to console
 *
 *				// if the error is critical it MUST be thrown
 *				if( err.isCritical() ) {
 *					throw err;
 *				}
 *
 *			} );
 *
 *		The result of the compilation is invalid and undefined after the error handler is called,
 *		and should be discarded.
 *
 *		The default error handler can be restored using:
 *			compiler.setErrorHandle( seq::Compiler::defaultErrorHandle );
 *
 * 8. Optimization
 *
 * 		The Sequensa compiler will not try to optimize the bytecode by default, it needs to be
 * 		explicitly told do do so using the compiler optimization flags. they can be supplied as a 3th
 * 		argument to the `compileStatic` function or set using `setOptimizationFlags` on the compiler object.
 *
 * 		The following flags are available; they can be combined using the | operator.
 *
 * 			Optimizations::None (default value)
 * 				Disables all forms of compile-time optimization,
 * 				this can result in slightly shorter compilation time
 *
 * 			Optimizations::All
 * 				Enables all forms of compile-time optimization,
 * 				this may not always be the desired option
 *
 * 			Optimizations::Name
 * 				Enables string table, requires NameTable to
 * 				be supplied to the compiler, or it will be ignored
 *
 * 			Optimizations::PureExpr
 * 				Optimizes expressions that only use pure values
 * 				e.g. (2 / 3), (3.1415 * (3 - 1))
 *
 * 			Optimizations::StrPreGen
 * 				Tries to pregenerate sorted name table, to possibly optimize index values,
 * 				and utilize the tiny storage (first 16 names) to it's fullest extent.
 *
 * 		`Name` optimization requires the name table to be supplied:
 *
 * 			compiler.setNameTable( &stringTable );
 *
 * 		All strings will be then written to that table instead of the bytecode, this can reduce
 * 		it's size and loading time. Warning: note that the name table MUST be then also supplied to the
 * 		executor, using `executor.setNameTable` method!
 *
 * 		The Name optimization is unavailable for Compiler::compileStatic
 *
 * 9. Decompiler
 *
 * 		Sequensa API provides a simple to use decompiler class that allows to convert Sequensa Bytecode
 * 		back to it's source code form. Note that this is not perfect as the source is extrapolated from the bytecode,
 * 		but the produced source should compile to identical bytecode.
 *
 * 		Usage:
 *
 * 			// create new decompiler object
 * 			seq::SourceDecompiler decomp;
 *
 * 			// Indent the generated code with two tabs (default value: single tab)
 * 			decomp.setIndentation( "\t\t" );
 *
 * 			// decompile bytecode from buffer reader and print it
 * 			std::cout << decomp.decompile( bufferReader );
 *
 * 		If you want to create your own Sequensa decompiler (or transcriber) extend the `seq::Decompiler` class,
 * 		for reference look into the `seq::SourceDecompiler` class.
 *
 * 10. Preprocessor
 *
 * 		All metadata provided by the API can be found in the SEQ_API_* macros:
 *
 *			SEQ_API_NAME - string - name of the API
 * 			SEQ_API_STANDARD - string - version of the Sequensa Standard implemented by the API
 * 			SEQ_API_VERSION_MAJOR - byte - Major component of the version number
 * 			SEQ_API_VERSION_MINOR - byte - Minor component of the version number
 * 			SEQ_API_VERSION_PATCH - byte - Patch component of the version number
 *
 *		Following preprocessor statements can be used to changed some aspects of the API at compile time.
 *		Warning: They must be placed BEFORE the inclusion of the API.
 *
 *			#define SEQ_IMPLEMENT - To implement the Sequensa API
 * 			#define SEQ_EXCLUDE_COMPILER - To exclude compiler code from the API
 * 			#define SEQ_EXCLUDE_DECOMPILER - To exclude decompiler code from the API
 */

#pragma once

#include <exception>
#include <string>
#include <cmath>
#include <vector>
#include <unordered_map>
#include <map>
#include <regex>
#include <cfloat>
#include <cstdlib>

// public metadata
#define SEQ_API_NAME "SeqAPI"
#define SEQ_API_STANDARD "2021-04-08"
#define SEQ_API_VERSION_MAJOR 2
#define SEQ_API_VERSION_MINOR 0
#define SEQ_API_VERSION_PATCH 0

// enum ranges
#define SEQ_MIN_OPCODE 1
#define SEQ_MAX_OPCODE 17
#define SEQ_MIN_DATA_TYPE 1
#define SEQ_MAX_DATA_TYPE 13
#define SEQ_MIN_CALL_TYPE 1
#define SEQ_MAX_CALL_TYPE 5
#define SEQ_MIN_OPERATOR 1
#define SEQ_MAX_OPERATOR 21

// tags
#define SEQ_TAG_FIRST 1
#define SEQ_TAG_LAST 2
#define SEQ_TAG_END 4

namespace seq {

	/// define "byte" (unsigned char)
	typedef unsigned char byte;

	// define StringTable, used for passing string arrays
	typedef std::vector<std::string> StringTable;

	/// forward definition of all sequensa API classes
	class FileHeader;
	class ByteBuffer;
	class BufferReader;
	class TokenReader;
	class InternalError;
	class CompilerError;
	class RuntimeError;
	class Executor;
	class FlowCondition;

	/// Opcodes - operation identifiers
	enum struct Opcode: byte {
		BLT = 1,  // BLT ;
		BLF = 2,  // BLF ;
		NIL = 3,  // NIL ;
		NUM = 4,  // NUM [HEAD] [TAIL...] ;
		INT = 5,  // INT [BYTE] ;
		STR = 6,  // STR [UTF8...] ;
		TYP = 7,  // TYP [TYPE] ;
		VMC = 8,  // VMC [TYPE] ;
		ARG = 9,  // ARG [SIZE] ;
		FUN = 10, // FUN [HEAD] [TAIL...] [BODY...] ;
		EXP = 11, // EXP [TYPE] [HEAD] [TAIL...] [L...] [R...] ;
		VAR = 12, // VAR [ASCI...] ;
		DEF = 13, // DEF [ASCI...] ;
		FLC = 14, // FLC [SIZE] [[SIZE] [BODY...]...] ;
		SSL = 15, // SSL [TAGS] [HEAD] [TAIL...] [BODY...] ;
		FNE = 16, // FNE [HEAD] [TAIL...] [BODY...] ;
		TEX = 17  // TEX [TYPE] [BYTE] [L...] [R...] ;
	};

	/// Sequensa data types
	enum struct DataType: byte {
		Bool   = 1, // maps to: BLT BLF
		Null   = 2, // maps to: NIL
		Number = 3, // maps to: NUM INT
		String = 4, // maps to: STR
		Type   = 5, // maps to: TYP
		VMCall = 6, // maps to: VMC
		Arg    = 7, // maps to: ARG
		Func   = 8, // maps to: FUN FNE
		Expr   = 9, // maps to: EXP TEX
		Name  = 10, // maps to: VAR DEF
		Flowc = 11, // maps to: FLC
		Stream = 12,// maps to: SSL
		Blob  = 13  // maps to: ---
	};

	enum struct ExprOperator: byte {
		Less           = 1, // <
		Greater        = 2, // >
		Equal          = 3, // =
		NotEqual       = 4, // !=
		NotGreater     = 5, // !>, <=
		NotLess        = 6, // !<, >=
		And            = 7, // &&
		Or             = 8, // ||
		Xor            = 9, // ^^
		Not            = 10, // !
		Multiplication = 11, // *
		Division       = 12, // /
		Addition       = 13, // +
		Subtraction    = 14, // -
		Modulo         = 15, // %
		Power          = 16, // **
		BinaryAnd      = 17, // &
		BinaryOr       = 18, // |
		BinaryXor      = 19, // ^
		BinaryNot      = 20, // ~
		Accessor       = 21  // ::
	};

	/// Simple fraction struct, used by seq::type::Number
	struct Fraction {
		const long numerator;
		const long denominator;
	};

	namespace type {

		class Null;
		class Stream;
		class Flowc;
		class Expression;
		class Function;
		class Name;
		class VMCall;
		class Type;
		class String;
		class Number;
		class Arg;
		class Bool;
		class Generic;

		class Generic {

			private:
				const DataType type;

			protected:
				Generic( const DataType type, bool anchor );
				bool anchor;

			public:
				virtual ~Generic() {}
				const DataType getDataType() const noexcept;
				const bool getAnchor() const noexcept;
				void setAnchor( bool anchor ) noexcept;
		};

		class Bool: public Generic {

			public:
				Bool( bool anchor, bool value );
				const bool getBool();

			private:
				const bool value;
		};

		class Number: public Generic {

			public:
				Number( bool anchor, long numerator, long denominator );
				Number( bool anchor, double value );
				const double getDouble();
				const long getLong();
				const bool isNatural();
				const Fraction getFraction();
				static byte sizeOfSigned( unsigned long value );
				static byte sizeOf( unsigned long value );

			private:
				const double value;
		};

		class Arg: public Generic {

			public:
				Arg( bool anchor, byte level );
				const byte getLevel();

			private:
				const byte level;
		};

		class String: public Generic {

			public:
				String( bool anchor, const char* value );
				std::string& getString();

			private:
				std::string value;
		};

		class Type: public Generic {

			public:
				Type( bool anchor, DataType value );
				const DataType getType();

			public:
				const DataType value;
		};

		class VMCall: public Generic {

			public:
				enum struct CallType: byte {
					Return = 1,
					Break = 2,
					Exit = 3,
					Again = 4,
					Final = 5
				};

				VMCall( bool anchor, CallType value );
				const CallType getCall();

			private:
				const CallType value;

		};

		class Name: public Generic {

			public:
				Name( bool anchor, bool define, std::string name );
				std::string& getName();
				const bool getDefine();

			private:
				const bool define;
				std::string name;
		};

		class Function: public Generic {

			public:
				Function( bool anchor, BufferReader* reader, bool end );
				Function( const Function& func );
				~Function();
				BufferReader& getReader();
				const bool hasEnd() noexcept;

			private:
				BufferReader* reader;
				const bool end;
		};

		class Expression: public Generic {

			public:
				Expression( bool anchor, ExprOperator op, BufferReader* left, BufferReader* right );
				Expression( const Expression& expr );
				~Expression();
				const ExprOperator getOperator();
				BufferReader& getLeftReader();
				BufferReader& getRightReader();

			private:
				const ExprOperator op;
				BufferReader* left;
				BufferReader* right;
		};

		class Flowc: public Generic {

			public:
				Flowc( bool anchor, const std::vector<FlowCondition*> readers );
				Flowc( const Flowc& flowc );
				~Flowc();
				const std::vector<FlowCondition*>& getConditions();

			private:
				const std::vector<FlowCondition*> conditions;
		};

		class Stream: public Generic {

			public:
				Stream( bool anchor, byte tags, BufferReader* reader );
				Stream( const Stream& stream );
				~Stream();
				const bool matchesTags( byte tags );
				const byte getTags();
				BufferReader& getReader();

			private:
				const byte tags;
				BufferReader* reader;
		};

		class Null: public Generic {

			public:
				Null( bool anchor );

		};

		class Blob: public Generic {

			public:
				Blob( bool anchor );
				virtual std::string toString();
				virtual Blob* copy();

		};

	}

	class Generic {

		public:
			Generic();
			Generic( type::Generic* generic );
			Generic( const seq::Generic& generic );
			Generic( seq::Generic&& generic );
			~Generic();

			Generic& operator= ( const Generic& generic );
			Generic& operator= ( Generic&& generic ) noexcept;

			const DataType getDataType() const noexcept;
			bool getAnchor() const noexcept;
			void setAnchor( bool anchor ) noexcept;

			type::Null& Null();
			type::Blob& Blob();
			type::Stream& Stream();
			type::Flowc& Flowc();
			type::Expression& Expression();
			type::Function& Function();
			type::Name& Name();
			type::VMCall& VMCall();
			type::Type& Type();
			type::String& String();
			type::Number& Number();
			type::Arg& Arg();
			type::Bool& Bool();

			type::Generic* getRaw();

		private:
			type::Generic* generic;

	};

	namespace type {

		/// define Sequensa native function signature
		typedef std::vector<seq::Generic>*(*Native)(std::vector<seq::Generic>*);

	}

	namespace util {

		byte packTags( const long pos, const long end ) noexcept;
		constexpr long whole( const double val ) noexcept;
		type::Generic* copyGeneric( const type::Generic* entity );
		seq::Generic numberCast( seq::Generic arg );
		seq::Generic boolCast( seq::Generic arg );
		seq::Generic stringCast( seq::Generic arg );
		std::vector<FlowCondition*> copyFlowConditions( const std::vector<FlowCondition*> conditions );
		seq::Fraction asFraction( const double value );
		seq::DataType toDataType( const std::string str );

		seq::Generic newBool( bool value, bool anchor = false ) noexcept;
		seq::Generic newNumber( double value, bool anchor = false ) noexcept;
		seq::Generic newArg( byte value, bool anchor = false ) noexcept;
		seq::Generic newString( const char* value, bool anchor = false ) noexcept;
		seq::Generic newType( DataType value, bool anchor = false ) noexcept;
		seq::Generic newVMCall( type::VMCall::CallType value, bool anchor = false ) noexcept;
		seq::Generic newFunction( BufferReader* reader, bool end, bool anchor = false ) noexcept;
		seq::Generic newExpression( ExprOperator op, BufferReader* left, BufferReader* right, bool anchor = false ) noexcept;
		seq::Generic newFlowc( const std::vector<FlowCondition*> readers, bool anchor = false ) noexcept;
		seq::Generic newStream( byte tags, BufferReader* reader, bool anchor = false ) noexcept;
		seq::Generic newNull( bool anchor = false ) noexcept;

		int insertUnique( StringTable* table, std::string entry );
		std::string tableToString( StringTable& table, std::string separator = " " );

	}

	/// define shorthand for stream
	typedef std::vector<seq::Generic> Stream;

	/// Internal API error
	class InternalError: public std::exception {

		private:
			std::string error;

		public:
			explicit InternalError ( const std::string& error );
			virtual const char* what() const throw();
	};

	/// Runtime error
	class RuntimeError: public std::exception {

		private:
			std::string error;

		public:
			explicit RuntimeError ( const std::string& error );
			virtual const char* what() const throw();
	};

	/// Executor Interruption
	class ExecutorInterrupt: public std::exception {

		private:
			byte code;

		public:
			explicit ExecutorInterrupt ( const byte code );
			virtual const char* what() const throw();
			byte getCode();
	};

	/// Compilation error
	class CompilerError: public std::exception {

		private:
			std::string error;
			byte level;

		public:
			explicit CompilerError ( const byte level, const std::string& unexpected, const std::string& expected, const std::string& structure, int line );
			explicit CompilerError ( const std::string& warning, const std::string& structure, int line );
			virtual const char* what() const throw();
			bool isCritical();
			bool isError();
			bool isWarning();
			byte getLevel();
	};

	class FileHeader {

		public:
			FileHeader();
			FileHeader( byte seq_major, byte seq_minor, byte seq_patch, std::map<std::string, std::string> properties );
			FileHeader( const FileHeader& header );
			FileHeader( FileHeader&& header );
			bool checkVersion( byte seq_major, byte seq_minor );
			bool checkPatch( byte seq_patch );
			std::string& getValue( const char* key );
			int getVersionMajor();
			int getVersionMinor();
			int getVersionPatch();
			std::string getVersionString();
			std::map<std::string, std::string> getValueMap();
			StringTable getValueTable( const char* key );

			FileHeader& operator= ( const FileHeader& header );
			FileHeader& operator= ( FileHeader&& header ) noexcept;

		private:
			byte seq_major;
			byte seq_minor;
			byte seq_patch;
			std::map<std::string, std::string> properties;
	};

	/// token reader
	class TokenReader {

		public:
			TokenReader( BufferReader& reader, bool useStringTable );
			DataType getDataType();
			bool isAnchored();
			seq::Generic& getGeneric();

		private:
			BufferReader& reader;
			byte header;
			bool anchor: 4;
			bool strings: 4;
			DataType type;
			seq::Generic generic;

			DataType getDataType( byte header );
			type::Bool* loadBool();
			type::Number* loadNumber();
			type::Arg* loadArg();
			type::String* loadString();
			type::Type* loadType();
			type::VMCall* loadCall();
			type::Name* loadName();
			type::Function* loadFunc();
			type::Expression* loadExpr();
			type::Flowc* loadFlowc();
			type::Stream* loadStream();
	};

	/// Byte buffer
	class ByteBuffer {

		public:
			ByteBuffer( byte* buffer, long length );
			BufferReader getReader();
			BufferReader getReader( long first, long last );
			void setStringTable( StringTable* table );
			StringTable* getStringTable();
			long size();

		private:
			byte* pointer;
			long length;
			StringTable* table;
	};

	class BufferReader {

		public:
			BufferReader( byte* buffer, long first, long last, StringTable* table = nullptr );
			const byte nextByte() noexcept;
			const byte peekByte() noexcept;
			const bool hasNext() noexcept;
			TokenReader next() noexcept;
			BufferReader* nextBlock( long length );
			FileHeader getHeader();
			Stream readAll();
			ByteBuffer getSubBuffer();
			StringTable* getTable();

			void nextString( std::string* str );
			unsigned long nextUnsigned();

		private:
			long first;
			long last;
			long position;
			byte* pointer;
			StringTable* table;
	};

	class BufferWriter {

		public:
			BufferWriter( std::vector<byte>& buffer, StringTable* table = nullptr );
			void putNull( bool anchor );
			void putBool( bool anchor, bool value );
			void putNumber( bool anchor, Fraction f );
			void putArg( bool anchor, byte level );
			void putString( bool anchor, const char* str );
			void putType( bool anchor, DataType type );
			void putCall( bool anchor, type::VMCall::CallType type );
			void putName( bool anchor, bool define, const char* name );
			void putFunc( bool anchor, std::vector<byte>& buffer, bool end );
			void putExpr( bool anchor, ExprOperator op, std::vector<byte>& left, std::vector<byte>& right );
			void putStaticAccess( bool anchor, const char* str, byte index );
			void putFlowc( bool anchor, std::vector<std::vector<byte>>& buffers );
			void putStream( bool anchor, byte tags, std::vector<byte>& buf );
			void putFileHeader( byte seq_major, byte seq_minor, byte seq_patch, const std::map<std::string, std::string>& data );

			// Currently it only supports pure types
			void putGeneric( Generic& gereric );

		public: // Only use if you know what are you doing!
			void putByte( byte b );
			void putString( const char* str );
			void putOpcode( bool anchor, seq::Opcode code );
			void putUnsigned( byte length, unsigned long value );
			void putUnsigned( unsigned long value );
			void putHead( byte left, byte right );
			void putBuffer( std::vector<byte>& buffer );
			void putArray( const byte* data, size_t size );

			std::vector<byte>& buffer;
			StringTable* table;
	};

	class StackLevel {

		public:
			StackLevel();
			StackLevel( seq::Generic arg );
			StackLevel( StackLevel&& level );
			seq::Generic getArg();
			Stream getVar( std::string& name, bool anchor );
			void setVar( std::string& name, Stream value );
			bool hasVar( std::string& name );
			void setArg( seq::Generic arg );

		private:
			seq::Generic arg;
			std::unordered_map<std::string, Stream> vars;
	};

	class FlowCondition {
		public:
			enum struct Type: byte {
				Type = 1,
				Value = 2,
				Range = 3
			};

			FlowCondition( Type type, seq::Generic a, seq::Generic b );
			bool validate( seq::Generic arg );

			Type type;
			seq::Generic a;
			seq::Generic b;
	};

	class CommandResult {
		public:
			enum struct ResultType: byte {
				Return = 1,
				Break = 2,
				Exit = 3,
				Again = 4,
				Final = 5,
				None = 6,
			};

			CommandResult( ResultType stt, Stream acc );

			ResultType stt;
			Stream acc;
	};

	class Executor {

		public:
			Executor( Executor* parent );
			Executor();
			void inject( std::string name, seq::type::Native native );
			void define( std::string name, seq::Stream stream );
			StackLevel* getLevel( int level );
			StackLevel* getTopLevel();
			void reset();
			std::string getResultString();
			seq::Generic getResult();
			seq::Stream& getResults();
			void setStrictMath( bool flag );
			void execute( ByteBuffer bb, seq::Stream args = { seq::Generic( new type::Null( false ) ) }, bool stack = true );

		public: // use these methods only if you know what you are doing
			void exit( seq::Stream& stream, byte code );
			Stream executeFunction( BufferReader br, Stream& stream, bool end, bool stack = true );
			CommandResult executeCommand( TokenReader* br, byte tags );
			CommandResult executeStream( Stream& stream );
			CommandResult executeAnchor( Generic entity, Stream& input_stream );
			Generic executeExprPair( Generic left, Generic right, ExprOperator op, bool anchor );
			Generic executeExpr( Generic entity );
			Stream resolveName( std::string& name, bool anchor );
			void defineName( std::string& name, Stream& value, bool define = true );
			Stream executeFlowc( std::vector<FlowCondition*> fcs, Stream& input_stream );
			Generic executeCast( Generic cast, Generic arg );
			type::Native resolveNative( std::string& name );
			std::unordered_map<std::string, type::Native>& getNativesMap();

		private:
			std::unordered_map<std::string, type::Native> natives;
			std::vector<StackLevel> stack;
			seq::Stream result;
			Executor* parent;
			bool strictMath: 1;
	};

#ifndef SEQ_EXCLUDE_COMPILER

	// optimizations bitfield type
	typedef unsigned int oflag_t;

	// read more about this enum in documentation at section 8.
	enum struct Optimizations: oflag_t {
		None = 0b0000,
		All = 0b1111,
		Name = 0b1000,
		PureExpr = 0b0100,
		StrPreGen = 0b0010
	};

	class Compiler {

		public:

			// define error handle signature
			using ErrorHandle = bool (*) (seq::CompilerError*);

			class Token {

				public:
					enum class Category: byte {
						Tag = 0,
						Set = 1,
						Load = 2,
						Bool = 3,
						Null = 4,
						Type = 5,
						Name = 6,
						Number = 7,
						Stream = 8,
						Operator = 9,
						String = 10,
						FuncBracket = 11,
						FlowBracket = 12,
						MathBracket = 13,
						Comma = 14,
						Colon = 15,
						Arg = 16,
						VMCall = 17
					};

					Token( unsigned int line, long data, bool anchor, Category category, std::string& raw, std::string& clean );
					Token( const Token& token );
					Token( Token&& token );
					const unsigned int getLine();
					const Category getCategory();
					const std::string& getRaw();
					const std::string& getClean();
					const bool isPrimitive();
					const bool isPure();
					const bool isNamed();
					const long getData();
					const bool getAnchor();
					std::string toString();

				private:
					const unsigned int line;
					const long data;
					const bool anchor;
					const Category category;
					const std::string raw;
					const std::string clean;

			};

		private:
			StringTable* names;
			StringTable* loades;
			ErrorHandle handle;
			oflag_t flags;

		public:
			Compiler();

			void setNameTable( StringTable* strings );
			void setLoadTable( StringTable* strings );
			void setErrorHandle( ErrorHandle handle );
			void setOptimizationFlags( oflag_t flags );

			std::vector<byte> compile( std::string code );
			std::vector<Token> tokenize( std::string code );

			static bool defaultErrorHandle( CompilerError* err );

			// Legacy API
			static std::vector<byte> compileStatic( std::string codec, StringTable* headerData = nullptr, oflag_t flags = (oflag_t) Optimizations::None );
			static std::vector<Token> tokenizeStatic( std::string code );

		private:
			Token construct( std::string raw, unsigned int line );

			int findStreamEnd( std::vector<Token>& tokens, int start, int end );
			int findOpening( std::vector<Token>& tokens, int index, Token::Category type );
			int findClosing( std::vector<Token>& tokens, int index, Token::Category type );

			std::vector<byte> assembleStream( std::vector<Token>& tokens, int start, int end, byte tags, bool embedded );
			std::vector<byte> assemblePrimitive( Token );
			std::vector<byte> assembleFlowc( std::vector<Token>& tokens, int start, int end, bool anchor );
			std::vector<byte> assembleExpression( std::vector<Token>& tokens, int start, int end, bool anchor, bool top, bool* pure = nullptr );
			std::vector<byte> assembleFunction( std::vector<Token>& tokens, int start, int end, bool anchor, bool raw = false );

			void optimizeIfApplicable( std::vector<Token>& tokens );
			int extractHeaderData( std::vector<Token>& tokens, StringTable* arrayPtr );
			StringTable* getTable();

			void emit( seq::CompilerError error );
			void fail( seq::CompilerError error );
			void warn( seq::CompilerError warning );

	};

#endif // SEQ_EXCLUDE_COMPILER

#ifndef SEQ_EXCLUDE_DECOMPILER

	class Indentation {

		public:
			Indentation();
			void push();
			void pop();
			void set(std::string& str, int base);
			std::string get();

		private:
			std::string str;
			int index;

	};

	class Decompiler {

		public:
			Decompiler();
			virtual ~Decompiler() {};

			void setIndentation(std::string str, int base = 0);
			std::string decompile(BufferReader& reader);
			std::string decompile(Generic& generic);
			std::string decompile(seq::Stream& stream);

		protected:
			Indentation indentation;

			virtual std::string writeBool( Generic& tr ) = 0;
			virtual std::string writeNull( Generic& tr ) = 0;
			virtual std::string writeNumber( Generic& tr ) = 0;
			virtual std::string writeFunc( Generic& tr ) = 0;
			virtual std::string writeFlowc( Generic& tr ) = 0;
			virtual std::string writeExpr( Generic& tr ) = 0;
			virtual std::string writeName( Generic& tr ) = 0;
			virtual std::string writeString( Generic& tr ) = 0;
			virtual std::string writeStream( Generic& tr ) = 0;
			virtual std::string writeVMCall( Generic& tr ) = 0;
			virtual std::string writeArg( Generic& tr ) = 0;
			virtual std::string writeType( Generic& tr ) = 0;

			virtual std::string writeInvalid( Generic& tr );

	};

	class SourceDecompiler: public Decompiler {

		private:
			std::string writeBool( Generic& g ) override;
			std::string writeNull( Generic& g ) override;
			std::string writeNumber( Generic& g ) override;
			std::string writeFunc( Generic& g ) override;
			std::string writeFlowc( Generic& g ) override;
			std::string writeExpr( Generic& g ) override;
			std::string writeName( Generic& g ) override;
			std::string writeString( Generic& g ) override;
			std::string writeStream( Generic& g ) override;
			std::string writeVMCall( Generic& g ) override;
			std::string writeArg( Generic& g ) override;
			std::string writeType( Generic& g ) override;

			std::string anchor( Generic& g );
			std::string condition( FlowCondition* c );
			std::string exprop( ExprOperator op );

	};

#endif // SEQ_EXCLUDE_DECOMPILER

}

using seq::byte;

#ifdef SEQ_IMPLEMENT

byte seq::util::packTags( const long pos, const long end ) noexcept {
	byte tags = 0;

	tags |= ( ( pos == 0 ) ? SEQ_TAG_FIRST : 0 );
	tags |= ( ( pos == end - 1 ) ? SEQ_TAG_LAST : 0 );
	tags |= ( ( pos == end ) ? SEQ_TAG_END : 0 );

	return tags;
}

constexpr long seq::util::whole( const double val ) noexcept {
	return (long) val;
}

seq::type::Generic* seq::util::copyGeneric( const seq::type::Generic* entity ) {

	typedef seq::type::Generic G;
	typedef G* (*copyFunc)( const G* );
	static copyFunc copyFuncArr[ SEQ_MAX_DATA_TYPE ] = {
			/* 1  Bool   */ [] (const G* entity) -> G* { return new seq::type::Bool( *(seq::type::Bool*) entity ); },
			/* 2  Null   */ [] (const G* entity) -> G* { return new seq::type::Null( *(seq::type::Null*) entity ); },
			/* 3  Number */ [] (const G* entity) -> G* { return new seq::type::Number( *(seq::type::Number*) entity ); },
			/* 4  String */ [] (const G* entity) -> G* { return new seq::type::String( *(seq::type::String*) entity ); },
			/* 5  Type   */ [] (const G* entity) -> G* { return new seq::type::Type( *(seq::type::Type*) entity ); },
			/* 6  VMCall */ [] (const G* entity) -> G* { return new seq::type::VMCall( *(seq::type::VMCall*) entity ); },
			/* 7  Arg    */ [] (const G* entity) -> G* { return new seq::type::Arg( *(seq::type::Arg*) entity ); },
			/* 8  Func   */ [] (const G* entity) -> G* { return new seq::type::Function( *(seq::type::Function*) entity ); },
			/* 9  Expr   */ [] (const G* entity) -> G* { return new seq::type::Expression( *(seq::type::Expression*) entity ); },
			/* 10 Name   */ [] (const G* entity) -> G* { return new seq::type::Name( *(seq::type::Name*) entity ); },
			/* 11 Flowc  */ [] (const G* entity) -> G* { return new seq::type::Flowc( *(seq::type::Flowc*) entity ); },
			/* 12 Stream */ [] (const G* entity) -> G* { return new seq::type::Stream( *(seq::type::Stream*) entity ); },
			/* 13 Blob   */ [] (const G* entity) -> G* { return ((seq::type::Blob*) entity)->copy(); }
	};

	// this may fail if given entity has incorrect DataType
	return copyFuncArr[((seq::byte) entity->getDataType()) - 1]( entity );

}

seq::Generic seq::util::numberCast( seq::Generic arg ) {
	seq::type::Number* num = nullptr;

	switch( arg.getDataType() ) {

		case seq::DataType::Number: return arg;

		case seq::DataType::Bool:
			if( arg.Bool().getBool() ) {
				num = new seq::type::Number( false, 1 );
			}else{
				num = new seq::type::Number( false, 0 );
			}
			break;

		case seq::DataType::Null:
			num = new seq::type::Number( false, 0 );
			break;

		case seq::DataType::String:
			try {
				num = new seq::type::Number( false, std::stod( arg.String().getString() ) );
			} catch (std::invalid_argument &err) {
				num = new seq::type::Number( false, 0 );
			}
			break;

		case seq::DataType::VMCall:
		case seq::DataType::Flowc:
		case seq::DataType::Func:
		case seq::DataType::Blob:
		case seq::DataType::Type:
			num = new seq::type::Number( false, 1 );
			break;

		// invalid casts: (stream, name, expr, arg)
		default: throw seq::InternalError( "Invalid cast!" );

	}

	return seq::Generic( num );
}

seq::Generic seq::util::boolCast( seq::Generic arg ) {
	if( arg.getDataType() == seq::DataType::Bool ) return arg;

	bool val = ( seq::util::numberCast( arg ).Number().getDouble() != 0 );

	return newBool( val );
}

seq::Generic seq::util::stringCast( seq::Generic arg ) {
	seq::type::String* str = nullptr;

	switch( arg.getDataType() ) {

		case seq::DataType::String: return arg;

		case seq::DataType::Bool:
			if( arg.Bool().getBool() ) {
				str = new seq::type::String( false, "true" );
			}else{
				str = new seq::type::String( false, "false" );
			}
			break;

		case seq::DataType::Null:
			str = new seq::type::String( false, "null" );
			break;

		case seq::DataType::Number:
			str = new seq::type::String( false, (
					arg.Number().isNatural() ?
							std::to_string( arg.Number().getLong() ) :
							std::to_string( arg.Number().getDouble() ) ).c_str()  );
			break;

		case seq::DataType::Flowc:
			str = new seq::type::String( false, "flowc" );
			break;

		case seq::DataType::VMCall:
		case seq::DataType::Func:
			str = new seq::type::String( false, "func" );
			break;

		case seq::DataType::Blob:
			str = new seq::type::String( false, arg.Blob().toString().c_str() );
			break;

		case seq::DataType::Type:
			str = new seq::type::String( false, "type" );
			break;

		// invalid casts: (stream, name, expr, arg)
		default: throw seq::InternalError( "Invalid cast from datatype id: " + std::to_string( (int) arg.getDataType() ) + "!" );

	}

	return seq::Generic( str );
}

std::vector<seq::FlowCondition*> seq::util::copyFlowConditions( const std::vector<seq::FlowCondition*> conditions ) {
	std::vector<seq::FlowCondition*> list;
	list.reserve( conditions.size() );

	for( const seq::FlowCondition* condition : conditions ) {
		list.push_back( new seq::FlowCondition( *condition ) );
	}

	return list;
}

seq::Fraction seq::util::asFraction( const double value ) {
	return seq::type::Number( false, value ).getFraction();
}

seq::DataType seq::util::toDataType( const std::string str ) {
	if( str == "number" ) return seq::DataType::Number;
	if( str == "bool" ) return seq::DataType::Bool;
	if( str == "string" ) return seq::DataType::String;
	if( str == "type" ) return seq::DataType::Type;
	throw seq::InternalError( "Invalid argument!" );
}

seq::Generic seq::util::newBool( bool value, bool anchor ) noexcept {
	return seq::Generic( new seq::type::Bool( anchor, value ) );
}

seq::Generic seq::util::newNumber( double value, bool anchor ) noexcept {
	return seq::Generic( new seq::type::Number( anchor, value ) );
}

seq::Generic seq::util::newArg( byte value, bool anchor ) noexcept {
	return seq::Generic( new seq::type::Arg( anchor, value ) );
}

seq::Generic seq::util::newString( const char* value, bool anchor ) noexcept {
	return seq::Generic( new seq::type::String( anchor, value ) );
}

seq::Generic seq::util::newType( DataType value, bool anchor ) noexcept {
	return seq::Generic( new seq::type::Type( anchor, value ) );
}

seq::Generic seq::util::newVMCall( type::VMCall::CallType value, bool anchor ) noexcept {
	return seq::Generic( new seq::type::VMCall( anchor, value ) );
}

seq::Generic seq::util::newFunction( BufferReader* reader, bool end, bool anchor ) noexcept {
	return seq::Generic( new seq::type::Function( anchor, reader, end ) );
}

seq::Generic seq::util::newExpression( ExprOperator op, BufferReader* left, BufferReader* right, bool anchor ) noexcept {
	return seq::Generic( new seq::type::Expression( anchor, op, left, right ) );
}

seq::Generic seq::util::newFlowc( const std::vector<FlowCondition*> readers, bool anchor ) noexcept {
	return seq::Generic( new seq::type::Flowc( anchor, readers ) );
}

seq::Generic seq::util::newStream( byte tags, BufferReader* reader, bool anchor ) noexcept {
	return seq::Generic( new seq::type::Stream( anchor, tags, reader ) );
}

seq::Generic seq::util::newNull( bool anchor ) noexcept {
	return seq::Generic( new seq::type::Null( anchor ) );
}

int seq::util::insertUnique( seq::StringTable* table, std::string entry ) {
	auto it = std::find(table->begin(), table->end(), entry);
	int index = std::distance(table->begin(), it);

	if( it == table->end() ) {
		table->push_back( entry );
	}

	return index;
}

std::string seq::util::tableToString( StringTable& table, std::string separator ) {
	std::string str;

	for( std::string& entry : table ) {
		str += entry;
		str += separator;
	}

	str.erase( str.size() - separator.size() );

	return str;
}

seq::BufferWriter::BufferWriter( std::vector<byte>& buffer, StringTable* table ): buffer( buffer ), table( table ) {}

void seq::BufferWriter::putByte( byte b ) {
	this->buffer.push_back( b );
}

void seq::BufferWriter::putOpcode( bool anchor, seq::Opcode code ) {
	this->putByte( (byte) code | (anchor ? 0b10000000 : 0) );
}

void seq::BufferWriter::putString( const char* str ) {
	if( table == nullptr ) {
		for ( long i = 0; str[i]; i ++ ) this->putByte( str[i] );
		this->putByte( 0 );
	}else{
		putUnsigned( util::insertUnique(table, str) );
	}
}

void seq::BufferWriter::putUnsigned( unsigned long value ) {
	if( value < 0b1111 ) {
		this->putHead( 0, value );
	}else{
		byte size = seq::type::Number::sizeOf(value);
		this->putHead( size, 0 );
		this->putUnsigned( size, value );
	}
}

void seq::BufferWriter::putUnsigned( byte length, unsigned long value ) {
	for( ; length != 0; length -- ) {
		this->putByte( (byte) ( value & 0xFFl ) );
		value = ( value >> 8 );
	}
}

void seq::BufferWriter::putHead( byte left, byte right ) {
	this->putByte( (left << 4) | right );
}

void seq::BufferWriter::putBuffer( std::vector<byte>& buf ) {
	this->buffer.insert(this->buffer.end(), buf.begin(), buf.end());
}

void seq::BufferWriter::putArray( const byte* data, size_t size ) {
	for( size_t i = 0; i < size; i ++ ) this->putByte( *(i + data) );
}

void seq::BufferWriter::putNull( bool anchor ) {
	this->putOpcode( anchor, seq::Opcode::NIL );
}

void seq::BufferWriter::putBool( bool anchor, bool value ) {
	this->putOpcode( anchor, (value ? seq::Opcode::BLT : seq::Opcode::BLF) );
}

void seq::BufferWriter::putNumber( bool anchor, seq::Fraction f ) {
	if( f.denominator == 1 && f.numerator >= 0 && f.numerator <= 0b01111111 ) {
		this->putOpcode( anchor, seq::Opcode::INT );
		this->putByte( (byte) f.numerator );
	}else{
		bool sign = (f.numerator < 0);
		unsigned long n = std::labs( f.numerator );

		this->putOpcode( anchor, seq::Opcode::NUM );
		byte a = seq::type::Number::sizeOf( n << 1 );
		byte b = seq::type::Number::sizeOf( f.denominator );
		this->putHead( a, b );
		this->putUnsigned( a, sign ? n | (1 << (a * 8 - 1)) : n );
		this->putUnsigned( b, f.denominator );
	}
}

void seq::BufferWriter::putArg( bool anchor, byte level ) {
	this->putOpcode( anchor, seq::Opcode::ARG );
	this->putByte( level );
}

void seq::BufferWriter::putString( bool anchor, const char* str ) {
	this->putOpcode( anchor, seq::Opcode::STR );
	this->putString( str );
}

void seq::BufferWriter::putType( bool anchor, DataType type ) {
	this->putOpcode( anchor, seq::Opcode::TYP );
	this->putByte( (byte) type );
}

void seq::BufferWriter::putCall( bool anchor, type::VMCall::CallType type ) {
	this->putOpcode( anchor, seq::Opcode::VMC );
	this->putByte( (byte) type );
}

void seq::BufferWriter::putName( bool anchor, bool define, const char* name ) {
	this->putOpcode( anchor, (define ? seq::Opcode::DEF : seq::Opcode::VAR) );
	this->putString( name );
}

void seq::BufferWriter::putFunc( bool anchor, std::vector<byte>& buf, bool end ) {
	this->putOpcode( anchor, (end ? seq::Opcode::FNE : seq::Opcode::FUN) );
	this->putUnsigned( buf.size() );
	this->putBuffer( buf );
}

void seq::BufferWriter::putExpr( bool anchor, seq::ExprOperator op, std::vector<byte>& left, std::vector<byte>& right ) {
	size_t left_size = left.size();
	size_t right_size = right.size();

	bool tiny = ( left_size < 16 && right_size < 16 );

	this->putOpcode( anchor, tiny ? seq::Opcode::TEX : seq::Opcode::EXP );
	this->putByte( (byte) op );

	if( tiny ) {
		this->putByte( (left_size << 4) | right_size );
	}else{
		byte a = seq::type::Number::sizeOf( left_size );
		byte b = seq::type::Number::sizeOf( right_size );
		this->putHead( a, b );
		this->putUnsigned( a, left_size );
		this->putUnsigned( b, right_size );
	}

	this->putBuffer( left );
	this->putBuffer( right );
}

void seq::BufferWriter::putFlowc( bool anchor, std::vector<std::vector<byte>>& buffers ) {
	this->putOpcode( anchor, seq::Opcode::FLC );
	this->putByte( (byte) buffers.size() );
	for( auto& buf : buffers ) {
		this->putUnsigned( buf.size() );
		this->putBuffer( buf );
	}
}

void seq::BufferWriter::putStream( bool anchor, byte tags, std::vector<byte>& buf ) {
	this->putOpcode( anchor, seq::Opcode::SSL );
	this->putByte( tags );
	this->putUnsigned( buf.size() );
	this->putBuffer( buf );
}

void seq::BufferWriter::putFileHeader( byte seq_major, byte seq_minor, byte seq_patch, const std::map<std::string, std::string>& data ) {
	this->putByte( 's' );
	this->putByte( 'q' );
	this->putByte( 'c' );
	this->putByte(  0  );
	this->putByte( seq_major );
	this->putByte( seq_minor );
	this->putByte( seq_patch );

	size_t size = data.size();

	if( size > 0xFF ) {
		throw seq::InternalError( "Header data section too long to encode!" );
	}

	this->putByte( (byte) size );

	for( const auto& x : data ) {
		size = x.first.size();

		if( size > 0xFF ) {
			throw seq::InternalError( "The maximum length of data key exceed!" );
		}

		this->putByte( (byte) size );
		this->putArray( (const byte*) x.first.data(), size );

		size = x.second.size();

		this->putUnsigned( size );
		this->putArray( (const byte*) x.second.data(), size );
	}

}

void seq::BufferWriter::putGeneric( seq::Generic& generic ) {

	bool anchor = generic.getAnchor();

	switch( generic.getDataType() ) {
		case seq::DataType::Bool: putBool(anchor, generic.Bool().getBool()); break;
		case seq::DataType::Number: putNumber(anchor, generic.Number().getFraction()); break;
		case seq::DataType::String: putString(anchor, generic.String().getString().c_str()); break;
		case seq::DataType::Null: putNull(anchor); break;
		case seq::DataType::Type: putType(anchor, generic.Type().getDataType()); break;

		default: throw seq::InternalError("Encoding of non-pure generics is not supported!");
	}

}

seq::FileHeader::FileHeader(): seq_major( 0 ), seq_minor( 0 ), seq_patch( 0 ), properties( {} ) {};

seq::FileHeader::FileHeader( byte _seq_major, byte _seq_minor, byte _seq_patch, std::map<std::string, std::string> _properties ): seq_major( _seq_major ), seq_minor( _seq_minor ), seq_patch( _seq_patch ), properties( _properties ) {}

seq::FileHeader::FileHeader( const FileHeader& header ) {
	this->seq_major = header.seq_major;
	this->seq_minor = header.seq_minor;
	this->seq_patch = header.seq_patch;
	this->properties = header.properties;
}

seq::FileHeader::FileHeader( FileHeader&& header ) {
	this->seq_major = header.seq_major;
	this->seq_minor = header.seq_minor;
	this->seq_patch = header.seq_patch;
	this->properties = std::move( header.properties );
}

bool seq::FileHeader::checkVersion( byte _seq_major, byte _seq_minor ) {
	return ( this->seq_major == _seq_major ) && ( this->seq_minor == _seq_minor );
}

bool seq::FileHeader::checkPatch( byte _seq_patch ) {
	return ( this->seq_patch == _seq_patch );
}

std::string& seq::FileHeader::getValue( const char* key ) {
	return this->properties.at( std::string(key) );
}

int seq::FileHeader::getVersionMajor() {
	return (int) this->seq_major;
}

int seq::FileHeader::getVersionMinor() {
	return (int) this->seq_minor;
}

int seq::FileHeader::getVersionPatch() {
	return (int) this->seq_patch;
}

std::string seq::FileHeader::getVersionString() {
	return std::to_string( this->getVersionMajor() ) + "." + std::to_string( this->getVersionMinor() ) + "." + std::to_string( this->getVersionPatch() );
}

std::map<std::string, std::string> seq::FileHeader::getValueMap() {
	return this->properties;
}

seq::StringTable seq::FileHeader::getValueTable( const char* key ) {
	try{
		std::string entry = this->properties.at(key), str = "";
		StringTable table;

		for( byte b : entry ) {
			if(b) str.push_back(b); else { table.push_back(str); str.clear(); }
		}

		return std::move(table);
	}catch(std::out_of_range& err) {
		return {};
	}
}

seq::FileHeader& seq::FileHeader::operator= ( const FileHeader& header ) {
	if( this != &header ) {
		this->seq_major = header.seq_major;
		this->seq_minor = header.seq_minor;
		this->seq_patch = header.seq_patch;
		this->properties = header.properties;
	}
	return *this;
}

seq::FileHeader& seq::FileHeader::operator= ( FileHeader&& header ) noexcept {
	if( this != &header ) {
		this->seq_major = header.seq_major;
		this->seq_minor = header.seq_minor;
		this->seq_patch = header.seq_patch;
		this->properties = std::move( header.properties );
	}
	return *this;
}

seq::type::Generic::Generic( const DataType _type, bool _anchor ): type( _type ), anchor( _anchor ) {}

const bool seq::type::Generic::getAnchor() const noexcept {
	return this->anchor;
}

void seq::type::Generic::setAnchor( bool anchor ) noexcept {
	this->anchor = anchor;
}

const seq::DataType seq::type::Generic::getDataType() const noexcept {
	return this->type;
}

seq::type::Bool::Bool( bool _anchor, bool _value ): seq::type::Generic( seq::DataType::Bool, _anchor ), value( _value ) {}

const bool seq::type::Bool::getBool() {
	return this->value;
}

seq::type::Arg::Arg( bool _anchor, byte _level ): seq::type::Generic( seq::DataType::Arg, _anchor ), level( _level ) {}

const byte seq::type::Arg::getLevel() {
	return this->level;
}

seq::type::Number::Number( bool _anchor, double _value ): seq::type::Generic( seq::DataType::Number, _anchor ), value( _value ) {}

seq::type::Number::Number( bool _anchor, long numerator, long denominator ): seq::type::Generic( seq::DataType::Number, _anchor ), value( (double) numerator / denominator ) {};

const double seq::type::Number::getDouble() {
	return this->value;
}

const long seq::type::Number::getLong() {
	return trunc( this->value );
}

const bool seq::type::Number::isNatural() {
	return this->getLong() == this->value;
}

const seq::Fraction seq::type::Number::getFraction() {

	long sign = ( this->value < 0 ) ? -1 : 1;
	double number = std::abs( this->value );
	double whole = std::trunc( number );
	double decimal = ( number - whole );
	unsigned long long multiplier = 1;

	if( decimal > 0 )
		for( double i = decimal; i > std::floor( i ) + FLT_EPSILON; i = multiplier * decimal )
			multiplier *= 10;

	long long part = std::round( decimal * multiplier );
	long hcf = 0;
	long u = part;
	long v = multiplier;

	while( true ) {
		if( !(u %= v) ) { hcf = v; break; }
		if( !(v %= u) ) { hcf = u; break; }
	}

	multiplier /= hcf;

	seq::Fraction fraction{ (long)(sign * ((part / hcf) + (whole * multiplier))), (long)multiplier };
	return fraction;
}

byte seq::type::Number::sizeOf( unsigned long value ) {
	int s = 0;

	for( int i = 1; i < 9; i ++ ) {
		if( value & 0b11111111 ) {
			s = i;
		}

		value = ( value >> 8 );
	}

	return s;
}

seq::type::String::String( bool _anchor, const char* _value ): seq::type::Generic( seq::DataType::String, _anchor ), value( _value ) {}

std::string& seq::type::String::getString() {
	return this->value;
}

seq::type::Type::Type( bool _anchor, seq::DataType _value ): seq::type::Generic( seq::DataType::Type, _anchor ), value( _value ) {}

const seq::DataType seq::type::Type::getType() {
	return this->value;
}

seq::type::VMCall::VMCall( bool _anchor, seq::type::VMCall::CallType _value ): seq::type::Generic( seq::DataType::VMCall, _anchor ), value( _value ) {}

const seq::type::VMCall::CallType seq::type::VMCall::getCall() {
	return this->value;
}

seq::type::Name::Name( bool _anchor, bool _define, std::string _name ): seq::type::Generic( seq::DataType::Name, _anchor ), define( _define ), name( _name ) {}

const bool seq::type::Name::getDefine() {
	return this->define;
}

std::string& seq::type::Name::getName() {
	return this->name;
}

seq::type::Function::Function( bool _anchor, seq::BufferReader* _reader, bool _end ): seq::type::Generic( seq::DataType::Func, _anchor ), reader( _reader ), end( _end ) {}

seq::type::Function::Function( const seq::type::Function& func ): seq::type::Generic( seq::DataType::Func, func.anchor ), reader( new seq::BufferReader( *(func.reader) ) ), end( func.end ) {}

seq::type::Function::~Function() {
	delete this->reader;
}

seq::BufferReader& seq::type::Function::getReader() {
	return *(this->reader);
}

const bool seq::type::Function::hasEnd() noexcept {
	return this->end;
}

seq::type::Expression::Expression( bool _anchor, seq::ExprOperator _op, seq::BufferReader* _left, seq::BufferReader* _right ): seq::type::Generic( seq::DataType::Expr, _anchor ), op( _op ), left( _left ), right( _right ) {}

seq::type::Expression::Expression( const seq::type::Expression& expr ): seq::type::Generic( seq::DataType::Expr, expr.anchor ), op( expr.op ), left( new seq::BufferReader( *(expr.left) ) ), right( new seq::BufferReader( *(expr.right) ) ) {}

seq::type::Expression::~Expression() {
	delete this->left;
	delete this->right;
}

const seq::ExprOperator seq::type::Expression::getOperator() {
	return this->op;
}

seq::BufferReader& seq::type::Expression::getLeftReader() {
	return *(this->left);
}

seq::BufferReader& seq::type::Expression::getRightReader() {
	return *(this->right);
}

seq::type::Stream::Stream( bool _anchor, byte _tags, BufferReader* _reader ): seq::type::Generic( seq::DataType::Stream, _anchor ), tags( _tags ), reader( _reader ) {}

seq::type::Stream::Stream( const seq::type::Stream& stream ): seq::type::Generic( seq::DataType::Stream, stream.anchor ), tags( stream.tags ), reader( new seq::BufferReader( *(stream.reader) ) ) {}

seq::type::Stream::~Stream() {
	delete this->reader;
}

const bool seq::type::Stream::matchesTags( byte _tags ) {

	// execute stream on end ONLY if it has that tag
	if( _tags & SEQ_TAG_END ) return this->tags & SEQ_TAG_END;

	// stream don't have any tags
	if( this->tags == 0 ) return true;

	 // check last & first tag
	if( this->tags & SEQ_TAG_FIRST ) return _tags & SEQ_TAG_FIRST;
	if( this->tags & SEQ_TAG_LAST ) return _tags & SEQ_TAG_LAST;

	// this stream is waiting for the end tag
	if( this->tags & SEQ_TAG_END ) return false;

	throw seq::InternalError( "Invalid Tag!" );
}

const byte seq::type::Stream::getTags() {
	return this->tags;
}

seq::BufferReader& seq::type::Stream::getReader() {
	return *(this->reader);
}

seq::type::Null::Null( bool _anchor ): seq::type::Generic( seq::DataType::Null, _anchor ) {}

seq::type::Blob::Blob( bool _anchor ): seq::type::Generic( seq::DataType::Blob, _anchor ) {}

std::string seq::type::Blob::toString() {
	return "blob";
}

seq::type::Blob* seq::type::Blob::copy() {
	return new seq::type::Blob( *this );
}

seq::type::Flowc::Flowc( bool _anchor, const std::vector< seq::FlowCondition* > _blocks ): seq::type::Generic( seq::DataType::Flowc, _anchor ), conditions( _blocks ) {}

seq::type::Flowc::Flowc( const seq::type::Flowc& flowc ): seq::type::Generic( seq::DataType::Flowc, flowc.anchor ), conditions( seq::util::copyFlowConditions( flowc.conditions ) ) {}

seq::type::Flowc::~Flowc() {
	for( const auto& fc : this->conditions ) delete fc;
}

const std::vector< seq::FlowCondition* >& seq::type::Flowc::getConditions() {
	return this->conditions;
}

seq::Generic::Generic() {
	this->generic = new seq::type::Null( false );
}

seq::Generic::Generic( seq::type::Generic* _generic ) {
	this->generic = _generic;
}

seq::Generic::Generic( const seq::Generic& _generic ) {
	this->generic = seq::util::copyGeneric( _generic.generic );
}

seq::Generic::Generic( seq::Generic&& _generic ) {
	this->generic = std::move( _generic.generic );
	_generic.generic = nullptr;
}

seq::Generic::~Generic() {
	if( this->generic != nullptr ) delete this->generic;
}

seq::Generic& seq::Generic::operator= ( const Generic& generic ) {
	if( this != &generic ) {
		seq::type::Generic* g = seq::util::copyGeneric( generic.generic );
		delete this->generic;
		this->generic = g;
	}
	return *this;
}

seq::Generic& seq::Generic::operator= ( Generic&& generic ) noexcept {
	if( this != &generic ) {
		delete this->generic;
		this->generic = generic.generic;
		generic.generic = nullptr;
	}
	return *this;
}

const seq::DataType seq::Generic::getDataType() const noexcept {
	return this->generic->getDataType();
}

bool seq::Generic::getAnchor() const noexcept {
	return this->generic->getAnchor();
}

void seq::Generic::setAnchor( bool anchor ) noexcept {
	return this->generic->setAnchor( anchor );
}

seq::type::Generic* seq::Generic::getRaw() {
	return this->generic;
}

seq::type::Null& seq::Generic::Null() {
	return *(static_cast<seq::type::Null*>(this->generic));
}

seq::type::Blob& seq::Generic::Blob() {
	return *(static_cast<seq::type::Blob*>(this->generic));
}

seq::type::Stream& seq::Generic::Stream() {
	return *(static_cast<seq::type::Stream*>(this->generic));
}

seq::type::Flowc& seq::Generic::Flowc() {
	return *(static_cast<seq::type::Flowc*>(this->generic));
}

seq::type::Expression& seq::Generic::Expression() {
	return *(static_cast<seq::type::Expression*>(this->generic));
}

seq::type::Function& seq::Generic::Function() {
	return *(static_cast<seq::type::Function*>(this->generic));
}

seq::type::Name& seq::Generic::Name() {
	return *(static_cast<seq::type::Name*>(this->generic));
}

seq::type::VMCall& seq::Generic::VMCall() {
	return *(static_cast<seq::type::VMCall*>(this->generic));
}

seq::type::Type& seq::Generic::Type() {
	return *(static_cast<seq::type::Type*>(this->generic));
}

seq::type::String& seq::Generic::String() {
	return *(static_cast<seq::type::String*>(this->generic));
}

seq::type::Number& seq::Generic::Number() {
	return *(static_cast<seq::type::Number*>(this->generic));
}

seq::type::Arg& seq::Generic::Arg() {
	return *(static_cast<seq::type::Arg*>(this->generic));
}

seq::type::Bool& seq::Generic::Bool() {
	return *(static_cast<seq::type::Bool*>(this->generic));
}

seq::InternalError::InternalError( const std::string& error ) {
	this->error = "Internal Sequensa error occured: " + error;
}

const char* seq::InternalError::what() const throw() {
	return this->error.c_str();
}

seq::RuntimeError::RuntimeError( const std::string& error ) {
	this->error = error;
}

const char* seq::RuntimeError::what() const throw() {
	return this->error.c_str();
}

seq::ExecutorInterrupt::ExecutorInterrupt( const byte code ) {
	this->code = code;
}

const char* seq::ExecutorInterrupt::what() const throw() {
	return std::to_string( (int) this->code ).c_str();
}

byte seq::ExecutorInterrupt::getCode() {
	return this->code;
}

seq::CompilerError::CompilerError( const byte level, const std::string& unexpected, const std::string& expected, const std::string& structure, int line ) {
	if( unexpected.empty() && expected.empty() ) this->error = "Unknown error"; else
	if( !unexpected.empty() && expected.empty() ) this->error = "Unexpected " + unexpected; else
	if( unexpected.empty() && !expected.empty() ) this->error = "Expected " + expected; else
	if( !unexpected.empty() && !expected.empty() ) this->error = "Unexpected " + unexpected + " (expected " + expected + ")";
	if( !structure.empty() ) this->error += " in " + structure;
	this->error += " at line: " + std::to_string(line);
	this->level = level;
}

seq::CompilerError::CompilerError( const std::string& warning, const std::string& structure, int line ) {
	this->error = warning;
	if( !structure.empty() ) this->error += " in " + structure;
	this->error += " at line: " + std::to_string(line);
	this->level = 0;
}

bool seq::CompilerError::isCritical() {
	return this->level == 2;
}

bool seq::CompilerError::isError() {
	return this->level > 0;
}

bool seq::CompilerError::isWarning() {
	return this->level == 0;
}

byte seq::CompilerError::getLevel() {
	return this->level;
}

const char* seq::CompilerError::what() const throw() {
	return this->error.c_str();
}

seq::ByteBuffer::ByteBuffer( byte* buffer, long length ) {
	this->pointer = buffer;
	this->length = length;
	this->table = nullptr;
}

void seq::ByteBuffer::setStringTable( StringTable* table ) {
	this->table = table;
}

seq::StringTable* seq::ByteBuffer::getStringTable() {
	return table;
}

long seq::ByteBuffer::size() {
	return length;
}

seq::BufferReader seq::ByteBuffer::getReader() {
	return seq::BufferReader( this->pointer, 0, this->length - 1, table );
}

seq::BufferReader seq::ByteBuffer::getReader( long first, long last ) {
	if( first < 0 || last > (this->length - 1) || first > last ) throw seq::InternalError( "Invalid BufferReader range!" );
	return seq::BufferReader( this->pointer, first, last, table );
}

seq::BufferReader::BufferReader( byte* buffer, long first, long last, StringTable* table ) {
	this->pointer = buffer;
	this->position = first - 1;
	this->first = first;
	this->last = last;
	this->table = table;
}

const byte seq::BufferReader::peekByte() noexcept {
	if( this->position >= this->last ) return 0;
	return this->pointer[ this->position + 1 ];
}

const byte seq::BufferReader::nextByte() noexcept {
	if( this->position >= this->last ) return 0;
	return this->pointer[ ++ this->position ];
}

const bool seq::BufferReader::hasNext() noexcept {
	return ( this->position < this->last );
}

seq::TokenReader seq::BufferReader::next() noexcept {
	return seq::TokenReader( *this, table != nullptr );
}

seq::BufferReader* seq::BufferReader::nextBlock( long length ) {
	if( length < 0 ) throw seq::InternalError( "Invalid block size!" );

	long new_pos = this->position + length;
	long old_pos = this->position + 1;

	this->position = std::min( new_pos, this->last );
	return new seq::BufferReader( this->pointer, old_pos, this->position, this->table );
}

seq::FileHeader seq::BufferReader::getHeader() {

	// validate file signature
	const byte s1[] = { this->nextByte(), this->nextByte(), this->nextByte(), this->nextByte() };
	const byte s2[] = { 's', 'q', 'c', 0 };
	if( !std::equal(std::begin(s1), std::end(s1), std::begin(s2)) ) throw InternalError( "Invalid header signature!" );

	// read sequensa version
	byte seq_major = this->nextByte();
	byte seq_minor = this->nextByte();
	byte seq_patch = this->nextByte();

	std::map<std::string, std::string> data;

	// read header data
	{
		// number of key-value pairs
		long n = this->nextByte();
		std::string key, val;

		for( ; n > 0; n -- ) {

			{ // load key
				size_t m = this->nextByte();
				for( ; m > 0; m -- ) key.push_back( this->nextByte() );
			}

			{ // load value
				size_t m = this->nextUnsigned();
				for( ; m > 0; m -- ) val.push_back( this->nextByte() );
			}

			data.insert( { key, val } );
			key.clear();
			val.clear();

		}
	}

	return seq::FileHeader( seq_major, seq_minor, seq_patch, data );
}

seq::Stream seq::BufferReader::readAll() {
	seq::Stream stream;
	while( this->hasNext() ) {
		TokenReader tr = this->next();
		stream.push_back( tr.getGeneric() );
	}

	return stream;
}

seq::ByteBuffer seq::BufferReader::getSubBuffer() {
	return seq::ByteBuffer( this->pointer + this->position + 1, this->last - this->position );
}

seq::StringTable* seq::BufferReader::getTable() {
	return table;
}

void seq::BufferReader::nextString( std::string* str ) {
	if( table != nullptr ) {
		try{
			*str = table->at( nextUnsigned() );
		}catch(std::out_of_range& err) {
			throw seq::InternalError("Invalid string index!");
		}
	}else{
		byte b;
		while( true ) {
			b = this->nextByte();
			if( b ) str->push_back( b ); else return;
		}
	}
}

unsigned long seq::BufferReader::nextUnsigned() {
	byte head = this->nextByte();
	unsigned long value = 0;

	// integer length capped at 8 bytes
	byte length = (head >> 4) % 9;

	// load number byte by byte
	for( byte i = 0; i < length; i ++ ) {
		value |= (( (long) this->nextByte() ) << (i * 8));
	}

	// add number offset
	return value + (head & 0b00001111);
}

seq::TokenReader::TokenReader( seq::BufferReader& reader, bool table ): reader( reader ), strings( table ) {
	byte header = reader.nextByte();

	this->header = (byte) ( header & 0b01111111 );
	this->anchor = (bool) ( header & 0b10000000 );
	this->type = getDataType( this->header );

	typedef seq::type::Generic G;
	typedef seq::TokenReader TR;
	typedef G* (*loadFunc)( TR* );

	static loadFunc loadFuncArr[ SEQ_MAX_DATA_TYPE ] = {
		/* 1  Bool   */ [] (TR* tr) -> G* { return tr->loadBool(); },
		/* 2  Null   */ [] (TR* tr) -> G* { return new seq::type::Null( tr->anchor ); },
		/* 3  Number */ [] (TR* tr) -> G* { return tr->loadNumber(); },
		/* 4  String */ [] (TR* tr) -> G* { return tr->loadString(); },
		/* 5  Type   */ [] (TR* tr) -> G* { return tr->loadType(); },
		/* 6  VMCall */ [] (TR* tr) -> G* { return tr->loadCall(); },
		/* 7  Arg    */ [] (TR* tr) -> G* { return tr->loadArg(); },
		/* 8  Func   */ [] (TR* tr) -> G* { return tr->loadFunc(); },
		/* 9  Expr   */ [] (TR* tr) -> G* { return tr->loadExpr(); },
		/* 10 Name   */ [] (TR* tr) -> G* { return tr->loadName(); },
		/* 11 Flowc  */ [] (TR* tr) -> G* { return tr->loadFlowc(); },
		/* 12 Stream */ [] (TR* tr) -> G* { return tr->loadStream(); },
		/* 13 Blob   */ [] (TR* tr) -> G* { return nullptr; }
	};

	this->generic = seq::Generic( loadFuncArr[((seq::byte) this->type) - 1]( this ) );
}

seq::Generic& seq::TokenReader::getGeneric() {
	return this->generic;
}

seq::DataType seq::TokenReader::getDataType( byte header ) {

	static seq::DataType dataTypeMap[ SEQ_MAX_OPCODE ] = {
		/* 1  BLT */ seq::DataType::Bool,
		/* 2  BLF */ seq::DataType::Bool,
		/* 3  NIL */ seq::DataType::Null,
		/* 4  NUM */ seq::DataType::Number,
		/* 5  INT */ seq::DataType::Number,
		/* 6  STR */ seq::DataType::String,
		/* 7  TYP */ seq::DataType::Type,
		/* 8  VMC */ seq::DataType::VMCall,
		/* 9  ARG */ seq::DataType::Arg,
		/* 10 FUN */ seq::DataType::Func,
		/* 11 EXP */ seq::DataType::Expr,
		/* 12 VAR */ seq::DataType::Name,
		/* 13 DEF */ seq::DataType::Name,
		/* 14 FLC */ seq::DataType::Flowc,
		/* 15 SSL */ seq::DataType::Stream,
		/* 16 FNE */ seq::DataType::Func,
		/* 17 TEX */ seq::DataType::Expr
	};

	if( header >= SEQ_MIN_OPCODE && header <= SEQ_MAX_OPCODE ) {
		return (seq::DataType) dataTypeMap[header - 1];
	}else{
		throw seq::InternalError( "Unknown head opcode! base: " + std::to_string( (int) header ) );
	}
}

seq::DataType seq::TokenReader::getDataType() {
	return this->type;
}

bool seq::TokenReader::isAnchored() {
	return this->anchor;
}

seq::type::Bool* seq::TokenReader::loadBool() {
	return new seq::type::Bool( this->anchor, this->header == (byte) seq::Opcode::BLT );
}

seq::type::Number* seq::TokenReader::loadNumber() {

	byte head = this->reader.nextByte();

	if( this->header == (byte) seq::Opcode::NUM ) {

		unsigned long n = 0, d = 0;

		// load left and right header, capped at 8 bytes
		byte a = (head >> 4) % 9;
		byte b = (head & 0b00001111) % 9;

		// load numerator
		for( byte i = 0; i < a; i ++ ) {
			n |= (( (long) this->reader.nextByte() ) << (i * 8));
		}

		// load denominator, defaults to 1
		if( b ) for( byte i = 0; i < b; i ++ ) {
			d |= (( (long) this->reader.nextByte() ) << (i * 8));
		} else d = 1;

		if( !d ) throw seq::InternalError( "Invalid denominator!" );

		// Handles negative numbers, sign holds a sign bit mask,
		// then if the mask matches with a value the number is inverted and mask is Xored off from the value,
		// otherwise nothing is done. (there is no sign bit that needs to be moved)
		// Warning: Denominator is considered unsigned
		unsigned long sign = (1ul << ((unsigned long) a * 8ul - 1ul));
		return new seq::type::Number( this->anchor, (n & sign) ? -(long)(sign ^ n) : n, d );

	}else{

		return new seq::type::Number( this->anchor, (long) (signed char) head, 1 );

	}
}

seq::type::Arg* seq::TokenReader::loadArg() {
	return new seq::type::Arg( this->anchor, this->reader.nextByte() );
}

seq::type::String* seq::TokenReader::loadString() {
	std::string str;
	this->reader.nextString(&str);
	return new seq::type::String( this->isAnchored(), str.c_str() );
}

seq::type::Type* seq::TokenReader::loadType() {
	byte b = this->reader.nextByte();
	if( b > SEQ_MAX_DATA_TYPE || b < SEQ_MIN_DATA_TYPE ) throw seq::InternalError( "Invalid data type!" );
	return new seq::type::Type( this->anchor, (seq::DataType) b );
}

seq::type::VMCall* seq::TokenReader::loadCall() {
	byte b = this->reader.nextByte();
	if( b > SEQ_MAX_CALL_TYPE && b < SEQ_MIN_CALL_TYPE ) throw seq::InternalError( "Invalid call type!" );
	return new seq::type::VMCall( this->anchor, (seq::type::VMCall::CallType) b );
}

seq::type::Name* seq::TokenReader::loadName() {
	std::string str;
	this->reader.nextString(&str);
	return new seq::type::Name( this->anchor, this->header == (byte) seq::Opcode::DEF, str );
}

seq::type::Function* seq::TokenReader::loadFunc() {
	long length = this->reader.nextUnsigned();
	if( !length ) throw seq::InternalError( "Invalid function size!" );
	return new seq::type::Function( this->anchor, this->reader.nextBlock( length ), this->header == (byte) seq::Opcode::FNE );
}

seq::type::Expression* seq::TokenReader::loadExpr() {

	if( this->reader.peekByte() < SEQ_MIN_OPERATOR || this->reader.peekByte() > SEQ_MAX_OPERATOR ) {
		throw seq::InternalError( "Invalid operator!" );
	}

	seq::ExprOperator op = (seq::ExprOperator) this->reader.nextByte();
	long l = 0, r = 0;

	byte head = this->reader.nextByte();
	byte a = (head >> 4);
	byte b = (head & 0b00001111);

	if( this->header == (byte) seq::Opcode::TEX ) {

		l = a;
		r = b;

	}else{

		a %= 9;
		b %= 9;

		for( byte i = 0; i < a; i ++ ) {
			l |= ( (long) this->reader.nextByte() ) << (i * 8);
		}

		for( byte i = 0; i < a; i ++ ) {
			r |= ( (long) this->reader.nextByte() ) << (i * 8);
		}

	}

	if( !(l && r) ) throw seq::InternalError( "Invalid expression size!" );

	seq::BufferReader* left = this->reader.nextBlock(l);
	seq::BufferReader* right = this->reader.nextBlock(r);

	return new seq::type::Expression(this->anchor, op, left, right);
}

seq::type::Flowc* seq::TokenReader::loadFlowc() {

	auto blocks = std::vector<seq::FlowCondition*>();
	byte block_count = this->reader.nextByte();
	for( byte i = 0; i < block_count; i ++ ) {

		long length = this->reader.nextUnsigned();
		if( !length ) throw seq::InternalError( "Invalid flowc size!" );
		seq::BufferReader* br = this->reader.nextBlock( length );
		seq::TokenReader tr = br->next();

		if( br->hasNext() ) {
			blocks.push_back( new seq::FlowCondition( seq::FlowCondition::Type::Range, tr.getGeneric(), br->next().getGeneric() ) );
		}else{
			if( tr.getDataType() == seq::DataType::Type ) {
				blocks.push_back( new seq::FlowCondition( seq::FlowCondition::Type::Type, tr.getGeneric(), seq::Generic() ) );
			}else{
				blocks.push_back( new seq::FlowCondition( seq::FlowCondition::Type::Value, tr.getGeneric(), seq::Generic() ) );
			}
		}

		delete br;

	}

	return new seq::type::Flowc( this->anchor, blocks );
}

seq::type::Stream* seq::TokenReader::loadStream() {
	byte tags = this->reader.nextByte();
	long length = this->reader.nextUnsigned();
	if( !length ) throw seq::InternalError( "Invalid stream size!" );
	return new seq::type::Stream( this->anchor, tags, this->reader.nextBlock( length ) );
}

seq::StackLevel::StackLevel() {
	this->arg = seq::Generic();
}

seq::StackLevel::StackLevel( seq::Generic arg ) {
	this->arg = arg;
}

seq::StackLevel::StackLevel( seq::StackLevel&& level ) {
	this->arg = std::move( level.arg );
	this->vars = std::move( level.vars );
}

seq::Generic seq::StackLevel::getArg() {
	return seq::Generic( this->arg );
}

seq::Stream seq::StackLevel::getVar( std::string& name, bool anchor ) {
	static seq::Stream ret;
	auto& vars = this->vars.at( name );
	ret.reserve( vars.size() );

	for( auto& g : vars ) {
		seq::Generic ng( g );
		ng.setAnchor( anchor );
		ret.push_back( ng );
	}

	return std::move(ret);
}

bool seq::StackLevel::hasVar( std::string& name ) {
	return this->vars.count(name) != 0;
}

void seq::StackLevel::setVar( std::string& name, seq::Stream value ) {
	this->vars[ name ] = value;
}

void seq::StackLevel::setArg( seq::Generic _arg ) {
	this->arg = _arg;
}

seq::FlowCondition::FlowCondition( seq::FlowCondition::Type _type, seq::Generic _a, seq::Generic _b ): type( _type ), a( _a ), b( _b ) {}

bool seq::FlowCondition::validate( seq::Generic arg ) {

	switch( this->type ) {
		case seq::FlowCondition::Type::Value: {
				auto type = arg.getDataType();
				if( type == this->a.getDataType() ) {
					switch( type ) {
						case seq::DataType::Null:
							return true;

						case seq::DataType::Bool:
							return this->a.Bool().getBool() == arg.Bool().getBool();

						case seq::DataType::Number:
							return this->a.Number().getDouble() == arg.Number().getDouble();

						case seq::DataType::String:
							return this->a.String().getString() == arg.String().getString();

						default:
							return false;
					}
				}
			}
			return false;

		case seq::FlowCondition::Type::Type:
			return (arg.getDataType() == this->a.Type().getType());

		case seq::FlowCondition::Type::Range:
			if( arg.getDataType() == seq::DataType::Number ) {
				double av = this->a.Number().getDouble();
				double bv = this->b.Number().getDouble();
				double val = arg.Number().getDouble();
				return val < bv && val > av;
			}
			return false;
	}

	return false;
}

seq::CommandResult::CommandResult( seq::CommandResult::ResultType _stt, seq::Stream _acc ): stt( _stt ), acc( _acc ) {}

seq::Executor::Executor( Executor* parent ) {
	this->stack.push_back( seq::StackLevel() );
	this->strictMath = false;
	this->parent = parent;
}

seq::Executor::Executor(): Executor( nullptr ) {};

void seq::Executor::inject( std::string name, seq::type::Native native ) {
	this->natives[ name ] = native;
}

void seq::Executor::define( std::string name, seq::Stream stream ) {
	this->getTopLevel()->setVar( name, stream );
}

seq::StackLevel* seq::Executor::getLevel( int level ) {
	try{
		return &(this->stack.at( level ));
	}catch( std::out_of_range& ex ) {
		return nullptr;
	}
}

seq::StackLevel* seq::Executor::getTopLevel() {
	return &(this->stack.back());
}

void seq::Executor::reset() {
	this->natives.clear();
}

std::string seq::Executor::getResultString() {
	return seq::util::stringCast( this->getResult() ).String().getString();
}

seq::Generic seq::Executor::getResult() {
	if( this->result.size() > 0 ) {
		return this->result.at(0);
	}else{
		return seq::util::newNull();
	}
}

seq::Stream& seq::Executor::getResults() {
	return this->result;
}

void seq::Executor::setStrictMath( bool flag ) {
	strictMath = flag;
}

void seq::Executor::execute( seq::ByteBuffer bb, seq::Stream args, bool stack ) {
	try{
		seq::Stream exitStream = this->executeFunction( bb.getReader(), args, true, stack );
		this->exit( exitStream, 0 );
	}catch( seq::ExecutorInterrupt& ex ) {
		// this->result set by the this->exit method
	}
}

void seq::Executor::exit( seq::Stream& stream, byte code ) {
	// stop program execution
	this->result = stream;
	throw seq::ExecutorInterrupt( code );
}

seq::Stream seq::Executor::executeFunction( seq::BufferReader fbr, seq::Stream& input_stream, bool end, bool stack ) {

	// push new stack into stack array
	if( stack ) this->stack.push_back( seq::StackLevel() );

	// accumulator of all returned entities
	seq::Stream acc;

	// turn end flag into offset
	int o = (end ? 0 : -1);

	// execute scope for each input_stream element
	for ( long i = 0; i <= (long) input_stream.size() + o; i ++ ) {

		const long size = input_stream.size();
		const byte tags = seq::util::packTags( i, size );
		seq::BufferReader br = fbr;

		// set current stack argument
		this->getTopLevel()->setArg( (i == size) ? seq::Generic( new seq::type::Null( false ) ) : input_stream[i] );

		// iterate over function code
		while( br.hasNext() ) {

			// run next command
			seq::TokenReader tk = br.next();
			seq::CommandResult cr = this->executeCommand( &tk, tags );

			// check state
			switch( cr.stt ) {

				case seq::CommandResult::ResultType::Return:
					// insert returned data to function output stream
					acc.insert(acc.end(), cr.acc.begin(), cr.acc.end());
					break;

				case seq::CommandResult::ResultType::Break:
					// exit scope
					goto exit;
					break;

				case seq::CommandResult::ResultType::Exit:
					// stop program execution (by throwing exception)
					this->exit( cr.acc, 0 );
					break;

				case seq::CommandResult::ResultType::Final:
					// exit scope and return value
					acc.insert(acc.end(), cr.acc.begin(), cr.acc.end());
					goto exit;
					break;

				case seq::CommandResult::ResultType::Again:
					// add returned arguments to CURRENT input stream
					if( i == size ) throw RuntimeError( "Native function 'again' can not be called from 'end' tagged stream!" );
					input_stream.erase( input_stream.begin(), input_stream.begin() + i + 1 );
					input_stream.insert( input_stream.begin(), cr.acc.begin(), cr.acc.end() );
					i = -1;
					break;

				default:
					break;

			}
		}
	}

	exit:

	// pop scope from stack
	if( stack ) this->stack.pop_back();

	// return all accumulated entities
	return acc;
}

seq::CommandResult seq::Executor::executeCommand( seq::TokenReader* tr, byte tags ) {

	// functions can only contain streams
	if( tr->getDataType() == seq::DataType::Stream ) {

		// execute stream if stream tags match current state
		auto& stream = tr->getGeneric().Stream();

		if( stream.matchesTags( tags ) ) {
			seq::Stream s = stream.getReader().readAll();
			return this->executeStream( s );
		}else{
			return CommandResult( seq::CommandResult::ResultType::None, seq::Stream() );
		}

	}

	throw seq::InternalError( "Invalid command in function!" );
}

seq::CommandResult seq::Executor::executeStream( seq::Stream& gs ) {

	seq::Stream acc;

	// iterate over stream entities
	for( int i = gs.size() - 1; i >= 0; i -- ) {

		seq::Generic& g = gs[i];
		seq::DataType t = g.getDataType();

		// if type is unsolid compute real value
		if( t == seq::DataType::Expr || t == seq::DataType::Arg ) {
			g = this->executeExpr( g );
			t = g.getDataType();
		}

		// handle embedded (nested) streams
		if( t == seq::DataType::Stream ) {
			Stream ss = g.Stream().getReader().readAll();
			seq::CommandResult cr = this->executeStream( ss );
			if( cr.stt != seq::CommandResult::ResultType::None ) {
				throw seq::InternalError( "Invalid result of embedded stream!" );
			}

			acc.insert( acc.begin(), cr.acc.begin(), cr.acc.end() );
			continue;
		}

		// execute anchored entities
		if( g.getAnchor() ) {

			// if acc is empty there is nothing to execute - skip
			if( acc.size() == 0 ) {
				continue;
			}

			// if entity is a VM Call
			if( t == seq::DataType::VMCall ) {

				// get VMCall type and using a hacky way cast it to ResultType, then return
				auto stt = (seq::CommandResult::ResultType) (byte) g.VMCall().getCall();
				return CommandResult( stt, acc );

			}else{

				// execute anchor and save result in acc
				seq::CommandResult cr = this->executeAnchor( g, acc );
				if( cr.stt == seq::CommandResult::ResultType::None ) {
					acc = std::move(cr.acc);
				}else{
					return cr;
				}
				continue;

			}
		}

		// if entity is a (unanchored) variable
		if( t == seq::DataType::Name ) {
			auto& name = g.Name();

			if( name.getDefine() ) { // define variable (set)

				this->defineName( name.getName(), acc );
				acc.clear();

			}else{ // read variable from stack

				auto tmp = this->resolveName( name.getName(), name.getAnchor() );

				// append tmp to acc
				acc.insert( acc.begin(), tmp.begin(), tmp.end() );
			}

			continue;
		}

		// if entity is a simple, solid value add it to acc
		acc.insert( acc.begin(), g );
	}

	return CommandResult( seq::CommandResult::ResultType::None, std::move(acc) );
}

seq::CommandResult seq::Executor::executeAnchor( seq::Generic entity, seq::Stream& input_stream ) {

	seq::DataType type = entity.getDataType();

	// if given entity is a variable
	if( type == seq::DataType::Name ) {

		seq::type::Name& name = entity.Name();

		// test if name refers to native function, and if so execute it
		try{
			// this will throw std::out_of_range if native is not found
			seq::Stream* ptr = resolveNative( name.getName() )( &input_stream );

			// If null pointer is returned the input_stream is to be treated as output
			if( ptr != nullptr ) {
				input_stream = std::move( *ptr );
				delete ptr;
			}

			return CommandResult( seq::CommandResult::ResultType::None, input_stream );
		} catch (std::out_of_range &ignore) {

			// if it isn't native, try finding it on the stack
			seq::Stream s = this->resolveName( name.getName(), true );
			s.insert( s.end(), input_stream.begin(), input_stream.end() );
			return this->executeStream( s );

		}

	}

	// execute anchored function
	if( type == seq::DataType::Func ) {
		auto& func = entity.Function();
		return CommandResult( seq::CommandResult::ResultType::None, this->executeFunction( func.getReader(), input_stream, func.hasEnd() ) );
	}

	// execute anchored flowc
	if( type == seq::DataType::Flowc ) {
		return CommandResult( seq::CommandResult::ResultType::None, this->executeFlowc( entity.Flowc().getConditions(), input_stream ) );
	}

	if( type == seq::DataType::Blob ) {
		throw seq::RuntimeError( "Invalid '" + entity.Blob().toString() + "' call!" );
	}

	// if not any of the above, simply cast args to anchor
	seq::Stream output_stream;
	for( auto& g : input_stream ) {
		output_stream.push_back( this->executeCast( entity, g ) );
	}
	return CommandResult( seq::CommandResult::ResultType::None, std::move(output_stream) );

}

seq::Generic seq::Executor::executeExprPair( seq::Generic left, seq::Generic right, seq::ExprOperator op, bool anchor ) {

	{
		const seq::DataType ltype = left.getDataType();
		const seq::DataType rtype = right.getDataType();

		// if left or right side is and expr or arg pass it through executeExpr
		if( ltype == seq::DataType::Expr || ltype == seq::DataType::Arg ) left = this->executeExpr(left);
		if( rtype == seq::DataType::Expr || rtype == seq::DataType::Arg ) right = this->executeExpr(right);
	}

	const seq::DataType ltype = left.getDataType();
	const seq::DataType rtype = right.getDataType();

	// handle accessor operator
	if( op == seq::ExprOperator::Accessor ) {

		if( ltype != seq::DataType::Name || rtype != seq::DataType::Number ) {
			throw seq::RuntimeError( "Invalid accessor operands, stream and number expected!" );
		}

		// try returning element from stream (left) at index (right)
		try{
			const Stream stream = std::move( this->resolveName( left.Name().getName(), anchor ) );
			return std::move( stream.at( right.Number().getLong() ) );
		}catch( std::out_of_range& err ){
			return seq::util::newNull(anchor);
		}

	}

	// return null if data types don't match (with exception of Not and BinaryNot)
	if( rtype != ltype ) {
		if( op != seq::ExprOperator::Not && op != seq::ExprOperator::BinaryNot ) {

			if( this->strictMath ) {
				throw seq::RuntimeError( "Expression operators don't match!" );
			}else{
				return seq::util::newNull(anchor);
			}

		}
	}

	typedef seq::type::Generic*(*ExprFunc)( bool, seq::type::Generic*, seq::type::Generic* );
	typedef seq::type::Generic*(*TypeFunc)( bool, seq::type::Generic*, seq::type::Generic*, byte op );

#	define SQEFN [] ( bool f, seq::type::Generic* a, seq::type::Generic* b ) -> seq::type::Generic*
#	define SQTFN [] ( bool f, seq::type::Generic* a, seq::type::Generic* b, byte op ) -> seq::type::Generic*
#	define SQNML( g ) ((seq::type::Number*) g)->getLong()
#	define SQNMD( g ) ((seq::type::Number*) g)->getDouble()
#	define SQSTR( g ) ((seq::type::String*) g)->getString()
#	define SQBOL( g ) ((seq::type::Bool*) g)->getBool()
#	define SQTYP( g ) ((seq::type::Type*) g)->getType()

	static const ExprFunc null_expr_func = SQEFN { return new seq::type::Null(f); };
	static const TypeFunc null_type_func = SQTFN { return new seq::type::Null(f); };

	static const ExprFunc expr_lambdas[SEQ_MAX_OPERATOR + 1][3] = {
		{ // Padding
			nullptr,
			nullptr,
			nullptr
		},
		{ // Less
			SQEFN { return new seq::type::Bool(f, SQNMD(a) < SQNMD(b)); },
			SQEFN { return new seq::type::Bool(f, SQBOL(a) < SQBOL(b)); },
			null_expr_func,
		},
		{ // Greater
			SQEFN { return new seq::type::Bool(f, SQNMD(a) > SQNMD(b)); },
			SQEFN { return new seq::type::Bool(f, SQBOL(a) > SQBOL(b)); },
			null_expr_func,
		},
		{ // Equal
			SQEFN { return new seq::type::Bool(f, SQNMD(a) == SQNMD(b)); },
			SQEFN { return new seq::type::Bool(f, SQBOL(a) == SQBOL(b)); },
			SQEFN { return new seq::type::Bool(f, SQSTR(a) == SQSTR(b)); },
		},
		{ // NotEqual
			SQEFN { return new seq::type::Bool(f, SQNMD(a) != SQNMD(b)); },
			SQEFN { return new seq::type::Bool(f, SQBOL(a) != SQBOL(b)); },
			SQEFN { return new seq::type::Bool(f, SQSTR(a) != SQSTR(b)); },
		},
		{ // NotGreater
			SQEFN { return new seq::type::Bool(f, SQNMD(a) <= SQNMD(b)); },
			SQEFN { return new seq::type::Bool(f, SQBOL(a) <= SQBOL(b)); },
			null_expr_func,
		},
		{ // NotLess
			SQEFN { return new seq::type::Bool(f, SQNMD(a) >= SQNMD(b)); },
			SQEFN { return new seq::type::Bool(f, SQBOL(a) >= SQBOL(b)); },
			null_expr_func,
		},
		{ // And
			SQEFN { return new seq::type::Bool(f, SQNML(a) != 0 && SQNML(b) != 0); },
			SQEFN { return new seq::type::Bool(f, SQBOL(a) && SQBOL(b)); },
			null_expr_func,
		},
		{ // Or
			SQEFN { return new seq::type::Bool(f, SQNML(a) != 0 || SQNML(b) != 0); },
			SQEFN { return new seq::type::Bool(f, SQBOL(a) || SQBOL(b)); },
			null_expr_func,
		},
		{ // Xor
			SQEFN { return new seq::type::Bool(f, (SQNML(a) != 0) != (SQNML(b) != 0)); },
			SQEFN { return new seq::type::Bool(f, SQBOL(a) != SQBOL(b)); },
			null_expr_func,
		},
		{ // Not
			SQEFN { return new seq::type::Bool(f, SQNML(b) == 0); },
			SQEFN { return new seq::type::Bool(f, !SQBOL(b)); },
			null_expr_func,
		},
		{ // Multiplication
			SQEFN { return new seq::type::Number(f, SQNMD(a) * SQNMD(b)); },
			SQEFN { return new seq::type::Bool(f, SQBOL(a) < SQBOL(b)); },
			null_expr_func,
		},
		{ // Division
			SQEFN { return new seq::type::Number(f, SQNMD(a) / SQNMD(b)); },
			SQEFN { return new seq::type::Bool(f, SQBOL(a) < SQBOL(b)); },
			null_expr_func,
		},
		{ // Addition
			SQEFN { return new seq::type::Number(f, SQNMD(a) + SQNMD(b)); },
			SQEFN { return new seq::type::Bool(f, SQBOL(a) || SQBOL(b)); },
			SQEFN { return new seq::type::String(f, (SQSTR(a) + SQSTR(b)).c_str() ); },
		},
		{ // Subtraction
			SQEFN { return new seq::type::Number(f, SQNMD(a) - SQNMD(b)); },
			SQEFN { return new seq::type::Bool(f, SQBOL(a) != SQBOL(b)); },
			null_expr_func,
		},
		{ // Modulo
			SQEFN { return new seq::type::Number(f, SQNML(a) % SQNML(b)); },
			null_expr_func,
			null_expr_func,
		},
		{ // Power
			SQEFN { return new seq::type::Number(f, std::pow( SQNMD(a), SQNMD(b) )); },
			null_expr_func,
			null_expr_func,
		},
		{ // BinaryAnd
			SQEFN { return new seq::type::Number(f, SQNML(a) & SQNML(b) ); },
			SQEFN { return new seq::type::Bool(f, SQBOL(a) && SQBOL(b)); },
			null_expr_func,
		},
		{ // BinaryOr
			SQEFN { return new seq::type::Number(f, SQNML(a) | SQNML(b) ); },
			SQEFN { return new seq::type::Bool(f, SQBOL(a) || SQBOL(b)); },
			null_expr_func,
		},
		{ // BinaryXor
			SQEFN { return new seq::type::Number(f, SQNML(a) ^ SQNML(b) ); },
			SQEFN { return new seq::type::Bool(f, SQBOL(a) != SQBOL(b)); },
			null_expr_func,
		},
		{ // BinaryNot
			SQEFN { return new seq::type::Number(f, ~ SQNML(b) ); },
			SQEFN { return new seq::type::Bool(f, !SQBOL(b)); },
			null_expr_func,
		},
		{ // Accessor (handled in different place)
			null_expr_func,
			null_expr_func,
			null_expr_func,
		}
	};

	static const TypeFunc simple_null_type_func = SQTFN {
		if( op == (byte) seq::ExprOperator::Equal ) return new seq::type::Bool( f, true );
		if( op == (byte) seq::ExprOperator::NotEqual ) return new seq::type::Bool( f, false );
		return new seq::type::Null( f );
	};

	static const TypeFunc simple_type_type_func = SQTFN {
		if( op == (byte) seq::ExprOperator::Equal ) return new seq::type::Bool( f, SQTYP(a) == SQTYP(b) );
		if( op == (byte) seq::ExprOperator::NotEqual ) return new seq::type::Bool( f, SQTYP(a) != SQTYP(b) );
		return new seq::type::Null( f );
	};

	static const TypeFunc type_lambdas[SEQ_MAX_DATA_TYPE + 1] = {
		/* 0 Padding */ nullptr,
		/* 1  Bool   */ SQTFN { return expr_lambdas[op][1]( f, a, b ); },
		/* 2  Null   */ simple_null_type_func,
		/* 3  Number */ SQTFN { return expr_lambdas[op][0]( f, a, b ); },
		/* 4  String */ SQTFN { return expr_lambdas[op][2]( f, a, b ); },
		/* 5  Type   */ simple_type_type_func,
		/* 6  VMCall */ null_type_func,
		/* 7  Arg    */ null_type_func,
		/* 8  Func   */ null_type_func,
		/* 9  Expr   */ null_type_func,
		/* 10 Name   */ null_type_func,
		/* 11 Flowc  */ null_type_func,
		/* 12 Stream */ null_type_func,
		/* 13 Blob   */ null_type_func,
	};

#	undef SQEFN
#	undef SQTFN
#	undef SQNML
#	undef SQNMD
#	undef SQSTR
#	undef SQBOL
#	undef SQTYP

	return seq::Generic( type_lambdas[ (byte) rtype ]( anchor, left.getRaw(), right.getRaw(), (byte) op ) );

}

seq::Generic seq::Executor::executeExpr( seq::Generic entity ) {

	// get entity properties
	seq::DataType type = entity.getDataType();
	bool anchor = entity.getAnchor();

	// if given entity is an expression pass it to executeExprPair
	if( type == seq::DataType::Expr ) {
		auto& expr = entity.Expression();

		seq::Generic left = expr.getLeftReader().next().getGeneric();
		seq::Generic right = expr.getRightReader().next().getGeneric();
		seq::ExprOperator op = expr.getOperator();

		return this->executeExprPair( std::move(left), std::move(right), op, anchor );
	}

	// if it is simple argument resolve it's value
	if( type == seq::DataType::Arg ) {
		auto& arg = entity.Arg();

		// get stack level to which the argument is referring
		long s = (long) this->stack.size() - 1L - (long) arg.getLevel();

		// according to Sequensa specification
		// if argument reference is invalid null should be returned instead
		if( s < 0 ) {
			return seq::util::newNull( anchor );
		}

		// return stack argument
		auto ret = std::move( this->stack[s].getArg() );
		ret.setAnchor(anchor);
		return ret;
	}

	// else: return unchanged
	return entity;

}

seq::Stream seq::Executor::resolveName( std::string& name, bool anchor ) {

	// iterate stack levels in search of the specified variable
	for( int i = (int) this->stack.size() - 1; i >= 0; i -- ) {

		try{
			return std::move( this->stack.at(i).getVar( name, anchor ) );
		} catch (std::out_of_range &err) {
			continue;
		}

	}

	// if executor has a parent, ask him
	if( parent != nullptr ) {
		return parent->resolveName( name, anchor );
	}

	// if symbol wasn't found throw runtime exception
	throw seq::RuntimeError( "Referenced undefined symbol: '" + name + "'" );

}

void seq::Executor::defineName( std::string& name, Stream& value, bool define ) {

	// iterate stack levels in search of the specified variable
	for( int i = (int) this->stack.size() - 1; i >= 0; i -- ) {

		try{

			auto& level = this->stack.at(i);

			// when it's found modify current value
			if( level.hasVar( name ) ) {
				level.setVar( name, value );
				return;
			}

		} catch (std::out_of_range &err) {
			continue;
		}

	}

	// if executor has a parent, ask him
	if( parent != nullptr ) {
		return parent->defineName( name, value, false );
	}

	// if symbol wasn't found create new variable in top stack level
	if( define ) {
		getTopLevel()->setVar(name, value);
	}

}

seq::type::Native seq::Executor::resolveNative( std::string& name ) {

	try{

		return this->natives.at( name );

	}catch(std::out_of_range &err) {

		if( parent != nullptr ) {

			return parent->resolveNative( name );

		}

		throw err;

	}

}

std::unordered_map<std::string, seq::type::Native>& seq::Executor::getNativesMap() {

	return this->natives;

}

seq::Stream seq::Executor::executeFlowc( std::vector<seq::FlowCondition*> fcs, seq::Stream& input_stream ) {

	seq::Stream acc;

	for( seq::Generic& arg : input_stream ) {

		// check if arg satisfies flowc conditions
		for( seq::FlowCondition* fc : fcs ) {
			if( fc->validate( arg ) ) {
				acc.push_back( arg );
				break;
			}
		}

	}

	// return matching entities
	return acc;
}

seq::Generic seq::Executor::executeCast( seq::Generic cast, seq::Generic arg ) {

	switch( cast.getDataType() ) {

		// simple value casts:
		case seq::DataType::Bool:
		case seq::DataType::Null:
		case seq::DataType::Number:
		case seq::DataType::String:
		case seq::DataType::VMCall:
		case seq::DataType::Flowc:
		case seq::DataType::Func:
			cast.setAnchor(false);
			return cast;

		// type cast:
		case seq::DataType::Type: {
			switch( cast.Type().getType() ) {

				case seq::DataType::Type:
					return seq::Generic( new seq::type::Type( false, arg.getDataType() ) );

				case seq::DataType::Bool:
					return seq::util::boolCast( arg );

				case seq::DataType::Number:
					return seq::util::numberCast( arg );

				case seq::DataType::String:
					return seq::util::stringCast( arg );

				default: throw seq::InternalError( "Invalid cast!" );
			}
		}

		// invalid casts: (stream, name, expr, arg)
		default: throw seq::InternalError( "Invalid cast!" );

	}

}

#ifndef SEQ_EXCLUDE_COMPILER
seq::Compiler::Token::Token( unsigned int _line, long _data, bool _anchor, Category _category, std::string& _raw, std::string& _clean ): line( _line ), data( _data ), anchor( _anchor ),  category( _category ), raw( _raw ), clean( _clean ) {}

seq::Compiler::Token::Token( const Token& token ): line( token.line ), data( token.data ), anchor( token.anchor ), category( token.category ), raw( token.raw ), clean( token.clean ) {}

seq::Compiler::Token::Token( seq::Compiler::Token&& token ): line( std::move( token.line ) ), data( std::move( token.data ) ), anchor( std::move( token.anchor ) ), category( std::move( token.category ) ), raw( std::move( token.raw ) ), clean( std::move( token.clean ) ) {}

const unsigned int seq::Compiler::Token::getLine() {
	return this->line;
}

const seq::Compiler::Token::Category seq::Compiler::Token::getCategory() {
	return this->category;
}

const std::string& seq::Compiler::Token::getRaw() {
	return this->raw;
}

const std::string& seq::Compiler::Token::getClean() {
	return this->clean;
}

const long seq::Compiler::Token::getData() {
	return this->data;
}

const bool seq::Compiler::Token::getAnchor() {
	return this->anchor;
}

const bool seq::Compiler::Token::isPrimitive() {
	using Category = seq::Compiler::Token::Category;
	return category == Category::Arg || category == Category::Null || category == Category::Bool || category == Category::Number ||
			category == Category::Type || category == Category::String || category == Category::VMCall;
}

const bool seq::Compiler::Token::isPure() {
	using Category = seq::Compiler::Token::Category;
	if( category == Category::Arg || category == Category::VMCall ) return false;
	return this->isPrimitive();
}

const bool seq::Compiler::Token::isNamed() {
	using Category = seq::Compiler::Token::Category;
	return category == Category::String || category == Category::Name;
}

std::string seq::Compiler::Token::toString() {
	const char* ccatstr[]{
		"Tag",
		"Set",
		"Load",
		"Bool",
		"Null",
		"Type",
		"Name",
		"Number",
		"Stream",
		"Operator",
		"String",
		"FuncBracket",
		"FlowBracket",
		"MathBracket",
		"Comma",
		"Colon",
		"Arg",
		"VMCall"
	};
	std::string catstr(ccatstr[(byte)getCategory()]);
	return catstr + "::" + this->getRaw();
}

seq::Compiler::Compiler() {
	handle = seq::Compiler::defaultErrorHandle;
	loades = nullptr;
	names = nullptr;
	flags = (oflag_t) Optimizations::None;
}

std::vector<byte> seq::Compiler::compile( std::string code ) {

	// tokenize the program
	auto tokens = tokenize( code );

	// perform some optimizations if they are enabled
	optimizeIfApplicable( tokens );

	// skip empty files
	if( tokens.empty() ) return std::vector<byte>();

	// compute load section size and store loads
	int offset = extractHeaderData( tokens, loades );

	// assemble bytecode
	return assembleFunction( tokens, offset, tokens.size(), true, true );

}

std::vector<byte> seq::Compiler::compileStatic( std::string code, StringTable* headerData, oflag_t flags ) {
	Compiler compiler;
	compiler.setLoadTable( headerData );
	compiler.setOptimizationFlags( flags );
	return compiler.compile(code);
}

std::vector<seq::Compiler::Token> seq::Compiler::tokenizeStatic( std::string code ) {
	return Compiler().tokenize( code );
}

seq::Compiler obj() {
	return seq::Compiler();
}

std::vector<seq::Compiler::Token> seq::Compiler::tokenize( std::string code ) {

	// define internal struct
	enum struct State {
		Start,
		Comment,
		String,
		Escape,
		Name,
		Number,
		Number2,
		NumberSign,
		Arg
	};

	// init stuff
	const static std::vector<std::string> long_operators = { "!=", ">=", "!>", "<=", "!<", "&&", "||", "^^", "**" };
	const static std::vector<char> short_operators = { '+', '-', '/', '%', '*', '>', '<', '=', '&', '|', '^', '~' };
	const static std::vector<std::string> brackets = { "{ ", "} ", "#{", "[ ", "] ", "#[", "( ", ") ", "#(" };

	State state = State::Start;
	std::string token;
	std::vector<seq::Compiler::Token> tokens;
	unsigned int line = 1;
	int roundBrackets = 0;
	int curlyBrackets = 0;
	int squareBrackets = 0;

	// function used to add token to token array based on tokenizer internal state
	auto next = [&] () -> void {
		if( !token.empty() ) {
			tokens.push_back( seq::Compiler::construct( token , line ) );
			token = "";
		}
	};

	// function used to create string from two chars, used for checking long operators
	auto bind = [] (char a, char b) -> std::string {
		std::string str;
		str += a;
		str += b;
		return str;
	};

	// brackets state checker
	auto updateBrackets = [&] (char chr) {
		switch( chr ) {
			case '{': curlyBrackets ++; break;
			case '}': curlyBrackets --; break;
			case '[': squareBrackets ++; break;
			case ']': squareBrackets --; break;
			case '(': roundBrackets ++; break;
			case ')': roundBrackets --; break;
			default: throw seq::InternalError( "Invalid lambda argument!" );
		}

		// bracket count cannot be negative
		if( curlyBrackets < 0 ) fail( seq::CompilerError( 1, "'}'", "", "", line ) );
		if( roundBrackets < 0 ) fail( seq::CompilerError( 1, "')'", "", "", line ) );

		// square brackets cannot be nested
		if( squareBrackets != 0 && squareBrackets != 1 ) {
			std::string msg = "'";
			msg += (char) chr;
			msg += '\'';

			fail( seq::CompilerError( 1, msg, "", "", line ) );
		}

	};

	const int size = code.size();
	for( int i = 0; i < size; i ++ ) {

		const char c = code[i];
		const char n = i + 1 < size ? code[i+1] : '\0';

		// keep line number up-to-date
		if( c == '\n' ) {
			if( state == State::Comment ) state = State::Start;
			if( state == State::String ) fail( CompilerError( 1, "end of line", "end of string", "", line ) );
			next();
			line ++;
			continue;
		}

		for( bool flag = true; flag; ) {

			flag = false;

			switch( state ) {

				case State::Start: {
					if( !token.empty() ) throw seq::InternalError( "Invalid tokenizer state!" );
					if( c == '/' && n == '/' ) { i ++; state = State::Comment; break; }
					if( c == '#' && n == '"' ) { state = State::String; token += "#\""; i ++; break; }
					if( c == '"' ) { state = State::String; token += c; break; }
					if( c == ' ' || c == '\n' || c == '\t' ) { break; }
					if( (c == '#' && (std::isalpha(n) || n == '_')) || (std::isalpha(c) || c == '_') ) { state = State::Name; token += c; break; }
					if( c == '<' && n == '<' ) { token += "<<"; i ++; next(); break; }
					if( (c == '#' && std::isdigit(n)) || std::isdigit(c) ) { state = State::Number; token += c; break; }
					if( (c == '-' && std::isdigit(n)) || (c == '#' && n == '-') ) { state = State::NumberSign; token += c; break; }
					if( std::find(long_operators.begin(), long_operators.end(), bind( c, n )) != long_operators.end() ) { token += c; token += n; i ++; next(); break; }
					if( std::find(short_operators.begin(), short_operators.end(), c) != short_operators.end() ) { token += c; next(); break; }
					if( c == '!' ) { token += "null"; next(); token += c; next(); break; }
					if( (c == '#' && n == '@') || c == '@' ) { state = State::Arg; token += c; break; }
					if( std::find(brackets.begin(), brackets.end(), bind( c, ' ' ) ) != brackets.end() ) { token += c; updateBrackets( c ); next(); break; }
					if( std::find(brackets.begin(), brackets.end(), bind( c, n ) ) != brackets.end() ) { token += c; token += n; updateBrackets( n ); i ++; next(); break; }
					if( c == ':' && n == ':' ) { token += "::"; i ++; next(); break; }
					if( c == ',' || c == ':' ) { token += c; next(); break; }

					std::string msg = "char: '";
					msg += (char) c;
					msg += '\'';
					fail( CompilerError( 1, msg, "", "", line ) );
					break;
				}

				case State::Comment:
					if( (c == '/' && n == '/') || c == '\n' ) {
						i ++;
						state = State::Start;
					}
					break;

				case State::String:
					if( c == '\\' ) {
						state = State::Escape;
					}else if( c == '"' ) {
						token += '"';
						state = State::Start;
						next();
					}else{
						token += c;
					}
					break;

				case State::Escape:
					switch( c ) {
						case 'e': token += '\e'; break;
						case 'n': token += '\n'; break;
						case 't': token += '\t'; break;
						case 'r': token += '\r'; break;
						case '\\': token += '\\'; break;
						case '"': token += '"'; break;
						default: fail( CompilerError( 1, std::string("char '") + (char) c + std::string("'"), "escape code (n, t, r, \\ or \")", "string", line ) );
					}
					state = State::String;
					break;

				case State::Name: // or Tag
					if( std::isalpha(c) || c == '_' ) token += c; else {

						if( c == ':' ) {
							if( std::isalpha(n) || n == '_' ) {
								token += c;
							}else{
								state = State::Start;
								flag = true;
								next();
							}
							break;
						}

						if( c == ';' ) {
							token += c;
						}else{
							flag = true;
						}

						state = State::Start;
						next();
					}
					break;

				case State::Number:
					if( std::isdigit(c) ) token += c; else {
						if( c == '.' ) {
							token += c;
							state = State::Number2;
						} else {
							state = State::Start;
							flag = true;
							next();
						}
					}
					break;

				case State::Number2:
					if( std::isdigit(c) ) token += c; else {
						state = State::Start;
						flag = true;
						next();
					}
					break;

				case State::NumberSign:
					if( c == '-' ) token += c; else {
						state = State::Number;
						flag = true;
					}
					break;

				case State::Arg:
					if( c == '@' ) token += c; else {
						state = State::Start;
						flag = true;
						next();
					}
					break;

			}

		}

	}

	// check brackets state
	if( state == State::String ) fail( CompilerError( 1, "end of input", "end of string", "", line ) );
	if( curlyBrackets != 0 ) fail( CompilerError( 1, "end of input", "curly bracket", "", line ) );
	if( roundBrackets != 0 ) fail( CompilerError( 1, "end of input", "round bracket", "", line ) );
	if( squareBrackets != 0 ) fail( CompilerError( 1, "end of input", "square bracket", "", line ) );

	// if some token is still left, add it.
	next();
	return tokens;
}

seq::Compiler::Token seq::Compiler::construct( std::string raw, unsigned int line ) {

	// setup regex'es and string vectors
	// bigger operator weight -> evaluated first
	const static std::vector<std::string> operators = { "+", "-", "/", "*", "**", "%", ">", "<", "=", "!=", ">=", "!>", "<=", "!<", "&&", "||", "^^", "&", "|", "^", "!", "::" };
	const static std::vector<byte> operator_weights = {  17,  16,  14,  15,   13,  14,  16,  16,  16,   16,   16,   16,   16,   16,   17,   17,   17,  15,  15,  15,  13,   12 };
	const static std::regex regex_arg("^@+$");
	const static std::regex regex_name("^[a-zA-Z_]{1}[a-zA-Z_0-9]*(:[a-zA-Z_0-9]+)*$");
	const static std::regex regex_number_1("^-{0,1}\\d+.\\d+$");
	const static std::regex regex_number_2("^-{0,1}\\d+$");

	const bool anchor = raw.front() == '#';
	std::string clean = anchor ? raw.substr(1) : raw;

	auto make = [&] ( seq::Compiler::Token::Category c, long data ) -> seq::Compiler::Token {
		return seq::Compiler::Token( line, data, anchor, c, raw, clean );
	};

	auto callTypeFromString = [] ( std::string str ) -> long {
		if( str == "return" ) return (byte) seq::type::VMCall::CallType::Return;
		if( str == "break" ) return (byte) seq::type::VMCall::CallType::Break;
		if( str == "exit" ) return (byte) seq::type::VMCall::CallType::Exit;
		if( str == "again" ) return (byte) seq::type::VMCall::CallType::Again;
		if( str == "final" ) return (byte) seq::type::VMCall::CallType::Final;
		return 0;
	};

	auto dataTypeFromString = [] ( std::string str ) -> long {
		if( str == "number" ) return (byte) seq::DataType::Number;
		if( str == "bool" ) return (byte) seq::DataType::Bool;
		if( str == "string" ) return (byte) seq::DataType::String;
		if( str == "type" ) return (byte) seq::DataType::Type;
		return 0;
	};

	auto tagTypeFromString = [] ( std::string str ) -> long {
		if( str == "first;" ) return SEQ_TAG_FIRST;
		if( str == "last;" ) return SEQ_TAG_LAST;
		if( str == "end;" ) return SEQ_TAG_END;
		return 0;
	};

	auto operatorDataFromString = [] ( std::string str, unsigned int weight ) -> long {
		seq::ExprOperator op;
		if( str == "<" ) op = seq::ExprOperator::Less; else
		if( str == ">" ) op = seq::ExprOperator::Greater; else
		if( str == "=" ) op = seq::ExprOperator::Equal; else
		if( str == "!=" ) op = seq::ExprOperator::NotEqual; else
		if( str == "!>" ) op = seq::ExprOperator::NotGreater; else
		if( str == "<=" ) op = seq::ExprOperator::NotGreater; else
		if( str == "!<" ) op = seq::ExprOperator::NotLess; else
		if( str == ">=" ) op = seq::ExprOperator::NotLess; else
		if( str == "&&" ) op = seq::ExprOperator::And; else
		if( str == "||" ) op = seq::ExprOperator::Or; else
		if( str == "^^" ) op = seq::ExprOperator::Xor; else
		if( str == "!" ) op = seq::ExprOperator::Not; else
		if( str == "*" ) op = seq::ExprOperator::Multiplication; else
		if( str == "/" ) op = seq::ExprOperator::Division; else
		if( str == "+" ) op = seq::ExprOperator::Addition; else
		if( str == "-" ) op = seq::ExprOperator::Subtraction; else
		if( str == "%" ) op = seq::ExprOperator::Modulo; else
		if( str == "**" ) op = seq::ExprOperator::Power; else
		if( str == "&" ) op = seq::ExprOperator::BinaryAnd; else
		if( str == "|" ) op = seq::ExprOperator::BinaryOr; else
		if( str == "^" ) op = seq::ExprOperator::BinaryXor; else
		if( str == "~" ) op = seq::ExprOperator::BinaryNot; else
		if( str == "::" ) op = seq::ExprOperator::Accessor; else
			throw seq::InternalError( "Invalid state!" );

		long operatorCode = (long) op;
		return weight | (operatorCode << 8);
	};


	{ // categorize and create new token
		auto callType = callTypeFromString( clean );
		if( callType != 0 ) return make( seq::Compiler::Token::Category::VMCall, callType );

		auto dataType = dataTypeFromString( clean );
		if( dataType != 0 ) return make( seq::Compiler::Token::Category::Type, dataType );

		auto tagType = tagTypeFromString( raw );
		if( tagType != 0 ) return make( seq::Compiler::Token::Category::Tag, tagType );

		if( raw == "set" ) return make( seq::Compiler::Token::Category::Set, 0 );
		if( raw == "load" ) return make( seq::Compiler::Token::Category::Load, 0 );
		if( clean == "true" || clean == "false" ) return make( seq::Compiler::Token::Category::Bool, clean == "true" ? 1 : 0 );
		if( clean == "null" ) return make( seq::Compiler::Token::Category::Null, 0 );
		if( clean == "number" || clean == "bool" || clean == "string" || clean == "type" ) return make( seq::Compiler::Token::Category::Type, dataTypeFromString( clean ) );
		if( std::regex_search( clean, regex_name) ) return make( seq::Compiler::Token::Category::Name, 0 );
		if( std::regex_search( clean, regex_number_1) || std::regex_search(clean, regex_number_2) ) return make( seq::Compiler::Token::Category::Number, 0 );
		if( raw == "<<" ) return make( seq::Compiler::Token::Category::Stream, 0 );
		if( clean == "{" || raw == "}" ) return make( seq::Compiler::Token::Category::FuncBracket, clean == "{" ? 1 : -1 );
		if( clean == "[" || raw == "]" ) return make( seq::Compiler::Token::Category::FlowBracket, clean == "[" ? 1 : -1 );
		if( clean == "(" || raw == ")" ) return make( seq::Compiler::Token::Category::MathBracket, clean == "(" ? 1 : -1 );
		if( raw == "," ) return make( seq::Compiler::Token::Category::Comma, 0 );
		if( raw == ":" ) return make( seq::Compiler::Token::Category::Colon, 0 );
		if( std::regex_search( clean, regex_arg) ) return make( seq::Compiler::Token::Category::Arg, clean.size() - 1 );

		if( clean.front() == '"' && clean.back() == '"' ) {
			clean = clean.substr(1, clean.size() - 2);
			return make( seq::Compiler::Token::Category::String, 0 );
		}

		auto it = std::find( operators.begin(), operators.end(), raw );
		if( it != operators.end() ) {
			return make( seq::Compiler::Token::Category::Operator, operatorDataFromString( raw, operator_weights.at(it - operators.begin()) ) );
		}
	}

	fail( seq::CompilerError( 2, "token: " + raw, "", "", line ) );
	throw seq::InternalError( "Critical error ignored!" );
}

int seq::Compiler::findStreamEnd( std::vector<seq::Compiler::Token>& tokens, int start, int end ) {
	int a = 0; // func_bracket counter
	int b = 0; // flow_bracket counter
	int c = 0; // math_bracket counter
	int l = 0; // current line

	try{
		l = tokens.at( start ).getLine();
	}catch( std::out_of_range& err ) {
		throw seq::InternalError( "Invalid start index!" );
	}

	try{

		for( int index = start; index < end; index ++ ) {
			seq::Compiler::Token& token = tokens[index];

			if( (int) token.getLine() != l ) {

				if( a == 0 && b == 0 && c == 0 ) {
					return index - 1;
				}

				l = token.getLine();

			}

			if( index + 1 >= end ) {
				if( a == 0 && b == 0 && c == 0 ) {
					return index;
				}
			}

			if( token.getCategory() == seq::Compiler::Token::Category::FuncBracket ) a += ( token.getClean().at(0) == (byte) '}' ) ? -1 : 1;
			if( token.getCategory() == seq::Compiler::Token::Category::FlowBracket ) a += ( token.getClean().at(0) == (byte) ']' ) ? -1 : 1;
			if( token.getCategory() == seq::Compiler::Token::Category::MathBracket ) a += ( token.getClean().at(0) == (byte) ')' ) ? -1 : 1;

		}

	}catch( std::out_of_range& err ) {
		throw seq::InternalError( "Invalid end index!" );
	}

	return ( a == 0 && b == 0 && c == 0 ) ? end - 1 : -1;
}

int seq::Compiler::findOpening( std::vector<seq::Compiler::Token>& tokens, int index, seq::Compiler::Token::Category type ) {
	index --;

	try {
		for( int i = -1; i != 0; index -- ) {
			auto& token = tokens.at(index);
			if( token.getCategory() == type ) i += token.getData();
		}
	}catch( std::out_of_range& err ) {
		throw seq::InternalError( "No opening token found!" );
	}

	return index;
}

int seq::Compiler::findClosing( std::vector<seq::Compiler::Token>& tokens, int index, seq::Compiler::Token::Category type ) {
	index ++;

	try {
		for( int i = 1; i != 0; index ++ ) {
			auto& token = tokens.at(index);
			if( token.getCategory() == type ) i += token.getData();
		}
	}catch( std::out_of_range& err ) {
		throw seq::InternalError( "No closing token found!" );
	}

	return index;
}

std::vector<byte> seq::Compiler::assembleStream( std::vector<seq::Compiler::Token>& tokens, int start, int end, byte tags, bool embedded ) {

	enum struct State: byte {
		Start,
		Continue,
		Set,
		Function,
		Expression,
		Flowc,
		Stream
	};

	std::vector<byte> arr;
	seq::BufferWriter bw( arr, getTable() );

	State state = State::Start;
	int statmentCounter = 0;
	bool dangling = false;

	// warning
	if( start <= end && !tokens[start].getAnchor() ) {
		warn( seq::CompilerError( "Dangling statement", "stream", tokens[start].getLine() ) );
		dangling = true;
	}

	for( int i = start; i <= end; i ++ ) {

		auto& token = tokens[i];

		switch( state ) {

			case State::Start:
				if( token.getCategory() == seq::Compiler::Token::Category::Set ) {
					state = State::Set;
					break;
				}
				// no break //

			case State::Continue:
				if( token.getCategory() == seq::Compiler::Token::Category::Name ) {
					bw.putName( token.getAnchor(), false, token.getClean().c_str() );
					state = State::Stream;
					break;
				}

				if( token.isPrimitive() ) {
					if( token.getCategory() == Compiler::Token::Category::VMCall ) {
						if( embedded ) {
							fail( seq::CompilerError( 1, "Build in native function '" + std::string( (char*) token.getClean().c_str() ) + "'", "", "embedded stream", token.getLine() ) );
						}else if( !dangling ){

							// warning
							if( statmentCounter >= 1 ) {
								warn( seq::CompilerError( "Unreachable statement", "stream", token.getLine() ) );
							}

						}
					}

					statmentCounter ++;

					auto buf = assemblePrimitive( token );
					bw.putBuffer( buf );
					state = State::Stream;
					break;
				}

				if( token.getCategory() == seq::Compiler::Token::Category::FuncBracket ) {
					state = State::Function;
					break;
				}

				if( token.getCategory() == seq::Compiler::Token::Category::MathBracket ) {
					state = State::Expression;
					break;
				}

				if( token.getCategory() == seq::Compiler::Token::Category::FlowBracket ) {
					state = State::Flowc;
					break;
				}

				fail( seq::CompilerError( 1, "token '" + token.getRaw() + "'", "name, value, argument, function, expression or flow controller", "stream", token.getLine() ) );
				break;


			case State::Set:
				if( token.getCategory() == seq::Compiler::Token::Category::Name ) {
					if( !token.getAnchor() ) {
						bw.putName( token.getAnchor(), true, token.getClean().c_str() );
						state = State::Stream;
						break;
					}else{
						fail( seq::CompilerError( 1, "anchor after 'set' keyword", "name", "stream", token.getLine() ) );
					}
				}
				fail( seq::CompilerError( 1, "token: '" + token.getRaw() + "'", "name", "stream", token.getLine() ) );
				break;


			case State::Function: {
					int j = findClosing( tokens, i - 1, seq::Compiler::Token::Category::FuncBracket ) - 1;
					auto buf = assembleFunction( tokens, i, j, tokens.at(i - 1).getAnchor() );
					bw.putBuffer(buf);
					i = j;
					state = State::Stream;
					statmentCounter ++;
					break;
				}


			case State::Expression: {
					int j = findClosing( tokens, i - 1, seq::Compiler::Token::Category::MathBracket ) - 1;
					auto buf = assembleExpression( tokens, i, j, tokens.at(i - 1).getAnchor(), true );
					bw.putBuffer(buf);
					i = j;
					state = State::Stream;
					statmentCounter ++;
					break;
				}


			case State::Flowc: {
					int j = findClosing( tokens, i - 1, seq::Compiler::Token::Category::FlowBracket ) - 1;
					auto buf = assembleFlowc( tokens, i, j, tokens.at(i - 1).getAnchor() );
					bw.putBuffer(buf);
					i = j;
					state = State::Stream;
					statmentCounter ++;
					break;
				}


			case State::Stream:
				if( token.getCategory() != seq::Compiler::Token::Category::Stream ) {
					fail( seq::CompilerError( 1, "token '" + tokens.at(i).getRaw() + "'", "'<<'", "stream", token.getLine() ) );
				}else{
					state = State::Continue;
				}
				break;

		}

	}

	if( state != State::Stream ) {
		fail( seq::CompilerError( 1, "end of stream", "", "stream", tokens[end].getLine() ) );
	}

	std::vector<byte> ret;
	seq::BufferWriter bw2( ret, getTable() );
	bw2.putStream( false, tags, arr );

	return ret;

}

std::vector<byte> seq::Compiler::assemblePrimitive( seq::Compiler::Token token ) {

	std::vector<byte> arr;
	seq::BufferWriter bw( arr, getTable() );
	const bool flag = token.getAnchor();

	try{

		switch( token.getCategory() ) {

			case seq::Compiler::Token::Category::Null:
				bw.putNull(flag);
				break;

			case seq::Compiler::Token::Category::Bool:
				bw.putBool(flag, (bool) token.getData());
				break;

			case seq::Compiler::Token::Category::Number:
				bw.putNumber(flag, seq::util::asFraction( std::stod( token.getClean() ) ) );
				break;

			case seq::Compiler::Token::Category::Type:
				bw.putType(flag, seq::util::toDataType( token.getClean() ));
				break;

			case seq::Compiler::Token::Category::String:
				bw.putString(flag, token.getClean().c_str());
				break;

			case seq::Compiler::Token::Category::VMCall: {
				bw.putCall(flag, (seq::type::VMCall::CallType) token.getData());
				break;
			}

			case seq::Compiler::Token::Category::Arg:
				bw.putArg(flag, token.getData());
				break;

			// Name is not a primitive value but this simplifies some things
			case seq::Compiler::Token::Category::Name:
				bw.putName( token.getAnchor(), false, token.getClean().c_str() );
				break;

			default:
				throw seq::InternalError( "Invalid argument " + token.toString() + "!" );
		}

	}catch( ... ){
		throw seq::InternalError( "Invalid argument " + token.toString() + "!" );
	}

	return arr;
}

std::vector<byte> seq::Compiler::assembleFlowc( std::vector<seq::Compiler::Token>& tokens, int start, int end, bool anchor ) {

	std::vector<std::vector<byte>> arr;

	bool expectSeparator = false;
	int i = start;

	for( ; i < end; i ++ ) {

		seq::Compiler::Token& token = tokens[i];

		if( token.getAnchor() ) {
			fail( seq::CompilerError( 1, "anchor", "", "flow controller", token.getLine() ) );
		}

		if( expectSeparator ) {

			if( token.getCategory() == seq::Compiler::Token::Category::Comma ) {
				expectSeparator = false;
			}else{
				fail( seq::CompilerError( 1, "token '" + token.getRaw() + "'", "','", "flow controller", tokens.at(i).getLine() ) );
			}

		}else{

			switch( token.getCategory() ) {

				case seq::Compiler::Token::Category::Number: {

					if( i + 1 < end && tokens.at( i + 1 ).getCategory() == seq::Compiler::Token::Category::Colon ) {

						if( i + 2 < end && tokens.at( i + 2 ).getCategory() == seq::Compiler::Token::Category::Number ) {

							if( tokens.at( i + 2 ).getAnchor() ) {
								fail( seq::CompilerError( 1, "anchor", "", "flow controller", token.getLine() ) );
							}else{
								auto arr1 = assemblePrimitive( token );
								auto arr2 = assemblePrimitive( tokens.at( i + 2 ) );
								arr1.reserve( arr1.size() + arr2.size() );
								arr1.insert( arr1.end(), arr2.begin(), arr2.end() );
								arr.push_back( arr1 );

								i += 2;
								expectSeparator = true;
							}

						}else{
							fail( seq::CompilerError( 1, "token '" + tokens.at( i + 2 ).getRaw() + "'", "number", "flow controller", tokens.at(i + 2).getLine() ) );
						}

					}else{
						arr.push_back( assemblePrimitive( token ) );
						expectSeparator = true;
					}

					break;
				}

				case seq::Compiler::Token::Category::String:
				case seq::Compiler::Token::Category::Bool:
				case seq::Compiler::Token::Category::Type:
				case seq::Compiler::Token::Category::Null: {
					arr.push_back( assemblePrimitive( token ) );
					expectSeparator = true;
					break;
				}

				default:
					fail( seq::CompilerError( 1, "token '" + token.getRaw() + "'", "value or range", "flow controller", token.getLine() ) );

			}

		}

	}

	if( !expectSeparator || arr.empty() ) {
		fail( seq::CompilerError( 1, "", "value or range", "flow controller", tokens.at(i).getLine() ) );
	}

	std::vector<byte> ret;
	seq::BufferWriter bw( ret, getTable() );
	bw.putFlowc(anchor, arr);

	return ret;

}

std::vector<byte> seq::Compiler::assembleExpression( std::vector<seq::Compiler::Token>& tokens, int start, int end, bool anchor, bool top, bool* pure ) {

	if( top && tokens[start].getCategory() == seq::Compiler::Token::Category::Stream ) {
		if( start + 1 < end ) {
			return assembleStream( tokens, start + 1, end - 1, 0, false );
		}else{
			fail( seq::CompilerError( 2, "end of embedded stream", "", "stream", tokens[start].getLine() ) );
		}
	}

	if( end - start == 1 ) {

		auto& token = tokens.at( start );

		if( pure != nullptr && !token.isPure() ) {
			*pure = false;
		}

		if( token.getAnchor() ) {
			fail( seq::CompilerError( 1, "anchor", "", "expression", token.getLine() ) );
		}

		return assemblePrimitive( token );
	}

	int h = -1;
	int j = -1;
	int l = 0;
	int f = 0;
	bool eop = false;

	for( int i = start; i < end; i ++ ) {

		auto& token = tokens[ i ];

		if( token.getCategory() == seq::Compiler::Token::Category::MathBracket ) {

			if( token.getData() == 1 ) {
				if( eop ) fail( seq::CompilerError( 1, "", "operator", "expression", token.getLine() ) );
			}else{
				if( !eop ) fail( seq::CompilerError( 1, "operator", "", "expression", token.getLine() ) );
			}

		} else {
			if( eop && token.getCategory() != seq::Compiler::Token::Category::Operator ) {
				fail( seq::CompilerError( 1, "", "operator", "expression", token.getLine() ) );
			}else if( !eop && token.getCategory() == seq::Compiler::Token::Category::Operator ) {
				fail( seq::CompilerError( 1, "operator", "", "expression", token.getLine() ) );
			}
			eop = !eop;
		}

	}

	while ( true ) {

		for( int i = start; i < end; i ++ ) {

			auto& token = tokens[ i ];

			if( token.getCategory() == seq::Compiler::Token::Category::MathBracket ) {
				l += token.getData();
			}

			if( l == f && token.getCategory() == seq::Compiler::Token::Category::Operator ) {
				int tmp = token.getData() & 0b11111111;
				if( h < tmp ) {
					h = tmp;
					j = i;
				}
			}

		}

		if( h == -1 ) {
			if( f != 0 ) {
				fail( seq::CompilerError( 2, "end of expression", "operator", "expression", tokens.at(end - 1).getLine() ) );
			}

			l = 0;
			f = 1;
		}else{
			break;
		}

	}

	// settings this to true in turn enables the optimization,
	// otherwise nothing will be done.
	bool isPure = flags & (oflag_t) Optimizations::PureExpr;

	auto lt = assembleExpression( tokens, start + f, j, false, false, &isPure );
	auto rt = assembleExpression( tokens, j + 1, end - f, false, false, &isPure );
	seq::ExprOperator op = (seq::ExprOperator) (tokens.at(j).getData() >> 8);

	std::vector<byte> arr;
	seq::BufferWriter bw( arr, getTable() );
	bw.putExpr(anchor, op, lt, rt);

	if( isPure ) {
		Generic computed;

		{
			ByteBuffer bb( arr.data(), arr.size() );
			BufferReader br = bb.getReader();
			TokenReader tr = br.next();
			Generic expr = tr.getGeneric();

			seq::Executor executor;
			computed = executor.executeExpr( expr );
		}

		arr.clear();
		bw.putGeneric(computed);

	}else if( pure != nullptr ) {
		*pure = false;
	}

	return arr;
}

std::vector<byte> seq::Compiler::assembleFunction( std::vector<seq::Compiler::Token>& tokens, int start, int end, bool anchor, bool raw ) {

	std::vector<byte> arr;
	seq::BufferWriter bw( arr, getTable() );
	bool hasEndTag = false;

	// throw on empty functions
	if( end - start < 2 ) {
		fail( seq::CompilerError( 1, "end of scope", "stream", "function", tokens.at(start).getLine() ) );
	}

	for( int i = start; i < end; i ++ ) {

		byte tags = 0;

		// if token is tag store its data in 'tags'
		if( tokens[i].getCategory() == seq::Compiler::Token::Category::Tag ) {
			tags = tokens[i].getData();

			// if function contains any 'end' tag change opcode from FUN to FNE
			if( tags == SEQ_TAG_END ) {
				hasEndTag = true;
			}

			// end must not be the last token
			if( (++ i) >= end ) {
				fail( seq::CompilerError( 1, "end of input", "start of stream", "function", tokens[i].getLine() ) );
			}
		}

		int j = findStreamEnd( tokens, i, end );
		if( j != -1 ) {

			auto buf = assembleStream( tokens, i, j, tags, false );
			bw.putBuffer(buf);
			i = j;

		}else{
			fail( seq::CompilerError( 1, "end of input", "end of stream", "function", tokens.at(i).getLine() ) );
		}

	}

	// write function to output
	std::vector<byte> ret;
	seq::BufferWriter bw2( ret, getTable() );

	if(raw) {
		bw2.putBuffer(arr);
	}else{
		bw2.putFunc( anchor, arr, hasEndTag );
	}

	return ret;
}

void seq::Compiler::optimizeIfApplicable( std::vector<Token>& tokens ) {

	// if string pre-gen is enabled try searching for strings in tokens
	// and pregenerating sorted string table
	if( flags & (oflag_t) Optimizations::StrPreGen ) {
		if( this->names != nullptr ) {

			typedef std::pair<int, std::string> Pair;
			std::vector<Pair> pairs;

			for( int i = tokens.size() - 1; i >= 0; i -- ) {
				auto& token = tokens[i];

				if( token.isNamed() && (i == 0 || tokens[i - 1].getCategory() != Compiler::Token::Category::Load) ) {

					const std::string& entry = token.getClean();

					auto it = std::find_if( pairs.begin(), pairs.end(), [&entry] (const Pair& p) {
						return p.second == entry;
					} );

					if( it == pairs.end() ) {
						pairs.push_back( { 1, entry } );
					}else{
						pairs[ std::distance(pairs.begin(), it) ].first ++;
					}

				}
			}

			std::sort(pairs.begin(), pairs.end(), [](Pair& a, Pair& b) {
				return a.first > b.first;
			});

			for( Pair& pair : pairs ) {
				names->push_back( pair.second );
			}

		}
	}

}

int seq::Compiler::extractHeaderData( std::vector<Token>& tokens, StringTable* table ) {

	int s = 0;

	// check if first token is an load statement otherwise there is nothing to extract
	if( tokens.size() > 0 && tokens.at(0).getCategory() == seq::Compiler::Token::Category::Load ) {

		int l = 0;
		const int size = tokens.size();

		// iterate over token array
		for( int i = 0; i < size; i ++ ) {
			auto& token = tokens[ i ];

			if( l != (int) token.getLine() ) {
				l = token.getLine();

				if( token.getCategory() == seq::Compiler::Token::Category::Load ) {

					// validate load statement
					if( i + 1 >= size || (i + 2 >= size && (int) tokens.at( i + 2 ).getLine() == l) ) {
						fail( seq::CompilerError( 2, "token " + token.getRaw(), "load statement", "header", l ) );
					}

					auto& token2 = tokens[ i + 1 ];

					if( token2.getCategory() != seq::Compiler::Token::Category::String || token2.getAnchor() ) {
						fail( seq::CompilerError( 2, "token " + token.getRaw(), "load statement", "header", l ) );
					}

					// add load to supplied load array
					if( table != nullptr ) {
						seq::util::insertUnique(table, token2.getClean());
					}

					s = i;
					continue;

				}

				// otherwise return offset
				return i;

			}
		}
	}

	return s;

}

seq::StringTable* seq::Compiler::getTable() {
	return ( flags & (seq::oflag_t) Optimizations::Name ) ? names : nullptr;
}

void seq::Compiler::emit( seq::CompilerError error ) {
	if( this->handle( &error ) || error.isCritical() ) {
		throw error;
	}
}

void seq::Compiler::fail( seq::CompilerError error ) {
	emit(error);
}

void seq::Compiler::warn( seq::CompilerError warning ) {
	emit(warning);
}

bool seq::Compiler::defaultErrorHandle( seq::CompilerError* err ) {
	return err->isError();
}

void seq::Compiler::setErrorHandle( seq::Compiler::ErrorHandle handle ) {
	this->handle = handle;
}

void seq::Compiler::setNameTable( seq::StringTable* strings ) {
	this->names = strings;
}

void seq::Compiler::setLoadTable( seq::StringTable* strings ) {
	this->loades = strings;
}

void seq::Compiler::setOptimizationFlags( seq::oflag_t flags ) {
	this->flags = flags;
}

#endif // SEQ_EXCLUDE_COMPILER

#ifndef SEQ_EXCLUDE_DECOMPILER

seq::Indentation::Indentation() {
	this->str = "\t";
	this->index = 0;
}

void seq::Indentation::push() {
	this->index ++;
}

void seq::Indentation::pop() {
	if( -- this->index < 0 ) {
		throw seq::InternalError( "Invalid indentation index!" );
	}
}

void seq::Indentation::set(std::string& str, int base) {
	this->str = str;
	this->index = base;
}

std::string seq::Indentation::get() {
	if( this->index == 0 ) return "";
	std::string indentation;

	for( int i = 0; i < this->index; i ++ ) {
		indentation += this->str;
	}

	return std::move(indentation);
}

seq::Decompiler::Decompiler() {
}

void seq::Decompiler::setIndentation(std::string str, int base) {
	this->indentation.set(str, base);
}

std::string seq::Decompiler::decompile(BufferReader& reader) {
	seq::Stream stream = reader.readAll();
	return decompile( stream );
}

std::string seq::Decompiler::decompile(Generic& generic) {
	typedef std::string (*Writer)(Decompiler*, Generic&);

	static Writer WriterArr[ SEQ_MAX_DATA_TYPE ] = {
		/* 1  Bool   */ [] (Decompiler* dc, Generic& g) { return dc->writeBool(g); },
		/* 2  Null   */ [] (Decompiler* dc, Generic& g) { return dc->writeNull(g); },
		/* 3  Number */ [] (Decompiler* dc, Generic& g) { return dc->writeNumber(g); },
		/* 4  String */ [] (Decompiler* dc, Generic& g) { return dc->writeString(g); },
		/* 5  Type   */ [] (Decompiler* dc, Generic& g) { return dc->writeType(g); },
		/* 6  VMCall */ [] (Decompiler* dc, Generic& g) { return dc->writeVMCall(g); },
		/* 7  Arg    */ [] (Decompiler* dc, Generic& g) { return dc->writeArg(g); },
		/* 8  Func   */ [] (Decompiler* dc, Generic& g) { return dc->writeFunc(g); },
		/* 9  Expr   */ [] (Decompiler* dc, Generic& g) { return dc->writeExpr(g); },
		/* 10 Name   */ [] (Decompiler* dc, Generic& g) { return dc->writeName(g); },
		/* 11 Flowc  */ [] (Decompiler* dc, Generic& g) { return dc->writeFlowc(g); },
		/* 12 Stream */ [] (Decompiler* dc, Generic& g) { return dc->writeStream(g); },
		/* 13 Blob   */ [] (Decompiler* dc, Generic& g) { return dc->writeInvalid(g); }
	};

	return WriterArr[((seq::byte) generic.getDataType()) - 1]( this, generic );
}

std::string seq::Decompiler::decompile(Stream& stream) {
	std::string str;

	for( Generic& generic : stream ) {
		str += decompile( generic );
	}

	return std::move(str);
}

std::string seq::Decompiler::writeInvalid(Generic& g) {
	throw seq::InternalError("Invalid!");
}

std::string seq::SourceDecompiler::writeBool( Generic& g ) {
	return anchor(g) + (g.Bool().getBool() ? "true " : "false ");
}

std::string seq::SourceDecompiler::writeNull( Generic& g ) {
	return anchor(g) + "null ";
}

std::string seq::SourceDecompiler::writeNumber( Generic& g ) {
	type::Number& n = g.Number();
	return anchor(g) + (n.isNatural() ? std::to_string(n.getLong()) : std::to_string(n.getDouble())) + " ";
}

std::string seq::SourceDecompiler::writeFunc( Generic& g ) {
	indentation.push();
	std::string str = anchor(g) + "{\n" + decompile( g.Function().getReader() );
	indentation.pop();
	return str + indentation.get() + "} ";
}

std::string seq::SourceDecompiler::writeFlowc( Generic& g ) {
	std::string str = anchor(g) + "[";

	for( FlowCondition* c : g.Flowc().getConditions() ) {
		str += condition( c );
		str += ", ";
	}

	str.pop_back();
	str.pop_back();

	return str + "] ";
}

std::string seq::SourceDecompiler::writeExpr( Generic& g ) {
	type::Expression& e = g.Expression();
	return "( " + decompile( e.getLeftReader() ) + exprop(e.getOperator()) + " " + decompile(e.getRightReader()) + ") ";
}

std::string seq::SourceDecompiler::writeName( Generic& g ) {
	type::Name& n = g.Name();
	return std::string(n.getDefine() ? "set " : "") + anchor(g) + n.getName() + " ";
}

std::string seq::SourceDecompiler::writeString( Generic& g ) {
	std::string str = anchor(g) + "\"";

	for( char chr : g.String().getString() ) {
		switch( chr ) {
			case '\n': str += "\\n"; break;
			case '\t': str += "\\t"; break;
			case '\"': str += "\\\""; break;
			case '\\': str += "\\\\"; break;
			case '\r': str += "\\r"; break;
			default: str += chr;
		}
	}

	return str + "\" ";
}

std::string seq::SourceDecompiler::writeStream( Generic& g ) {
	seq::Stream stream = g.Stream().getReader().readAll();
	int size = stream.size();
	byte tags = g.Stream().getTags();

	std::string str = indentation.get();

	if( tags & SEQ_TAG_END ) str += "end; ";
	if( tags & SEQ_TAG_FIRST ) str += "first; ";
	if( tags & SEQ_TAG_LAST ) str += "last; ";

	for( int i = 0; i < size; i ++ ) {
		Generic& g = stream[i];
		std::string sub = decompile( stream[i] );

		if( g.getDataType() == DataType::Stream ) {

			// remove trailing newline
			sub.pop_back();

			str += "( << " + sub + ") ";
		}else{
			str += sub;
		}

		if( i + 1 != size ) str += "<< ";
	}

	return str + "\n";
}

std::string seq::SourceDecompiler::writeVMCall( Generic& g ) {
	using CallType = type::VMCall::CallType;
	switch( g.VMCall().getCall() ) {
		case CallType::Again: return anchor(g) + "again ";
		case CallType::Break: return anchor(g) + "break ";
		case CallType::Exit: return anchor(g) + "exit ";
		case CallType::Final: return anchor(g) + "final ";
		case CallType::Return: return anchor(g) + "return ";
	}

	throw seq::InternalError("Invalid VMCall ID!");
}

std::string seq::SourceDecompiler::writeArg( Generic& g ) {
	return anchor(g) + std::string( (size_t) g.Arg().getLevel() + 1, '@' ) + " ";
}

std::string seq::SourceDecompiler::writeType( Generic& g ) {
	DataType type = g.Type().getType();

	if( type == seq::DataType::Number ) return anchor(g) + "number ";
	if( type == seq::DataType::Bool ) return anchor(g) + "bool ";
	if( type == seq::DataType::String ) return anchor(g) + "string ";
	if( type == seq::DataType::Type ) return anchor(g) + "type ";

	throw seq::InternalError("Invalid Type ID!");
}

std::string seq::SourceDecompiler::anchor( Generic& g ) {
	return std::string(g.getAnchor() ? "#" : "");
}

std::string seq::SourceDecompiler::condition( FlowCondition* c ) {
	std::string str = decompile( c->a );

	// remove trailing space
	str.pop_back();

	if( c->type != FlowCondition::Type::Range ) {
		return str;
	}else{
		std::string str2 = decompile( c->b );

		// remove trailing space
		str2.pop_back();

		return str + ":" + str2;
	}
}

std::string seq::SourceDecompiler::exprop( ExprOperator op ) {
	switch( op ) {
		case ExprOperator::Less: return "<";
		case ExprOperator::Greater: return ">";
		case ExprOperator::Equal: return "=";
		case ExprOperator::NotEqual: return "!=";
		case ExprOperator::NotGreater: return "<=";
		case ExprOperator::NotLess: return ">=";
		case ExprOperator::And: return "&&";
		case ExprOperator::Or: return "||";
		case ExprOperator::Xor: return "^^";
		case ExprOperator::Not: return "!";
		case ExprOperator::Multiplication: return "*";
		case ExprOperator::Division: return "/";
		case ExprOperator::Addition: return "+";
		case ExprOperator::Subtraction: return "-";
		case ExprOperator::Modulo: return "%";
		case ExprOperator::Power: return "**";
		case ExprOperator::BinaryAnd: return "&";
		case ExprOperator::BinaryOr: return "|";
		case ExprOperator::BinaryXor: return "^";
		case ExprOperator::BinaryNot: return "~";
		case ExprOperator::Accessor: return "::";
	}

	throw seq::InternalError("Invalid state!");
}

#endif // SEQ_EXCLUDE_DECOMPILER

#undef SEQ_IMPLEMENT
#endif // SEQ_IMPLEMENT

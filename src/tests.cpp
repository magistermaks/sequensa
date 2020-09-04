
#include "SeqAPI.hpp"
#include "vstl.hpp"

void print_buffer( seq::ByteBuffer& bb ) {
    seq::BufferReader br = bb.getReader();
    while( br.hasNext() ) std::cout << (int) br.nextByte() << " ";
}


TEST( buffer_reader_simple, {

    byte buffer[] = { 'A', 'B', 'C', 'D' };
    seq::ByteBuffer bb = seq::ByteBuffer( buffer, 4 );
    seq::BufferReader br = bb.getReader();

    CHECK( br.nextByte(), (byte) 'A' );
    CHECK( br.peekByte(), (byte) 'B' );
    CHECK( br.nextByte(), (byte) 'B' );
} );


TEST( buffer_writer_simple, {

    std::vector<byte> buf;
    seq::BufferWriter bw( buf );

    bw.putByte( 'S' );
    bw.putBool( true, true );
    bw.putBool( true, false );
    bw.putBool( false, true );
    bw.putBool( false, false );
    bw.putNull( false );

    CHECK( buf[0], (byte) 'S' );
    CHECK( buf[1], (byte) ((byte) seq::Opcode::BLT | 0b10000000) );
    CHECK( buf[2], (byte) ((byte) seq::Opcode::BLF | 0b10000000) );
    CHECK( buf[3], (byte) ((byte) seq::Opcode::BLT | 0b00000000) );
    CHECK( buf[4], (byte) ((byte) seq::Opcode::BLF | 0b00000000) );
    CHECK( buf[5], (byte) ((byte) seq::Opcode::NIL | 0b00000000) );
} );


TEST( buffer_writer_complex, {

    std::vector<byte> arr;
    seq::BufferWriter bw( arr );

    bw.putByte( 'S' );
    bw.putBool( true, true );
    bw.putNumber( false, (seq::Fraction) {12, 1} );
    bw.putNumber( true, (seq::Fraction) {1, 2} );
    bw.putNumber( false, (seq::Fraction) {324221312413, 1} );
    bw.putString( true, (byte*) "Hello World!" );
    bw.putType( false, seq::DataType::Number );
    bw.putCall( false, seq::type::VMCall::CallType::Return );
    bw.putArg( false, 3 );
    bw.putName( false, true, (byte*) "name" );

    seq::ByteBuffer bb( arr.data(), arr.size() );
    seq::BufferReader br = bb.getReader();

    try{

        CHECK( br.nextByte(), (byte) 'S' );

        { // bool
            seq::TokenReader tr = br.next();
            CHECK( (byte) tr.getDataType(), (byte) seq::DataType::Bool );
            CHECK( tr.isAnchored(), true );
            CHECK( ((seq::type::Bool*) tr.getGeneric())->getBool(), true );
        }

        { // small number
            seq::TokenReader tr = br.next();
            CHECK( (byte) tr.getDataType(), (byte) seq::DataType::Number );
            CHECK( tr.isAnchored(), false );
            CHECK( ((seq::type::Number*) tr.getGeneric())->getLong(), (long) 12 );
        }

        { // fraction
            seq::TokenReader tr = br.next();
            CHECK( (byte) tr.getDataType(), (byte) seq::DataType::Number );
            CHECK( tr.isAnchored(), true );
            CHECK( ((seq::type::Number*) tr.getGeneric())->getDouble(), (double) 0.5 );
        }

        { // big number
            seq::TokenReader tr = br.next();
            CHECK( (byte) tr.getDataType(), (byte) seq::DataType::Number );
            CHECK( tr.isAnchored(), false );
            CHECK( ((seq::type::Number*) tr.getGeneric())->getLong(), (long) 324221312413 );
        }

        { // string
            seq::TokenReader tr = br.next();
            CHECK( (byte) tr.getDataType(), (byte) seq::DataType::String );
            CHECK( tr.isAnchored(), true );
            CHECK_ELSE( ((seq::type::String*) tr.getGeneric())->getString(), seq::string( (byte*) "Hello World!" ) ) {
                FAIL( "Invalid string!" );
            }
        }

        { // type
            seq::TokenReader tr = br.next();
            CHECK( (byte) tr.getDataType(), (byte) seq::DataType::Type );
            CHECK( tr.isAnchored(), false );
            CHECK( (byte) ((seq::type::Type*) tr.getGeneric())->getType(), (byte) seq::DataType::Number );
        }

        { // VM call
            seq::TokenReader tr = br.next();
            CHECK( (byte) tr.getDataType(), (byte) seq::DataType::VMCall );
            CHECK( tr.isAnchored(), false );
            CHECK( (byte) ((seq::type::VMCall*) tr.getGeneric())->getCall(), (byte) seq::type::VMCall::CallType::Return );
        }

        { // arg
            seq::TokenReader tr = br.next();
            CHECK( (byte) tr.getDataType(), (byte) seq::DataType::Arg );
            CHECK( tr.isAnchored(), false );
            CHECK( ((seq::type::Arg*) tr.getGeneric())->getLevel(), (byte) 3 );
        }

        { // name (variable)
            seq::TokenReader tr = br.next();
            CHECK( (byte) tr.getDataType(), (byte) seq::DataType::Name );
            CHECK( tr.isAnchored(), false );
            seq::type::Name* name = (seq::type::Name*) tr.getGeneric();
            CHECK( name->getDefine(), true );
            CHECK_ELSE( name->getName(), seq::string( (byte*) "name" ) ) {
                FAIL( "Invalid string! - " + std::string( (char*) name->getName().c_str() ) );
            }
        }

    }catch( seq::InternalError& err ) {
        FAIL( err.what() );
    }

} );


TEST( buffer_writer_function, {

    std::vector<byte> func_arr;
    seq::BufferWriter func_bw( func_arr );
    func_bw.putByte( (byte) 'A' );
    func_bw.putByte( (byte) 'B' );
    func_bw.putByte( (byte) 'C' );

    std::vector<byte> arr;
    seq::BufferWriter bw( arr );
    bw.putFunc( false, func_arr );
    bw.putNull( false );

    seq::ByteBuffer bb( arr.data(), arr.size() );
    seq::BufferReader br = bb.getReader();

    seq::TokenReader tr = br.next();
    CHECK( (byte) tr.getDataType(), (byte) seq::DataType::Func );
    CHECK( tr.isAnchored(), false );

    {
        seq::BufferReader func_br = ((seq::type::Function*) tr.getGeneric())->getReader();
        CHECK( func_br.nextByte(), (byte) 'A' );
        CHECK( func_br.nextByte(), (byte) 'B' );
        CHECK( func_br.nextByte(), (byte) 'C' );
        CHECK( func_br.nextByte(), (byte) 0 );
    }

    {
        seq::BufferReader func_br = ((seq::type::Function*) tr.getGeneric())->getReader();
        CHECK( func_br.nextByte(), (byte) 'A' );
        CHECK( func_br.nextByte(), (byte) 'B' );
        CHECK( func_br.nextByte(), (byte) 'C' );
        CHECK( func_br.nextByte(), (byte) 0 );
    }

    seq::TokenReader tr2 = br.next();
    CHECK( (byte) tr2.getDataType(), (byte) seq::DataType::Null );
    CHECK( tr2.isAnchored(), false );
} );


TEST( buffer_writer_expression, {

    std::vector<byte> left_arr;
    seq::BufferWriter left_bw( left_arr );
    left_bw.putByte( (byte) 'A' );
    left_bw.putByte( (byte) 'B' );
    left_bw.putByte( (byte) 'C' );

    std::vector<byte> right_arr;
    seq::BufferWriter right_bw( right_arr );
    right_bw.putByte( (byte) '1' );
    right_bw.putByte( (byte) '2' );
    right_bw.putByte( (byte) '3' );

    std::vector<byte> arr;
    seq::BufferWriter bw( arr );
    bw.putExpr( false, seq::ExprOperator::Addition, left_arr, right_arr );
    bw.putNull( false );

    seq::ByteBuffer bb( arr.data(), arr.size() );
    seq::BufferReader br = bb.getReader();

    seq::TokenReader tr = br.next();
    CHECK( (byte) tr.getDataType(), (byte) seq::DataType::Expr );
    CHECK( tr.isAnchored(), false );

    CHECK( (byte) ((seq::type::Expression*) tr.getGeneric())->getOperator(), (byte) seq::ExprOperator::Addition );

    {
        seq::BufferReader expr_br = ((seq::type::Expression*) tr.getGeneric())->getLeftReader();
        CHECK( expr_br.nextByte(), (byte) 'A' );
        CHECK( expr_br.nextByte(), (byte) 'B' );
        CHECK( expr_br.nextByte(), (byte) 'C' );
        CHECK( expr_br.nextByte(), (byte) 0 );
    }

    {
        seq::BufferReader expr_br = ((seq::type::Expression*) tr.getGeneric())->getRightReader();
        CHECK( expr_br.nextByte(), (byte) '1' );
        CHECK( expr_br.nextByte(), (byte) '2' );
        CHECK( expr_br.nextByte(), (byte) '3' );
        CHECK( expr_br.nextByte(), (byte) 0 );
    }

    seq::TokenReader tr2 = br.next();
    CHECK( (byte) tr2.getDataType(), (byte) seq::DataType::Null );
    CHECK( tr2.isAnchored(), false );

} );


TEST( buffer_writer_flowc, {

    std::vector<byte> a_arr;
    seq::BufferWriter a_bw( a_arr );
    a_bw.putNumber( false, (seq::Fraction) {12, 1} );
    a_bw.putNumber( false, (seq::Fraction) {24, 1} );

    std::vector<byte> b_arr;
    seq::BufferWriter b_bw( b_arr );
    b_bw.putBool( false, false );

    std::vector<byte> arr;
    seq::BufferWriter bw( arr );

    std::vector<std::vector<byte>> bvv = { a_arr, b_arr };
    bw.putFlowc( false, bvv );
    bw.putNull( false );

    seq::ByteBuffer bb( arr.data(), arr.size() );
    seq::BufferReader br = bb.getReader();

    {
        seq::TokenReader tr = br.next();
        CHECK( (byte) tr.getDataType(), (byte) seq::DataType::Flowc );
        CHECK( tr.isAnchored(), false );
        seq::type::Flowc* f = ((seq::type::Flowc*) tr.getGeneric());

        std::vector< seq::FlowCondition* > fcs = f->getConditions();

        CHECK( fcs.size(), (size_t) 2 );
        CHECK( (byte) fcs.at(0)->type, (byte) seq::FlowCondition::Type::Range );
        CHECK( ( (seq::type::Number*) fcs.at(0)->a )->getLong(), (long) 12);
        CHECK( ( (seq::type::Number*) fcs.at(0)->b )->getLong(), (long) 24);
        CHECK( (byte) fcs.at(1)->type, (byte) seq::FlowCondition::Type::Value );
        CHECK( ( (seq::type::Bool*) fcs.at(1)->a )->getBool(), false );
    }

} );


TEST( buffer_writer_stream, {

    std::vector<byte> stream_arr;
    seq::BufferWriter stream_bw( stream_arr );
    stream_bw.putByte( (byte) 'A' );
    stream_bw.putByte( (byte) 'B' );
    stream_bw.putByte( (byte) 'C' );

    std::vector<byte> arr;
    seq::BufferWriter bw( arr );

    bw.putStream( true, 0b00000011, stream_arr );
    bw.putNull( false );

    seq::ByteBuffer bb( arr.data(), arr.size() );
    seq::BufferReader br = bb.getReader();

    try{
        seq::TokenReader tr = br.next();
        CHECK( (byte) tr.getDataType(), (byte) seq::DataType::Stream );
        CHECK( tr.isAnchored(), true );
        seq::type::Stream* s = ((seq::type::Stream*) tr.getGeneric());

        seq::BufferReader sbr = s->getReader();
        CHECK( sbr.nextByte(), (byte) 'A' );
        CHECK( sbr.nextByte(), (byte) 'B' );
        CHECK( sbr.nextByte(), (byte) 'C' );
        CHECK( sbr.nextByte(), (byte) 0 );

        seq::TokenReader tr2 = br.next();
        CHECK( (byte) tr2.getDataType(), (byte) seq::DataType::Null );
        CHECK( tr2.isAnchored(), false );
    }catch( seq::InternalError& err ) {
        FAIL( err.what() );
    }


} );


TEST( buffer_writer_file_header, {
    std::vector<byte> arr;
    seq::BufferWriter bw( arr );

    std::map<seq::string, seq::string> data;
    data[ (byte*) "test"] = (byte*) "Hello World!";
    data[ (byte*) "Foo"] = (byte*) "Bar";
    data[ (byte*) "number"] = (byte*) "123.4";

    bw.putFileHeader( 1, 2, 3, data );
    bw.putByte( (byte) 'A' );

    seq::ByteBuffer bb( arr.data(), arr.size() );
    seq::BufferReader br = bb.getReader();
    seq::FileHeader header = br.getHeader();

    CHECK( header.checkVersion( 1, 2 ), true );
    CHECK( header.checkVersion( 1, 3 ), false );
    CHECK( header.checkPatch( 3 ), true );
    CHECK( header.checkPatch( 4 ), false );

    CHECK_ELSE( header.getValue( (byte*) "test" ), seq::string( (byte*) "Hello World!" ) ) {
        FAIL( "Invalid string! - " + std::string( (char*) header.getValue( (byte*) "test" ).c_str() ) );
    }

    CHECK_ELSE( header.getValue( (byte*) "Foo" ), seq::string( (byte*) "Bar" ) ) {
        FAIL( "Invalid string! - " + std::string( (char*) header.getValue( (byte*) "Foo" ).c_str() ) );
    }

    CHECK_ELSE( header.getValue( (byte*) "number" ), seq::string( (byte*) "123.4" ) ) {
        FAIL( "Invalid string! - " + std::string( (char*) header.getValue( (byte*) "number" ).c_str() ) );
    }

    CHECK( br.nextByte(), (byte) 'A' );
} );


TEST( executor_hello_world, {

	std::vector<byte> arr_1;
	seq::BufferWriter bw_1( arr_1 );
	bw_1.putCall( true, seq::type::VMCall::CallType::Exit );
	bw_1.putString( false, (byte*) "Hello World!" );

	std::vector<byte> arr_2;
	seq::BufferWriter bw_2( arr_2 );
	bw_2.putStream( false, 0, arr_1 );

	seq::ByteBuffer bb( arr_2.data(), arr_2.size() );

	seq::Stream args;
	args.push_back( new seq::type::Null( false ) );

	seq::Executor exe;
	exe.execute( bb, args );

	CHECK( (byte) exe.getResult()->getDataType(), (byte) seq::DataType::String );
	CHECK_ELSE( ((seq::type::String*) exe.getResult())->getString(), seq::string( (byte*) "Hello World!" ) ) {
		FAIL( "Invalid String!" );
	}

} );


TEST( executor_hello_world_var, {

	try{

		std::vector<byte> arr_1;
		seq::BufferWriter bw_1( arr_1 );
		bw_1.putName( false, true, (byte*) "var" );
		bw_1.putString( false, (byte*) "Hello World!" );

		std::vector<byte> arr_2;
		seq::BufferWriter bw_2( arr_2 );
		bw_2.putCall( true, seq::type::VMCall::CallType::Exit );
		bw_2.putName( false, false, (byte*) "var" );

		std::vector<byte> arr_3;
		seq::BufferWriter bw_3( arr_3 );
		bw_3.putStream( false, 0, arr_1 );
		bw_3.putStream( false, 0, arr_2 );

		seq::ByteBuffer bb( arr_3.data(), arr_3.size() );

		seq::Stream args;
		args.push_back( new seq::type::Null( false ) );

		seq::Executor exe;
		exe.execute( bb, args );

		CHECK( (byte) exe.getResult()->getDataType(), (byte) seq::DataType::String );
		CHECK_ELSE( ((seq::type::String*) exe.getResult())->getString(), seq::string( (byte*) "Hello World!" ) ) {
			FAIL( "Invalid String!" );
		}

	}catch( seq::InternalError& err ) {
        FAIL( err.what() );
    }

} );

TEST( execute_hello_world_func, {

	std::vector<byte> arr_1;
	seq::BufferWriter bw_1( arr_1 );
	bw_1.putCall( true, seq::type::VMCall::CallType::Return );
	bw_1.putString( false, (byte*) "Hello World!" );

	std::vector<byte> arr_2;
	seq::BufferWriter bw_2( arr_2 );
	bw_2.putStream( false, 0, arr_1 );

	std::vector<byte> arr_3;
	seq::BufferWriter bw_3( arr_3 );
	bw_3.putName( false, true, (byte*) "var" );
	bw_3.putFunc( false, arr_2 );

	std::vector<byte> arr_4;
	seq::BufferWriter bw_4( arr_4 );
	bw_4.putCall( true, seq::type::VMCall::CallType::Exit );
	bw_4.putName( true, false, (byte*) "var" );
	bw_4.putNull( false );

	std::vector<byte> arr_5;
	seq::BufferWriter bw_5( arr_5 );
	bw_5.putStream( false, 0, arr_3 );
	bw_5.putStream( false, 0, arr_4 );

	seq::ByteBuffer bb( arr_5.data(), arr_5.size() );

	seq::Stream args;
	args.push_back( new seq::type::Null( false ) );

	seq::Executor exe;
	exe.execute( bb, args );

	CHECK( (byte) exe.getResult()->getDataType(), (byte) seq::DataType::String );
	CHECK_ELSE( ((seq::type::String*) exe.getResult())->getString(), seq::string( (byte*) "Hello World!" ) ) {
		FAIL( "Invalid String!" );
	}

} );

TEST( executor_native, {

	seq::Executor exe;
	exe.inject( (byte*) "sum", [] ( seq::Stream input ) -> seq::Stream {
		double sum = 0;
		for( auto arg : input ) {
			seq::type::Number* num = seq::util::numberCast( arg );
			sum += num->getDouble();
			delete num;
		}
		input.clear();

		seq::Stream acc;
		acc.push_back( new seq::type::Number( false, sum ) );
		return acc;
	} );

	std::vector<byte> arr_1;
	seq::BufferWriter bw_1( arr_1 );
	bw_1.putCall( true, seq::type::VMCall::CallType::Exit );
	bw_1.putName( true, false, (byte*) "sum" );
	bw_1.putNumber( false, (seq::Fraction) {1, 1} );
	bw_1.putNumber( false, (seq::Fraction) {2, 1} );
	bw_1.putNumber( false, (seq::Fraction) {3, 1} );
	bw_1.putNumber( false, (seq::Fraction) {4, 1} );

	std::vector<byte> arr_2;
	seq::BufferWriter bw_2( arr_2 );
	bw_2.putStream( false, 0, arr_1 );

	seq::ByteBuffer bb( arr_2.data(), arr_2.size() );

	seq::Stream args;
	args.push_back( new seq::type::Null( false ) );

	exe.execute( bb, args );

	CHECK( (byte) exe.getResult()->getDataType(), (byte) seq::DataType::Number );
	CHECK( ((seq::type::Number*) exe.getResult())->getLong(), (long) 10 );

} );

TEST( expression_simple, {

	std::vector<byte> arr_1;
	seq::BufferWriter bw_1( arr_1 );
	bw_1.putNumber( false, (seq::Fraction) {6, 1} );

	std::vector<byte> arr_2;
	seq::BufferWriter bw_2( arr_2 );
	bw_2.putNumber( false, (seq::Fraction) {7, 1} );

	std::vector<byte> arr_3;
	seq::BufferWriter bw_3( arr_3 );
	bw_3.putCall( true, seq::type::VMCall::CallType::Exit );
	bw_3.putExpr( false, seq::ExprOperator::Multiplication, arr_1, arr_2 );

	std::vector<byte> arr_4;
	seq::BufferWriter bw_4( arr_4 );
	bw_4.putStream( false, 0, arr_3 );

	seq::ByteBuffer bb( arr_4.data(), arr_4.size() );

	seq::Stream args;
	args.push_back( new seq::type::Null( false ) );

	seq::Executor exe;
	exe.execute( bb, args );

	CHECK( (byte) exe.getResult()->getDataType(), (byte) seq::DataType::Number );
	CHECK( ((seq::type::Number*) exe.getResult())->getLong(), (long) 42 );

} );

TEST( tokenizer_basic, {

	std::vector<seq::Compiler::Token> tokens = seq::Compiler::tokenize( "#exit << 1 << \"Hello\" << 3"_b );

	if( tokens.at(0).getCategory() != seq::Compiler::Token::Category::VMC ) {
		FAIL( "Expected 'VMCall' token at index 0, found " + tokens.at(0).toString() );
	}

	if( tokens.at(1).getCategory() != seq::Compiler::Token::Category::Stream ) {
		FAIL( "Expected 'Stream' token at index 1, found " + tokens.at(1).toString() );
	}

	if( tokens.at(2).getCategory() != seq::Compiler::Token::Category::Number ) {
		FAIL( "Expected 'Number' token at index 2, found " + tokens.at(2).toString() );
	}

	if( tokens.at(6).getCategory() != seq::Compiler::Token::Category::Number ) {
		FAIL( "Expected 'Number' token at index 6, found " + tokens.at(6).toString() );
	}

} );

TEST( ce_hello_world, {

	try{

		auto tokens = seq::Compiler::tokenize( "#exit << \"Hello World!\""_b );
		auto buf = seq::Compiler::assembleFunction( tokens, 0, tokens.size(), true );
		seq::ByteBuffer bb( buf.data() + 3, buf.size() - 3 );

		seq::Stream args;
		args.push_back( new seq::type::Null( false ) );

		seq::Executor exe;
		exe.execute( bb, args );

		CHECK( (byte) exe.getResult()->getDataType(), (byte) seq::DataType::String );
		CHECK_ELSE( ((seq::type::String*) exe.getResult())->getString(), seq::string( (byte*) "Hello World!" ) ) {
			FAIL( "Invalid String!" );
		}

	}catch( seq::CompilerError& err ) {
		FAIL( err.what() );
	}catch( seq::InternalError& err ) {
		FAIL( err.what() );
	}

} );

TEST( ce_hello_world_func, {

	try{

		seq::string code = (byte*) ("set var << {\n"
				"	#return << \"Hello World!\"\n"
				"}\n"
				"#exit << #var << null\n");

		auto tokens = seq::Compiler::tokenize( code );

		//for( auto& t : tokens ) std::cout << t.toString() << std::endl;

		auto buf = seq::Compiler::assembleFunction( tokens, 0, tokens.size(), true );
		seq::ByteBuffer bb( buf.data() + 3, buf.size() - 3 );

		seq::Stream args;
		args.push_back( new seq::type::Null( false ) );

		seq::Executor exe;
		exe.execute( bb, args );

		CHECK( (byte) exe.getResult()->getDataType(), (byte) seq::DataType::String );
		CHECK_ELSE( ((seq::type::String*) exe.getResult())->getString(), seq::string( (byte*) "Hello World!" ) ) {
			FAIL( "Invalid String!" );
		}

	}catch( seq::CompilerError& err ) {
		FAIL( err.what() );
	}catch( seq::InternalError& err ) {
		FAIL( err.what() );
	}

} );

TEST( ce_expression, {

	try{

		seq::string code = (byte*) ("#exit << ( 1 + 2 + 3 + 4 )");

		auto tokens = seq::Compiler::tokenize( code );

		//for( auto& t : tokens ) std::cout << t.toString() << std::endl;

		auto buf = seq::Compiler::assembleFunction( tokens, 0, tokens.size(), true );
		seq::ByteBuffer bb( buf.data() + 3, buf.size() - 3 );

		seq::Stream args;
		args.push_back( new seq::type::Null( false ) );

		seq::Executor exe;
		exe.execute( bb, args );

		CHECK( (byte) exe.getResult()->getDataType(), (byte) seq::DataType::Number );
		CHECK( ((seq::type::Number*) exe.getResult())->getLong(), (long) 10 );

	}catch( seq::CompilerError& err ) {
		FAIL( err.what() );
	}catch( seq::InternalError& err ) {
		FAIL( err.what() );
	}catch( std::out_of_range& err ) {
		FAIL( err.what() );
	}

} );


TEST( ce_expression_nested_right, {

	try{

		seq::string code = (byte*) ("#exit << ( 1 + ( 2 + 3 ) )");

		auto tokens = seq::Compiler::tokenize( code );

		//for( auto& t : tokens ) std::cout << t.toString() << std::endl;

		auto buf = seq::Compiler::assembleFunction( tokens, 0, tokens.size(), true );
		seq::ByteBuffer bb( buf.data() + 3, buf.size() - 3 );

		seq::Stream args;
		args.push_back( new seq::type::Null( false ) );

		seq::Executor exe;
		exe.execute( bb, args );

		CHECK( (byte) exe.getResult()->getDataType(), (byte) seq::DataType::Number );
		CHECK( ((seq::type::Number*) exe.getResult())->getLong(), (long) 6 );

	}catch( seq::CompilerError& err ) {
		FAIL( err.what() );
	}catch( seq::InternalError& err ) {
		FAIL( err.what() );
	}catch( std::out_of_range& err ) {
		FAIL( err.what() );
	}

} );

TEST( ce_expression_nested_left, {

	try{

		seq::string code = (byte*) ("#exit << ( ( 2 + 3 ) + 1 )");

		auto tokens = seq::Compiler::tokenize( code );

		//for( auto& t : tokens ) std::cout << t.toString() << std::endl;

		auto buf = seq::Compiler::assembleFunction( tokens, 0, tokens.size(), true );
		seq::ByteBuffer bb( buf.data() + 3, buf.size() - 3 );

		seq::Stream args;
		args.push_back( new seq::type::Null( false ) );

		seq::Executor exe;
		exe.execute( bb, args );

		CHECK( (byte) exe.getResult()->getDataType(), (byte) seq::DataType::Number );
		CHECK( ((seq::type::Number*) exe.getResult())->getLong(), (long) 6 );

	}catch( seq::CompilerError& err ) {
		FAIL( err.what() );
	}catch( seq::InternalError& err ) {
		FAIL( err.what() );
	}catch( std::out_of_range& err ) {
		FAIL( err.what() );
	}

} );


TEST( ce_expression_nested_double, {

	try{

		seq::string code = (byte*) ("#exit << ( ( 2 + 3 ) + ( 4 + 1 ) )");

		auto tokens = seq::Compiler::tokenize( code );

		//for( auto& t : tokens ) std::cout << t.toString() << std::endl;

		auto buf = seq::Compiler::assembleFunction( tokens, 0, tokens.size(), true );
		seq::ByteBuffer bb( buf.data() + 3, buf.size() - 3 );

		seq::Stream args;
		args.push_back( new seq::type::Null( false ) );

		seq::Executor exe;
		exe.execute( bb, args );

		CHECK( (byte) exe.getResult()->getDataType(), (byte) seq::DataType::Number );
		CHECK( ((seq::type::Number*) exe.getResult())->getLong(), (long) 10 );

	}catch( seq::CompilerError& err ) {
		FAIL( err.what() );
	}catch( seq::InternalError& err ) {
		FAIL( err.what() );
	}catch( std::out_of_range& err ) {
		FAIL( err.what() );
	}

} );

TEST( ce_expression_float, {

	try{

		seq::string code = (byte*) ("#exit << ( ( 2.14 + 3.41 ) * 100 )");

		auto tokens = seq::Compiler::tokenize( code );

		//for( auto& t : tokens ) std::cout << t.toString() << std::endl;

		auto buf = seq::Compiler::assembleFunction( tokens, 0, tokens.size(), true );
		seq::ByteBuffer bb( buf.data() + 3, buf.size() - 3 );

		seq::Stream args;
		args.push_back( new seq::type::Null( false ) );

		seq::Executor exe;
		exe.execute( bb, args );

		CHECK( (byte) exe.getResult()->getDataType(), (byte) seq::DataType::Number );
		CHECK( ((seq::type::Number*) exe.getResult())->getLong(), 555l );

	}catch( seq::CompilerError& err ) {
		FAIL( err.what() );
	}catch( seq::InternalError& err ) {
		FAIL( err.what() );
	}catch( std::out_of_range& err ) {
		FAIL( err.what() );
	}

} );


TEST( ce_expression_complex, {

	try{

		seq::string code = (byte*) (
				"set var << {\n"
				"	#return << @\n"
				"}\n"
				"#exit << #var << 10\n");

		auto buf = seq::Compiler::compile( code );
		seq::ByteBuffer bb( buf.data(), buf.size() );

		seq::Stream args;
		args.push_back( new seq::type::Null( false ) );

		seq::Executor exe;
		exe.execute( bb, args );

		CHECK( (byte) exe.getResult()->getDataType(), (byte) seq::DataType::Number );
		CHECK( ((seq::type::Number*) exe.getResult())->getLong(), 10l );

	}catch( seq::CompilerError& err ) {
		FAIL( err.what() );
	}catch( seq::InternalError& err ) {
		FAIL( err.what() );
	}

} );

TEST( ce_arg, {

	try{

		seq::string code = (byte*) (
				"#{\n"
				"	#exit << #{\n"
				"		#return << (@ + @@) \n"
				"	} << 20\n"
				"} << 10\n");

		auto buf = seq::Compiler::compile( code );
		seq::ByteBuffer bb( buf.data(), buf.size() );

		seq::Stream args;
		args.push_back( new seq::type::Null( false ) );

		seq::Executor exe;
		exe.execute( bb, args );

		CHECK( (byte) exe.getResult()->getDataType(), (byte) seq::DataType::Number );
		CHECK( ((seq::type::Number*) exe.getResult())->getLong(), 30l );

	}catch( seq::CompilerError& err ) {
		FAIL( err.what() );
	}catch( seq::InternalError& err ) {
		FAIL( err.what() );
	}

} );


BEGIN


#include "SeqAPI.hpp"
#include "vstl.hpp"

void print_buffer( seq::ByteBuffer& bb ) {
    seq::BufferReader br = bb.getReader();
    while( br.hasNext() ) std::cout << (int) br.nextByte() << " ";
}

seq::Stream native_join_strings( seq::Stream input ) {
	seq::string str;

	for( auto& arg : input ) {
		if( arg.getDataType() == seq::DataType::String ) {
			str += arg.String().getString();
		}else{
			throw seq::RuntimeError( "Invalid argument for 'join', string expected!" );
		}
	}
	input.clear();

	seq::Stream acc;
	acc.push_back( seq::Generic( new seq::type::String( false, str.c_str() ) ) );
	return acc;
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

    CHECK( br.nextByte(), (byte) 'S' );

    { // bool
    	seq::TokenReader tr = br.next();
        CHECK( (byte) tr.getDataType(), (byte) seq::DataType::Bool );
        CHECK( tr.isAnchored(), true );
        CHECK( tr.getGeneric().Bool().getBool(), true );
    }

    { // small number
        seq::TokenReader tr = br.next();
        CHECK( (byte) tr.getDataType(), (byte) seq::DataType::Number );
        CHECK( tr.isAnchored(), false );
        CHECK( tr.getGeneric().Number().getLong(), (long) 12 );
    }

    { // fraction
        seq::TokenReader tr = br.next();
        CHECK( (byte) tr.getDataType(), (byte) seq::DataType::Number );
        CHECK( tr.isAnchored(), true );
        CHECK( tr.getGeneric().Number().getDouble(), (double) 0.5 );
    }

    { // big number
        seq::TokenReader tr = br.next();
        CHECK( (byte) tr.getDataType(), (byte) seq::DataType::Number );
        CHECK( tr.isAnchored(), false );
        CHECK( tr.getGeneric().Number().getLong(), (long) 324221312413 );
    }

    { // string
        seq::TokenReader tr = br.next();
        CHECK( (byte) tr.getDataType(), (byte) seq::DataType::String );
        CHECK( tr.isAnchored(), true );
        CHECK_ELSE( tr.getGeneric().String().getString(), seq::string( (byte*) "Hello World!" ) ) {
        	FAIL( "Invalid string!" );
        }
    }

    { // type
        seq::TokenReader tr = br.next();
        CHECK( (byte) tr.getDataType(), (byte) seq::DataType::Type );
        CHECK( tr.isAnchored(), false );
        CHECK( (byte) tr.getGeneric().Type().getType(), (byte) seq::DataType::Number );
    }

    { // VM call
        seq::TokenReader tr = br.next();
        CHECK( (byte) tr.getDataType(), (byte) seq::DataType::VMCall );
        CHECK( tr.isAnchored(), false );
        CHECK( (byte) tr.getGeneric().VMCall().getCall(), (byte) seq::type::VMCall::CallType::Return );
    }

    { // arg
        seq::TokenReader tr = br.next();
        CHECK( (byte) tr.getDataType(), (byte) seq::DataType::Arg );
        CHECK( tr.isAnchored(), false );
        CHECK( tr.getGeneric().Arg().getLevel(), (byte) 3 );
    }

    { // name (variable)
        seq::TokenReader tr = br.next();
        CHECK( (byte) tr.getDataType(), (byte) seq::DataType::Name );
        CHECK( tr.isAnchored(), false );
        seq::Generic generic = tr.getGeneric();
        auto& name = generic.Name();
        CHECK( name.getDefine(), true );
        CHECK_ELSE( name.getName(), seq::string( (byte*) "name" ) ) {
            FAIL( "Invalid string! - " + std::string( (char*) name.getName().c_str() ) );
        }
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
    	auto g = tr.getGeneric();
        seq::BufferReader func_br = tr.getGeneric().Function().getReader();
        CHECK( func_br.nextByte(), (byte) 'A' );
        CHECK( func_br.nextByte(), (byte) 'B' );
        CHECK( func_br.nextByte(), (byte) 'C' );
        CHECK( func_br.nextByte(), (byte) 0 );
    }

    {
    	auto g = tr.getGeneric();
        seq::BufferReader func_br = g.Function().getReader();
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

    CHECK( (byte) tr.getGeneric().Expression().getOperator(), (byte) seq::ExprOperator::Addition );

    {
    	seq::Generic generic = tr.getGeneric();
        seq::BufferReader expr_br = generic.Expression().getLeftReader();
        CHECK( expr_br.nextByte(), (byte) 'A' );
        CHECK( expr_br.nextByte(), (byte) 'B' );
        CHECK( expr_br.nextByte(), (byte) 'C' );
        CHECK( expr_br.nextByte(), (byte) 0 );
    }

    {
    	seq::Generic generic = tr.getGeneric();
        seq::BufferReader expr_br = generic.Expression().getRightReader();
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

        seq::Generic generic = tr.getGeneric();
        std::vector< seq::FlowCondition* > fcs = generic.Flowc().getConditions();

        CHECK( fcs.size(), (size_t) 2 );
        CHECK( (byte) fcs.at(0)->type, (byte) seq::FlowCondition::Type::Range );
        CHECK( fcs.at(0)->a.Number().getLong(), (long) 12);
        CHECK( fcs.at(0)->b.Number().getLong(), (long) 24);
        CHECK( (byte) fcs.at(1)->type, (byte) seq::FlowCondition::Type::Value );
        CHECK( fcs.at(1)->a.Bool().getBool(), false );
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

    seq::TokenReader tr = br.next();
    CHECK( (byte) tr.getDataType(), (byte) seq::DataType::Stream );
    CHECK( tr.isAnchored(), true );

    auto g = tr.getGeneric();
    seq::BufferReader sbr = g.Stream().getReader();
    CHECK( sbr.nextByte(), (byte) 'A' );
    CHECK( sbr.nextByte(), (byte) 'B' );
    CHECK( sbr.nextByte(), (byte) 'C' );
    CHECK( sbr.nextByte(), (byte) 0 );

    seq::TokenReader tr2 = br.next();
    CHECK( (byte) tr2.getDataType(), (byte) seq::DataType::Null );
    CHECK( tr2.isAnchored(), false );


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

		auto args = (seq::Stream) { seq::Generic( new seq::type::Null( false ) ) };

		seq::Executor exe;
		exe.execute( bb, args );

		CHECK( (byte) exe.getResult().getDataType(), (byte) seq::DataType::String );
		CHECK_ELSE( exe.getResult().String().getString(), seq::string( (byte*) "Hello World!" ) ) {
			FAIL( "Invalid String!" );
		}

} );


TEST( executor_hello_world_var, {

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

	auto args = (seq::Stream) { seq::Generic( new seq::type::Null( false ) ) };

	seq::Executor exe;
	exe.execute( bb, args );

	CHECK( (byte) exe.getResult().getDataType(), (byte) seq::DataType::String );
	CHECK_ELSE( exe.getResult().String().getString(), seq::string( (byte*) "Hello World!" ) ) {
		FAIL( "Invalid String!" );
	}

} );

TEST( executor_hello_world_func, {

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

	auto args = (seq::Stream) { seq::Generic( new seq::type::Null( false ) ) };

	seq::Executor exe;
	exe.execute( bb, args );

	CHECK( (byte) exe.getResult().getDataType(), (byte) seq::DataType::String );
	CHECK_ELSE( exe.getResult().String().getString(), seq::string( (byte*) "Hello World!" ) ) {
		FAIL( "Invalid String!" );
	}

} );

TEST( executor_native, {

	seq::Executor exe;
	exe.inject( (byte*) "sum", [] ( seq::Stream input ) -> seq::Stream {
		double sum = 0;
		for( auto& arg : input ) {
			sum += seq::util::numberCast( arg ).Number().getDouble();
		}
		input.clear();

		seq::Stream acc;
		acc.push_back( seq::Generic( new seq::type::Number( false, sum ) ) );
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

	auto args = (seq::Stream) { seq::Generic( new seq::type::Null( false ) ) };

	exe.execute( bb, args );

	CHECK( (byte) exe.getResult().getDataType(), (byte) seq::DataType::Number );
	CHECK( exe.getResult().Number().getLong(), (long) 10 );

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

	auto args = (seq::Stream) { seq::Generic( new seq::type::Null( false ) ) };

	seq::Executor exe;
	exe.execute( bb, args );

	CHECK( (byte) exe.getResult().getDataType(), (byte) seq::DataType::Number );
	CHECK( exe.getResult().Number().getLong(), (long) 42 );

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

	auto tokens = seq::Compiler::tokenize( "#exit << \"Hello World!\""_b );
	auto buf = seq::Compiler::assembleFunction( tokens, 0, tokens.size(), true );
	seq::ByteBuffer bb( buf.data() + 3, buf.size() - 3 );

	auto args = (seq::Stream) { seq::Generic( new seq::type::Null( false ) ) };

	seq::Executor exe;
	exe.execute( bb, args );

	CHECK( (byte) exe.getResult().getDataType(), (byte) seq::DataType::String );
	CHECK_ELSE( exe.getResult().String().getString(), seq::string( (byte*) "Hello World!" ) ) {
		FAIL( "Invalid String!" );
	}

} );

TEST( ce_hello_world_func, {

	seq::string code = (byte*) ("set var << {\n"
			"	#return << \"Hello World!\"\n"
			"}\n"
			"#exit << #var << null\n");

	auto tokens = seq::Compiler::tokenize( code );

	auto buf = seq::Compiler::assembleFunction( tokens, 0, tokens.size(), true );
	seq::ByteBuffer bb( buf.data() + 3, buf.size() - 3 );

	auto args = (seq::Stream) { seq::Generic( new seq::type::Null( false ) ) };

	seq::Executor exe;
	exe.execute( bb, args );

	CHECK( (byte) exe.getResult().getDataType(), (byte) seq::DataType::String );
	CHECK_ELSE( exe.getResult().String().getString(), seq::string( (byte*) "Hello World!" ) ) {
		FAIL( "Invalid String!" );
	}

} );

TEST( ce_expression, {

	seq::string code = (byte*) ("#exit << ( 1 + 2 + 3 + 4 )");

	auto tokens = seq::Compiler::tokenize( code );

	auto buf = seq::Compiler::assembleFunction( tokens, 0, tokens.size(), true );
	seq::ByteBuffer bb( buf.data() + 3, buf.size() - 3 );

	auto args = (seq::Stream) { seq::Generic( new seq::type::Null( false ) ) };

	seq::Executor exe;
	exe.execute( bb, args );

	CHECK( (byte) exe.getResult().getDataType(), (byte) seq::DataType::Number );
	CHECK( exe.getResult().Number().getLong(), (long) 10 );

} );


TEST( ce_expression_nested_right, {

	seq::string code = (byte*) ("#exit << ( 1 + ( 2 + 3 ) )");

	auto tokens = seq::Compiler::tokenize( code );

	auto buf = seq::Compiler::assembleFunction( tokens, 0, tokens.size(), true );
	seq::ByteBuffer bb( buf.data() + 3, buf.size() - 3 );

	auto args = (seq::Stream) { seq::Generic( new seq::type::Null( false ) ) };

	seq::Executor exe;
	exe.execute( bb, args );

	CHECK( (byte) exe.getResult().getDataType(), (byte) seq::DataType::Number );
	CHECK( exe.getResult().Number().getLong(), (long) 6 );

} );

TEST( ce_expression_nested_left, {

	seq::string code = (byte*) ("#exit << ( ( 2 + 3 ) + 1 )");

	auto tokens = seq::Compiler::tokenize( code );

	auto buf = seq::Compiler::assembleFunction( tokens, 0, tokens.size(), true );
	seq::ByteBuffer bb( buf.data() + 3, buf.size() - 3 );

	auto args = (seq::Stream) { seq::Generic( new seq::type::Null( false ) ) };

	seq::Executor exe;
	exe.execute( bb, args );

	CHECK( (byte) exe.getResult().getDataType(), (byte) seq::DataType::Number );
	CHECK( exe.getResult().Number().getLong(), (long) 6 );

} );


TEST( ce_expression_nested_double, {

	seq::string code = (byte*) ("#exit << ( ( 2 + 3 ) + ( 4 + 1 ) )");

	auto tokens = seq::Compiler::tokenize( code );

	auto buf = seq::Compiler::assembleFunction( tokens, 0, tokens.size(), true );
	seq::ByteBuffer bb( buf.data() + 3, buf.size() - 3 );

	auto args = (seq::Stream) { seq::Generic( new seq::type::Null( false ) ) };

	seq::Executor exe;
	exe.execute( bb, args );

	CHECK( (byte) exe.getResult().getDataType(), (byte) seq::DataType::Number );
	CHECK( exe.getResult().Number().getLong(), (long) 10 );

} );

TEST( ce_expression_float, {

	seq::string code = (byte*) ("#exit << ( ( 2.14 + 3.41 ) * 100 )");

	auto tokens = seq::Compiler::tokenize( code );

	auto buf = seq::Compiler::assembleFunction( tokens, 0, tokens.size(), true );
	seq::ByteBuffer bb( buf.data() + 3, buf.size() - 3 );

	auto args = (seq::Stream) { seq::Generic( new seq::type::Null( false ) ) };

	seq::Executor exe;
	exe.execute( bb, args );

	CHECK( (byte) exe.getResult().getDataType(), (byte) seq::DataType::Number );
	CHECK( exe.getResult().Number().getLong(), 555l );

} );


TEST( ce_expression_complex, {

	seq::string code = (byte*) (
			"set var << {\n"
			"	#return << @\n"
			"}\n"
			"#exit << #var << 10\n");

	auto buf = seq::Compiler::compile( code );
	seq::ByteBuffer bb( buf.data(), buf.size() );

	auto args = (seq::Stream) { seq::Generic( new seq::type::Null( false ) ) };

	seq::Executor exe;
	exe.execute( bb, args );

	CHECK( (byte) exe.getResult().getDataType(), (byte) seq::DataType::Number );
	CHECK( exe.getResult().Number().getLong(), 10l );

} );

TEST( ce_arg, {

	// MEMLEAK 56 bytes (24 direct, 32 indirect) in 1 block (direct seq::type::Function, indirect seq::BufferReader (?))

	seq::string code = (byte*) (
			"#{\n"
			"	#exit << #{\n"
			"		#return << (@ + @@) \n"
			"	} << 20\n"
			"} << 10\n");

	auto buf = seq::Compiler::compile( code );
	seq::ByteBuffer bb( buf.data(), buf.size() );

	auto args = (seq::Stream) { seq::Generic( new seq::type::Null( false ) ) };

	seq::Executor exe;
	exe.execute( bb, args );

	CHECK( (byte) exe.getResult().getDataType(), (byte) seq::DataType::Number );
	CHECK( exe.getResult().Number().getLong(), 30l );

} );


TEST( ce_string_escape_codes, {

	seq::string code = (byte*) (
			"#exit << \"\\\\\\n\\t\\\"\"");

	auto buf = seq::Compiler::compile( code );
	seq::ByteBuffer bb( buf.data(), buf.size() );

	auto args = (seq::Stream) { seq::Generic( new seq::type::Null( false ) ) };

	seq::Executor exe;
	exe.execute( bb, args );

	CHECK( (byte) exe.getResult().getDataType(), (byte) seq::DataType::String );
	CHECK_ELSE( exe.getResult().String().getString(), seq::string( (byte*) "\\\n\t\"" ) ) {
		FAIL( "Invalid String!" );
	}

} );


TEST( ce_flowc_1, {

	seq::string code = (byte*) (
			"#exit << #[true] << true");

	auto buf = seq::Compiler::compile( code );
	seq::ByteBuffer bb( buf.data(), buf.size() );

	auto args = (seq::Stream) { seq::Generic( new seq::type::Null( false ) ) };

	seq::Executor exe;
	exe.execute( bb, args );

	CHECK( (byte) exe.getResult().getDataType(), (byte) seq::DataType::Bool );
	CHECK( exe.getResult().Bool().getBool(), true );

} );

TEST( ce_flowc_2, {

	seq::string code = (byte*) (
			"#exit << #[true] << false << true");

	auto buf = seq::Compiler::compile( code );
	seq::ByteBuffer bb( buf.data(), buf.size() );

	auto args = (seq::Stream) { seq::Generic( new seq::type::Null( false ) ) };

	seq::Executor exe;
	exe.execute( bb, args );

	CHECK( (byte) exe.getResult().getDataType(), (byte) seq::DataType::Bool );
	CHECK( exe.getResult().Bool().getBool(), true );

} );

TEST( ce_flowc_3, {

	seq::string code = (byte*) (
			"#exit << #[number] << true << null << \"Hello\" << 2");

	auto buf = seq::Compiler::compile( code );
	seq::ByteBuffer bb( buf.data(), buf.size() );

	auto args = (seq::Stream) { seq::Generic( new seq::type::Null( false ) ) };

	seq::Executor exe;
	exe.execute( bb, args );

	CHECK( (byte) exe.getResult().getDataType(), (byte) seq::DataType::Number );
	CHECK( exe.getResult().Number().getLong(), 2l );

} );

TEST( ce_flowc_4, {

	seq::string code = (byte*) (
			"#exit << #[2] << true << null << \"Hello\" << 2");

	auto buf = seq::Compiler::compile( code );
	seq::ByteBuffer bb( buf.data(), buf.size() );

	auto args = (seq::Stream) { seq::Generic( new seq::type::Null( false ) ) };

	seq::Executor exe;
	exe.execute( bb, args );

	CHECK( (byte) exe.getResult().getDataType(), (byte) seq::DataType::Number );
	CHECK( exe.getResult().Number().getLong(), 2l );

} );


TEST( ce_type_cast_bool_1, {

	seq::string code = (byte*) ("#exit << #bool << 1");

	auto buf = seq::Compiler::compile( code );
	seq::ByteBuffer bb( buf.data(), buf.size() );

	auto args = (seq::Stream) { seq::Generic( new seq::type::Null( false ) ) };

	seq::Executor exe;
	exe.execute( bb, args );

	CHECK( (byte) exe.getResult().getDataType(), (byte) seq::DataType::Bool );
	CHECK( exe.getResult().Bool().getBool(), true );

} );

TEST( ce_type_cast_bool_2, {

	seq::string code = (byte*) ("#exit << #bool << 0");

	auto buf = seq::Compiler::compile( code );
	seq::ByteBuffer bb( buf.data(), buf.size() );

	auto args = (seq::Stream) { seq::Generic( new seq::type::Null( false ) ) };

	seq::Executor exe;
	exe.execute( bb, args );

	CHECK( (byte) exe.getResult().getDataType(), (byte) seq::DataType::Bool );
	CHECK( exe.getResult().Bool().getBool(), false );

} );

TEST( ce_type_cast_number_1, {

	seq::string code = (byte*) ("#exit << #number << \"123\"");

	auto buf = seq::Compiler::compile( code );
	seq::ByteBuffer bb( buf.data(), buf.size() );

	auto args = (seq::Stream) { seq::Generic( new seq::type::Null( false ) ) };

	seq::Executor exe;
	exe.execute( bb, args );

	CHECK( (byte) exe.getResult().getDataType(), (byte) seq::DataType::Number );
	CHECK( exe.getResult().Number().getLong(), 123l );

} );

TEST( ce_type_cast_number_2, {

	seq::string code = (byte*) ("#exit << #number << true");

	auto buf = seq::Compiler::compile( code );
	seq::ByteBuffer bb( buf.data(), buf.size() );

	auto args = (seq::Stream) { seq::Generic( new seq::type::Null( false ) ) };

	seq::Executor exe;
	exe.execute( bb, args );

	CHECK( (byte) exe.getResult().getDataType(), (byte) seq::DataType::Number );
	CHECK( exe.getResult().Number().getLong(), 1l );

} );

TEST( ce_type_cast_string_1, {

	seq::string code = (byte*) ("#exit << #string << true");

	auto buf = seq::Compiler::compile( code );
	seq::ByteBuffer bb( buf.data(), buf.size() );

	auto args = (seq::Stream) { seq::Generic( new seq::type::Null( false ) ) };

	seq::Executor exe;
	exe.execute( bb, args );

	CHECK( (byte) exe.getResult().getDataType(), (byte) seq::DataType::String );
	CHECK_ELSE( exe.getResult().String().getString(), seq::string( (byte*) "true" ) ) {
		FAIL( "Invalid String!" );
	}

} );

TEST( ce_type_cast_string_2, {

	seq::string code = (byte*) ("#exit << #string << 123.2");

	auto buf = seq::Compiler::compile( code );
	seq::ByteBuffer bb( buf.data(), buf.size() );

	auto args = (seq::Stream) { seq::Generic( new seq::type::Null( false ) ) };

	seq::Executor exe;
	exe.execute( bb, args );

	CHECK( (byte) exe.getResult().getDataType(), (byte) seq::DataType::String );
	CHECK( std::stod( seq::util::toStdString( exe.getResult().String().getString() ) ), 123.2 );

} );


TEST( ce_stream_tags, {

	// MEMLEAK 288 bytes in 1 block (seq::type::String)

	seq::string code = (byte*) (
			"#exit << #join << #{\n"
            "	first; #return << \"first\"\n"
            "	last; #return << \"last\"\n"
            "	#return << \"none\"\n"
            "	end; #return << \"end\"\n"
			"} << 1 << 2 << 3");

	auto buf = seq::Compiler::compile( code );
	seq::ByteBuffer bb( buf.data(), buf.size() );

	auto args = (seq::Stream) { seq::Generic( new seq::type::Null( false ) ) };

	seq::Executor exe;
	exe.inject( "join"_b, native_join_strings );
	exe.execute( bb, args );

	CHECK( (byte) exe.getResult().getDataType(), (byte) seq::DataType::String );
	CHECK_ELSE( exe.getResult().String().getString(), seq::string( (byte*) "firstnonenonelastnoneend" ) ) {
		FAIL( "Invalid String!" );
	}

} );

TEST( ce_complex_sum, {

	// MEMLEAK 24 bytes in 1 block (seq::type::Number)
	// MEMLEAK 72 bytes in 3 blocks (seq::type::Number)

	seq::string code = (byte*) (
			"#exit << #{\n"
			"	first; set x << 0\n"
			"	set x << #{\n"
			"		#return << (@@ + @)\n"
			"	} << x\n"
			"	last; #return << x\n"
			"} << 1 << 2 << 3 << 4\n"
			);

	auto buf = seq::Compiler::compile( code );
	seq::ByteBuffer bb( buf.data(), buf.size() );

	auto args = (seq::Stream) { seq::Generic( new seq::type::Null( false ) ) };

	seq::Executor exe;
	exe.execute( bb, args );

	CHECK( (byte) exe.getResult().getDataType(), (byte) seq::DataType::Number );
	CHECK( exe.getResult().Number().getLong(), 10l );

} );


REGISTER_EXCEPTION( seq_compiler_error, seq::CompilerError );
REGISTER_EXCEPTION( seq_internal_error, seq::InternalError );
REGISTER_EXCEPTION( seq_runtime_error, seq::RuntimeError );

BEGIN

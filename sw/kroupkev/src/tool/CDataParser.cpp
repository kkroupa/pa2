#include "CDataParser.hpp"

/*
 * Trims whitespaces to the left of the text.
 * @param[in, out] s string that is to be trimmed
 */
string & CDataParser::TrimSpaceLeft ( string & s ) {
	auto it = std::find_if( s.begin( ), s.end( ), [ ] ( char c ) { return ! std::isspace<char>( c, std::locale::classic( ) ); } );
	s.erase( s.begin( ), it );
	return s;
}

/**
 * Trims whitespaces to the right of the text.
 * @param[in, out] s string that is to be trimmed
 */
string & CDataParser::TrimSpaceRight ( string & s ) {
	auto it = std::find_if( s.rbegin( ), s.rend( ), [ ] ( char c ) { return ! std::isspace<char>( c, std::locale::classic( ) ); } );
	s.erase( it.base( ), s.end( ) );
	return s;
}

/**
 * Author of the following 3 methods is Techie Delight.
 * Source: https://www.techiedelight.com/trim-string-cpp-remove-leading-trailing-spaces/.
 * Trims all surrounding whitespaces of the text.
 * @param[in, out] s string that is to be trimmed
 */
string & CDataParser::TrimSurroundingSpace ( string & s ) {
	return TrimSpaceLeft( TrimSpaceRight( s ) );
}

/**
 * Trims all whitespaces inside of the given text.
 * @param[in, out] token string that is to be trimmed
 * @param[in] delimiter char delimiter (not just a whitespace)
 */
void CDataParser::TrimAllSpaces ( string & token, const char & delimiter ) {
	token.erase( remove( token.begin( ), token.end( ), delimiter ), token.end( ) );
}

/**
 * Tokenizes given string based on configuration parameters.
 * @param[in] s string that is going to be tokenized
 * @param[in] allowQuotes if true, allows quotes in tokenized parts
 * @param[in] allowSpace if true, allows spaces in tokenized parts
 * @param[in] delimiter char delimiter (not just a whitespace)
 * @return tokenized string vector
 */
vector<string>
CDataParser::Split ( string & s, const bool & allowQuotes, const bool & allowSpace, const char & delim ) {
	if ( ! allowSpace )
		TrimAllSpaces( s, ' ' );
	vector<string> tokenizedString;
	bool quoteFlag = false;;
	tokenizedString.emplace_back( "" );
	for ( char i : s ) {
		if ( i == '\"' ) {
			quoteFlag = ! quoteFlag;
			continue;
		}
		if ( i == ',' && ! quoteFlag )
			tokenizedString.emplace_back( "" );
		else
			tokenizedString[ tokenizedString.size( ) - 1 ] += i;
	}
	for ( string & i : tokenizedString )
		TrimSurroundingSpace( i );
	return tokenizedString;
}

/**
 * Tokenizes given string based on given delimeter.
 * @param[in,out] s string reference to be tokenized
 */
vector<string>
CDataParser::Split ( string & s, const char & delim = ',' ) {
	std::replace( s.begin( ), s.end( ), delim, ' ');
	vector<string> tokenizedStr;
	stringstream ss ( s );
	if ( ! ss )
		return vector<string> { };
	string temp;
	while ( ss >> temp )
	    tokenizedStr.push_back(temp);
	return tokenizedStr;
}

/**
 * Extracts file name from the path.
 * @param[in, out] filePath path to the given file.
 */
string & CDataParser::SplitFileName ( string & filePath ) {
	const size_t pathSlash = filePath.find_last_of( "\\/" );
	if ( string::npos != pathSlash )
		filePath.erase( 0, pathSlash + 1 );

	const size_t extensionDot = filePath.rfind( '.' );
	if ( string::npos != extensionDot )
		filePath.erase( extensionDot );

	return filePath;
}

/**
 * Extracts file extension from the path.
 * @param[in, out] input full file name with extension.
 */
string CDataParser::GetFileExtension ( const string & input ) {
	if ( input.find_last_of( '.' ) != std::string::npos )
		return input.substr( input.find_last_of( '.' ) + 1 );
	return "";
}

/**
 * Selects the corrent parsing format and passes the stream.
 * @param[in, out] db database to be inserted to
 * @param[in, out] ifs input filestream
 * @param[in, out] filePath path to input file with the table
 * @return false if appropriate parsing format was not found, delegated otherwise
 */
bool CDataParser::ParseTable ( CDatabase & db, ifstream & ifs, string & filePath ) {
	string extension = GetFileExtension( filePath );
	if ( extension == "csv" || extension == "CSV" )
		return ParseCSV( db, ifs, filePath );
	return false;
}

/**
 * CSV import.
 * Parameters are same as CDataParser::ParseTable, since they are delegated directly here.
 * @return true, if table was parsed and added to the database
 */
bool CDataParser::ParseCSV ( CDatabase & db, ifstream & ifs, string & filePath ) {
	string tmp;

	// table existence check
	SplitFileName( filePath );
	if ( db.TableExists( filePath ) )
		return false;

	// first row - table column data types
	getline( ifs, tmp );
	if ( tmp.empty( ) || * ( tmp.end( ) - 1 ) == ',' )
		return false;
	vector<string> columnTypes = Split( tmp, false, false );
	for ( const string & i : columnTypes ) {
		if ( i.empty( ) )
			return false;
		if ( i != CLog::TYPE_STRING && i != CLog::TYPE_INT && i != CLog::TYPE_DOUBLE ) {
			CLog::HighlightedMsg( CLog::DP, filePath, string( "" ).append( CLog::DP_NO_DATATYPES ).append( "\u001b[0m" ) );
			return false;
		}
	}
	if ( ifs.eof( ) )
		return false;
	tmp.clear( );

	// second row - table column data types
	size_t requiredColumns = columnTypes.size( );
	getline( ifs, tmp );
	if ( tmp.empty( ) || * ( tmp.end( ) - 1 ) == ',' )
		return false;

	vector<string> columnNames = Split( tmp, false, false );
	for ( const string & i : columnNames )
		if ( i.empty( ) )
			return false;
	if ( ifs.eof( ) || columnNames.size( ) != requiredColumns )
		return false;
	tmp.clear( );

	// save the column header information
	vector<CCell *> header;
	header.reserve( requiredColumns );
	for ( string & i : columnNames )
		header.push_back( new CString( std::move( i ) ) );
	auto * parsedResult = new CTable { header };
	int lines = 2;

	// table rows
	while ( getline( ifs, tmp ) ) {
		// empty lines..
		if ( tmp.empty( ) ) {
			CLog::BoldMsg( CLog::DP, filePath, string( "" ).append( CLog::DP_EMPTY_LINE ).append( to_string( lines ) ).append( ".\u001b[0m" ) );
			delete parsedResult;
			return false;
		}

		// wrong formatting
		if ( * ( tmp.end( ) - 1 ) == ',' ) {
			CLog::BoldMsg( CLog::DP, filePath, string( "" ).append( CLog::DP_LINE_MISMATCH ).append( to_string( lines ) ).append( ".\u001b[0m" ) );
			delete parsedResult;
			return false;
		}

		// checks for number of columns to insert
		vector<string> newRow = Split( tmp, false, true );
		if ( newRow.size( ) != requiredColumns ) {
			CLog::BoldMsg( CLog::DP, filePath, string( "" ).append( CLog::DP_LINE_MISMATCH ).append( to_string( lines ) ).append( ".\u001b[0m" ) );
			delete parsedResult;
			return false;
		}

		// parse to appropriate data types
		int cnt = 0;
		vector<CCell *> newTypedRow;
		try {
			for ( const string & i : newRow ) {
				if ( columnTypes[ cnt ] == CLog::TYPE_STRING ) {
					newTypedRow.push_back( new CString( i ) );
				}
				else if ( columnTypes[ cnt ] == CLog::TYPE_INT ) {
					int x = stoi( i );
					newTypedRow.push_back( new CInt( x ) );
				}
				else {
					double x = stod( i );
					newTypedRow.push_back( new CDouble( x ) );
				}
				++ cnt;
			}
		} catch ( std::logic_error const & e ) {
			delete parsedResult;
			for ( const auto & i : newTypedRow )
				delete i;
			 return false;
		}
		if ( ! parsedResult->InsertShallowRow( newTypedRow ) )
			return false;
		++ lines;
	}

	if ( lines == 2 )
		return false;
	db.InsertTable( filePath, parsedResult );
	return true;
}
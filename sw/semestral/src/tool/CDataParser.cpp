#include "CDataParser.hpp"

const string CDataParser::TYPE_STRING = "string";
const string CDataParser::TYPE_DOUBLE = "double";
const string CDataParser::TYPE_INT = "int";

string & CDataParser::TrimSpaceLeft ( string & s ) {
	auto it = std::find_if( s.begin( ), s.end( ),
	                        [ ] ( char c ) {
		                        return ! std::isspace<char>( c, std::locale::classic( ) );
	                        } );
	s.erase( s.begin( ), it );
	return s;
}

string & CDataParser::TrimSpaceRight ( string & s ) {
	auto it = std::find_if( s.rbegin( ), s.rend( ),
	                        [ ] ( char c ) {
		                        return ! std::isspace<char>( c, std::locale::classic( ) );
	                        } );
	s.erase( it.base( ), s.end( ) );
	return s;
}

string & CDataParser::TrimSurroundingSpace ( string & s ) {
	return TrimSpaceLeft( TrimSpaceRight( s ) );
}

void CDataParser::TrimAllSpaces ( string & token, const char & delimiter ) {
	token.erase( remove( token.begin( ), token.end( ), delimiter ), token.end( ) );
}

vector<string>
CDataParser::Split ( string & s, const bool & allowQuotes, const bool & allowSpace, const char & delim ) {
	if ( ! allowSpace )
		TrimAllSpaces( s, ' ' );
	vector<string> tokenizedString;
	bool quoteFlag = false;
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

string & CDataParser::SplitFileName ( string & filePath ) {
	const size_t pathSlash = filePath.find_last_of( "\\/" );
	if ( string::npos != pathSlash )
		filePath.erase( 0, pathSlash + 1 );

	const size_t extensionDot = filePath.rfind( '.' );
	if ( string::npos != extensionDot )
		filePath.erase( extensionDot );

	return filePath;
}

string CDataParser::GetFileExtension ( const string & input ) {
	if ( input.find_last_of( '.' ) != std::string::npos )
		return input.substr( input.find_last_of( '.' ) + 1 );
	return "";
}

bool CDataParser::ParseCSV ( CDatabase & db, ifstream & ifs, string & filePath ) {
	string tmp;

	// table existence check
	SplitFileName( filePath );
	if ( db.TableExists( filePath ) )
		return false;

	// table column data types
	getline( ifs, tmp );
	if ( * ( tmp.end( ) - 1 ) == ',' )
		return false;
	vector<string> columnTypes = Split( tmp, false, false );
	for ( const string & i : columnTypes ) {
		if ( i.empty( ) )
			return false;
		if ( i != TYPE_STRING && i != TYPE_INT && i != TYPE_DOUBLE ) {
			CLog::HighlightedMsg( CLog::DP, filePath,
			                      string( "" ).append( CLog::DP_NO_DATATYPES ).append( "\u001b[0m" ) );
			return false;
		}
	}
	if ( ifs.eof( ) )
		return false;
	tmp.clear( );

	// table column names
	size_t requiredColumns = columnTypes.size( );

	getline( ifs, tmp );
	if ( * ( tmp.end( ) - 1 ) == ',' )
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
		if ( * ( tmp.end( ) - 1 ) == ',' || tmp.empty( ) ) {
			CLog::HighlightedMsg( CLog::DP, filePath,
			                      string( "" ).append( CLog::DP_LINE_MISMATCH ).append( to_string( lines ) ).append(
					                      ".\u001b[0m" ) );
			return false;
		}
		++ lines;

		// checks for number of columns to insert
		vector<string> newRow = Split( tmp, false, true );
		if ( newRow.size( ) != requiredColumns ) {
			CLog::HighlightedMsg( CLog::DP, filePath,
			                      string( "" ).append( CLog::DP_LINE_MISMATCH ).append( to_string( lines ) ).append(
					                      ".\u001b[0m" ) );
			return false;
		}

		// parse to appropriate data types
		int cnt = 0;
		vector<CCell *> newTypedRow;
		for ( const string & i : newRow ) {
			if ( columnTypes[ cnt ] == TYPE_STRING )
				newTypedRow.push_back( new CString( i ) );
			else if ( columnTypes[ cnt ] == TYPE_INT )
				newTypedRow.push_back( new CInt( stoi( i ) ) );
			else
				newTypedRow.push_back( new CDouble( stod( i ) ) );
			++ cnt;
		}

		if ( ! parsedResult->InsertRow( newTypedRow ) )
			return false;
	}
	db.InsertTable( filePath, parsedResult );
	return true;
}

bool CDataParser::ParseTable ( CDatabase & db, ifstream & ifs, string & filePath ) {
	string extension = GetFileExtension( filePath );
	if ( extension == "csv" || extension == "CSV" )
		return ParseCSV( db, ifs, filePath );
//	else if -- new table formats to be added
	return false;
}
#pragma once

#include <iostream>
#include <iomanip>
#include <string>

/**
 * This module works as an application logger, storing messages to print, providing better UI.
 */
namespace CLog {
	using namespace std;

	static constexpr int APP_PADDING ( )
	{ return 14; }
	
	const string APP_LINE_HEIGHT    = "\n";
	const string APP_COLOR_GOOD     = "\033[0;32m";
	const string APP_COLOR_RESULT   = "\033[1;36m";
	const string APP_COLOR_BAD      = "\u001b[31m";
	const string APP_COLOR_RESET    = "\u001b[0m";

	const string APP                = "APP";
	const string APP_EX_EXIT        = "Exiting the application due to exception error.";

	const string FM                 = "FILE MANAGER";
	const string FM_CFG_FOUND       = "Configuration file found.";
	const string FM_CFG_NOT_FOUND   = string( APP_COLOR_BAD ).append( "Configuration file not found." ).append( APP_COLOR_RESET );
	const string FM_CFG_FAILED      = string( APP_COLOR_BAD ).append( "Configuration file wasn't imported properly." ).append( APP_COLOR_RESET );
	const string FM_LOADING         = "Loading tables from the file into database..";
	const string FM_IMPORT_OK       = string( APP_COLOR_GOOD ).append( " successfully imported!" ).append( APP_COLOR_RESET );
	const string FM_IMPORT_FAIL     = string( APP_COLOR_BAD ).append(" is in incorrect format or doesn't exist." ).append( APP_COLOR_RESET );

	const string DP                 = "DATA PARSER";
	const string DP_LINE_MISMATCH   = string( APP_COLOR_BAD ).append(" number of columns does not match at line " );
	const string DP_NO_DATATYPES    = string( APP_COLOR_BAD ).append(" data types for the columns were not provided." );
	const string DP_EMPTY_LINE      = string( APP_COLOR_BAD ).append(" incorrect formatting - empty line at " );

	const string CON                = "CONSOLE";
	const string CON_START          = "Starting console..";
	const string CON_INVALID_QUERY  = string( APP_COLOR_BAD ).append(" invalid command." ).append( APP_COLOR_RESET );
	const string CON_EOF_DETECTED   = "EOF detected. Exiting application.";
	const string CON_EXIT           = "Exiting application.";
	const string CON_LISTING_T      = "Listing existing tables..";
	const string CON_LISTING_Q      = "Listing saved queries..";

	const string TAB_NO_BODY        = string( APP_COLOR_BAD ).append( "Unable to render - table body is missing." ).append( APP_COLOR_RESET );
	const string TAB_NO_DATA        = "The table doesn't have any columns.";
	const string TAB_INVALID_INDEX  = "Invalid index.";

	const string QP                 = "QUERY PARSER";
	const string QP_NO_SUCH_TABLE   =  string( APP_COLOR_BAD ).append(" table was not found." ).append( APP_COLOR_RESET );
	const string QP_NO_SUCH_COL     =  string( APP_COLOR_BAD ).append(" column was not found." ).append( APP_COLOR_RESET );
	const string QP_QUERY_ADDED     =  string( APP_COLOR_GOOD ).append(" query was added!" ).append( APP_COLOR_RESET );
	const string QP_TABLE_EXISTS    =  string( APP_COLOR_BAD ).append(" name already taken (please use TABLES or QUERIES to prevent this)." ).append( APP_COLOR_RESET );
	const string QP_INVALID_CON     =  string( APP_COLOR_BAD ).append(" the condition clause is missing an operand." ).append( APP_COLOR_RESET );
	const string QP_INVALID_REL     =  string( APP_COLOR_BAD ).append(" the condition clause is missing a relation operator." ).append( APP_COLOR_RESET );
	const string QP_CON_PARSE_ERROR =  string( APP_COLOR_BAD ).append(" was unable to convert constant to a correct type." ).append( APP_COLOR_RESET );
	const string QP_INVALID_OPER    =  string( APP_COLOR_BAD ).append(" invalid comparison operator was entered." ).append( APP_COLOR_RESET );
	const string QP_EMPTY_RESULTS   =  string( APP_COLOR_BAD ).append( "The resulting query is empty." ).append( APP_COLOR_RESET );
	const string QP_DUP_COL         = string( APP_COLOR_BAD ).append( "Unable to perform operation - the table has column duplicates." ).append( APP_COLOR_RESET );
	const string QP_NO_COMMON_COL   =  string( APP_COLOR_BAD ).append( "The tables do not share any columns." ).append( APP_COLOR_RESET );


	/**
	 * Standart message.
	 * @param[in] branch module from where the message is originated
	 * @param[in] message log message string
	 * @param[in, out] ost output stream to log to.
	 */
	inline void Msg ( const string & branch, const string & message, const string & extraPad = APP_LINE_HEIGHT, ostream & ost = cout ) {
		ost << setw( APP_PADDING( ) ) << right << branch << ": " << message << extraPad << endl;
	}

	/**
	 * Highlighted message
	 * @param[in] branch module from where the message is originated
	 * @param[in] highlighted highlighted part of the message
	 * @param[in] message log message string
	 * @param[in, out] ost output stream to log to.
	 */
	inline void
	HighlightedMsg ( const string & branch, const string & highlighted, const string & message,
	                 const string & extraPad = APP_LINE_HEIGHT, ostream & ost = cout ) {
		ost << setw( APP_PADDING( ) ) << right << branch << ": " << "\u001b[1m"
		<< string( "\"\u001b[1m" ).append( highlighted ).append( "\u001b[0m\"" ) << message << extraPad << endl;
	}

	/**
	 * Bold message
	 * @param[in] branch module from where the message is originated
	 * @param[in] highlighted highlighted part of the message
	 * @param[in] message log message string
	 * @param[in, out] ost output stream to log to.
	 */
	inline void
	BoldMsg ( const string & branch, const string & highlighted, const string & message,
	          const string & extraPad = APP_LINE_HEIGHT, ostream & ost = cout ) {
		ost << setw( APP_PADDING( ) ) << right << branch << ": " << "\u001b[1m"
		<< string( "\u001b[1m" ).append( highlighted ).append( "\u001b[0m" ) << message << extraPad << endl;
	}
}
#pragma once

#include <string>
#include <map>
#include <iostream>
#include <vector>

#include "CLog.hpp"
#include "CConsole.hpp"
#include "../tool/CDataParser.hpp"
#include "../database/CDatabase.hpp"
//
#include "../query/CSelection.hpp"
#include "../query/CAlias.hpp"
#include "../query/CProjection.hpp"
#include "../query/CNaturalJoin.hpp"
#include "../query/CJoin.hpp"
#include "../query/CUnion.hpp"
#include "../query/CIntersect.hpp"
#include "../query/CMinus.hpp"
#include "../query/CCartesian.hpp"

using namespace std;

/**
 * This module's responsibility is to parse string queries entered by the user.
 */
class CQueryParser {
private:
	CDatabase & m_Database;
	vector<string> m_Operators;

public:
	explicit CQueryParser( CDatabase & ref );

	static bool ReadQSave ( const string & queryDetails, const char & saveDelimiter, string & output );
	static bool ReadQName ( const string & fullQuery, string & output );
	static bool ReadQParenthesis ( const string & queryDetails, const char & delStart, const char & delEnd, size_t & stringPos, string & output );

	int ProcessQuery ( const string & basicString) const;
	bool ValidateConditionSyntax ( const string & query, CCondition * output ) const;
};
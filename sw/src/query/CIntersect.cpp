#include "CIntersect.hpp"

CIntersect::CIntersect ( CDatabase & ref, const pair<string, string> & tableNames )
		: m_Database( ref ),
		  m_QueryResult( nullptr ),
		  m_QuerySaveName( "" ),
		  m_TableNames( std::make_pair( tableNames.first, tableNames.second ) ),
		  m_Resolved( false ) { }

CIntersect::~CIntersect ( ) {
	delete m_QueryResult;
}

bool CIntersect::Evaluate ( ) {
	// query/table search
	if ( ( m_Operands.first.m_TRef = m_Database.GetTable( m_TableNames.first ) ) != nullptr ) { }
	else if ( ( m_Operands.first.m_QRef = m_Database.GetTableQ( m_TableNames.first ) ) != nullptr ) {
		m_Operands.first.m_Origin   = m_Operands.first.m_QRef;
		m_Operands.first.m_TRef = m_Operands.first.m_QRef->GetQueryResult( );
	} else {
		return false;
	}
	if ( m_Operands.first.m_TRef->HasDuplicateColumns( ) ) {
		CLog::Msg( CLog::QP, CLog::QP_DUP_COL );
		return false;
	}

	if ( ( m_Operands.second.m_TRef = m_Database.GetTable( m_TableNames.second ) ) != nullptr ) { }
	else if ( ( m_Operands.second.m_QRef = m_Database.GetTableQ( m_TableNames.second ) ) != nullptr ) {
		m_Operands.second.m_Origin   = m_Operands.second.m_QRef;
		m_Operands.second.m_TRef = m_Operands.second.m_QRef->GetQueryResult( );
	} else {
		return false;
	}
	if ( m_Operands.second.m_TRef->HasDuplicateColumns( ) ) {
		CLog::Msg( CLog::QP, CLog::QP_DUP_COL );
		return false;
	}

	// header info check
	m_Operands.first.m_TRef->SortColumns( );
	m_Operands.second.m_TRef->SortColumns( );
	if ( ! m_Operands.first.m_TRef->HasIdenticalHeader( m_Operands.second.m_TRef ) ) {
		CLog::Msg( CLog::QP, CLog::QP_DIFF_HEADER );
		return false;
	}

	size_t tmp = m_Operands.first.m_TRef->GetColumnCount( );
	for ( size_t j = 0; j < tmp; ++ j ) {
		if ( m_Operands.first.m_TRef->GetColumnType( j ) != m_Operands.second.m_TRef->GetColumnType( j ) ) {
			CLog::Msg( CLog::QP, CLog::QP_DIFF_TYPE );
			return false;
		}
	}

	m_QueryResult = new CTable ( m_Operands.first.m_TRef->GetColumnNames( ) );

	// transformation for easier row access
	vector<vector<CCell *>> tA, tB;
	CTable::SortRows( ( tA = m_Operands.first.m_TRef->Transform( ) ) );
	CTable::SortRows( ( tB = m_Operands.second.m_TRef->Transform( ) ) );
	vector<vector<CCell *>> res ( tA.size( ) + tB.size( ) );
	set_intersection( tA.begin( ), tA.end( ), tB.begin( ), tB.end( ), res.begin( ), CTable::RowComparator( ) );

	tmp = 0;
	for ( const auto & i : res ) {
		if ( i.empty( ) )
			continue;
		if ( ! m_QueryResult->InsertShallowRow( CTable::GetDeepRow( i ) ) )
			return false;
		++ tmp;
	}
	if ( tmp == 0 ) {
		CLog::Msg( CLog::QP, CLog::QP_EMPTY_RESULTS );
		return false;
	}

	m_Resolved = true;
	return true;
}

CTable * CIntersect::GetQueryResult ( ) {
	return m_QueryResult;
}

void CIntersect::ArchiveQueryName ( const string & name ) {
	if ( m_QuerySaveName != name )
		m_QuerySaveName = name;
}

string CIntersect::GetSQL ( ) const {
	if ( ! m_Resolved )
		return "";
	CTableQuery * origin;
	string output;
	string tmp = string( "( SELECT " ).append( CLog::APP_COLOR_RESULT ).append( "*" ).append( CLog::APP_COLOR_RESET ).append( " FROM ");
	origin = m_Operands.first.m_Origin;
	output += "( ";
	output += origin ? origin->GetSQL( ) : string( tmp ).append( CLog::APP_COLOR_RESULT ).append( m_TableNames.first ).append( CLog::APP_COLOR_RESET ).append( " )");
	output += " INTERSECT ";
	origin = m_Operands.second.m_Origin;
	output += origin ? origin->GetSQL( ) : string( tmp ).append( CLog::APP_COLOR_RESULT ).append( m_TableNames.second ).append( CLog::APP_COLOR_RESET ).append( " )");
	output += " )";
	return output;
}

bool CIntersect::IsDerived ( ) const {
	return false;
}
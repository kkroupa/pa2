#ifndef __PROGTEST__

#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <cassert>
#include <cstring>
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <stdexcept>
#include <algorithm>

using namespace std;
#endif /* __PROGTEST__ */

/*
 * Verze k 1/5/2020:
 * Preklad dokumentace do anglictiny, jak code-review hodnoceni doporucuje.
 * Odstraneni zbytecnych komentaru.
 * Odstraneni debugovacich nastroju, "produkcni" verze.
*/

struct CWallet {
    string code;
    int income;
    int expense;
    explicit CWallet ( string code = "", int income = 0, int expenses = 0 )
    : code( std::move( code ) ), income( income ), expense( expenses ) { };
};

struct CCitizen {
    string name;
    string addr;
    CWallet acc;

    explicit CCitizen ( string name, string addr, CWallet acc )
    : name( std::move( name ) ), addr( std::move( addr ) ), acc( std::move( acc ) ) { };

    /* Citizen comparator based on account codes. */
    static bool CmpAcc ( const CCitizen * x, const CCitizen * y ) {
        return x->acc.code > y->acc.code;
    }

    /* Citizen comparator based on names, addresses if names match. */
    static bool CmpNameAddr ( const CCitizen * x, const CCitizen * y ) {
        if ( x->name == y->name )
            return x->addr < y->addr;
        return x->name < y->name;
    }
};

class CIterator {
private:
    vector<CCitizen *>::const_iterator it;
    const vector<CCitizen *> & origin_ref;

public:
    explicit CIterator ( const vector<CCitizen *> & ref ) : origin_ref( ref ) {
		/*
		 * The constructor saves the reference to the original vector of pointers.
		 * The iterator is then constructed based on it.
		*/
        it = origin_ref.begin( );
    }

    bool AtEnd ( ) const {
        return it == origin_ref.end( );
    }
    void Next ( ) {
        if ( !AtEnd( ) ) it++;
    }
    string Name ( ) const {
        return ( !AtEnd( ) ? ( * it )->name : "" );
    }
    string Addr ( ) const {
        return ( !AtEnd( ) ? ( * it )->addr : "" );
    }
    string Account ( ) const {
        return ( !AtEnd( ) ? ( * it )->acc.code : "" );
    }
};

/*
 * Each instance of this object contains following vectors pointing at the same set of registered citizens:
 * db_code - pointers sorted by bank accounts.
 * db_name - pointers sorted by name/surname, or addresses.
 * This approach gives us ability to binary search trough our register based on current criterion.
 * Example:
 * Income( "Pepa" ) => object knows to search in sorted-by-name container db_name, because we're searching for a name.
*/
class CTaxRegister {
private:
    vector<CCitizen *> db_code;
    vector<CCitizen *> db_name;
    CCitizen * tmp_cit = nullptr;

public:
    ~CTaxRegister ( ) {
        /* Same objects, two vectors referencing them -> freeing up only one is enough. */
        for ( const CCitizen * i : db_code )
            delete i;
    }

    /*
     * Method checks the user input and adds new references to the containers.
     * @param[in] tmp: User input.
     * @return: true if user input was correct and successfully added into containers.
    */
    bool ValidateCitizen ( CCitizen * tmp ) {
        if ( db_code.empty( ) && db_name.empty( ) ) {
            db_code.push_back( tmp );
            db_name.push_back( tmp );
            return true;
        }

        /* Bank account duplicate check */
        auto tmp_it_acc = lower_bound( db_code.begin( ), db_code.end( ), tmp, CCitizen::CmpAcc );
        if (
            tmp_it_acc != db_code.end( ) &&
            ( ( * tmp_it_acc )->acc.code == tmp->acc.code ||
            ( ( * tmp_it_acc )->name == tmp->name && ( * tmp_it_acc )->addr == tmp->addr ) )
        ) {
            delete tmp;
            return false;
        }

        /* Name/address duplicate check */
        auto tmp_it_name = lower_bound( db_name.begin( ), db_name.end( ), tmp, CCitizen::CmpNameAddr );
        if (
            tmp_it_name != db_name.end( ) &&
            ( ( * tmp_it_name )->acc.code == tmp->acc.code ||
            ( ( * tmp_it_name )->name == tmp->name && ( * tmp_it_name )->addr == tmp->addr ) )
        ) {
            delete tmp;
            return false;
        }

        /* Searching for the positions to insert to */
        if ( tmp_it_acc != db_code.end( ) )
            db_code.insert( tmp_it_acc, tmp );
        else
            db_code.push_back( tmp );

        if ( tmp_it_name != db_name.end( ) )
            db_name.insert( tmp_it_name, tmp );
        else
            db_name.push_back( tmp );

        return true;
    }

	/*
	 * Method manages the expense/income on the citizen account.
	 * Many methods bellow  use this one, each one with its own configuration, hence there are more parameters.
	 *
	 * Configuration:
	 * @param[in] code_input: Transaction is based on bank account code (true), name/address (false).
	 * @param[in]    expense: Transaction is of type expense (true), income (false).
	 *
	 * Inputs:
	 * @param[in] name, addr: Citizen name/address.
	 * @param[in]       code: Bank account code of the citizen.
	 * @param[in]     amount: Amount.
	 * @return true, if user input was corrent and transaction was successful.
	*/
    bool ManageWallet ( const bool code_input, const bool expense,
                        const string & name, const string & addr,
                        const string & code,
                        const int & amount
    ) {
        if ( amount <= 0 || db_code.empty( ) )
            return false;

        /* Citizen searching */
        tmp_cit = new CCitizen( name, addr, CWallet( code ) );
        vector<CCitizen *>::iterator tmp_it;
        if ( code_input ) {
            tmp_it = lower_bound( db_code.begin( ), db_code.end( ), tmp_cit, CCitizen::CmpAcc );
            if ( tmp_it == db_code.end( ) || ( ( * tmp_it )->acc.code != tmp_cit->acc.code ) ) {
                delete tmp_cit;
                return false;
            }
        } else {
            tmp_it = lower_bound( db_name.begin( ), db_name.end( ), tmp_cit, CCitizen::CmpNameAddr );
            if ( tmp_it == db_name.end( ) ||
                ( ( * tmp_it )->name != tmp_cit->name ) ||
                ( ( * tmp_it )->addr != tmp_cit->addr ) )
            {
                delete tmp_cit;
                return false;
            }
        }

        expense ? ( * tmp_it )->acc.expense += amount : ( * tmp_it )->acc.income += amount;
        delete tmp_cit;
        return true;
    }

    bool Birth ( const string & name, const string & addr, const string & account ) {
        if ( name.empty( ) || addr.empty( ) || account.empty( ) )
            return false;
        return ValidateCitizen( new CCitizen( name, addr, CWallet( account, 0 ) ) );
    }

    bool Death ( const string & name, const string & addr ) {
        if ( db_code.empty( ) || name.empty( ) || addr.empty( ) )
            return false;
        string code;
        tmp_cit = new CCitizen( name, addr, CWallet( ) );
        auto tmp_it_name = lower_bound( db_name.begin( ), db_name.end( ), tmp_cit, CCitizen::CmpNameAddr );
        if ( tmp_it_name == db_name.end( ) ||
            ( ( * tmp_it_name )->name != tmp_cit->name ) ||
            ( ( * tmp_it_name )->addr != tmp_cit->addr ) ) {
            delete tmp_cit;
            return false;
        }
        code = ( * tmp_it_name )->acc.code;
        tmp_cit->acc.code = code;
        auto tmp_it_code = lower_bound( db_code.begin( ), db_code.end( ), tmp_cit, CCitizen::CmpAcc );
        delete tmp_cit;
        tmp_cit = * tmp_it_code;

        if ( tmp_it_code == db_code.end( ) )
            return false;

        db_name.erase( tmp_it_name );
        db_code.erase( tmp_it_code );
        delete tmp_cit;
        return true;
    }

    bool Income ( const string & account, int amount ) {
        return ManageWallet( true, false, "", "", account, amount );
    }

    bool Income ( const string & name, const string & addr, int amount ) {
        return ManageWallet( false, false, name, addr, "", amount );
    }

    bool Expense ( const string & account, int amount ) {
        return ManageWallet( true, true, "", "", account, amount );
    }

    bool Expense ( const string & name, const string & addr, int amount ) {
        return ManageWallet( false, true, name, addr, "", amount );
    }

    bool Audit ( const string & name, const string & addr, string & account, int & sumIncome, int & sumExpense ) const {
        if ( name.empty( ) || addr.empty( ) || db_code.empty( ) )
            return false;

        CCitizen * tmp = new CCitizen( name, addr, CWallet( ) );
        auto tmp_it_name = lower_bound( db_name.begin( ), db_name.end( ), tmp, CCitizen::CmpNameAddr );
        if ( tmp_it_name == db_name.end( ) || ( ( * tmp_it_name )->name != tmp->name ) ||
             ( ( * tmp_it_name )->addr != tmp->addr ) ) {
            delete tmp;
            return false;
        }
        delete tmp;
        account = ( * tmp_it_name )->acc.code;
        sumIncome = ( * tmp_it_name )->acc.income;
        sumExpense = ( * tmp_it_name )->acc.expense;
        return true;
    }

    CIterator ListByName ( ) const {
        return CIterator( db_name );
    }
};

#ifndef __PROGTEST__
int main ( ) {
    return 0;
}
#endif /* __PROGTEST__ */

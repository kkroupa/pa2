#include <iostream>
#include <fstream>
#include <algorithm>

using namespace std;

const size_t N_MIN = 5;

const char * INVALID_INPUT = "Nespravny vstup.";
const char * UNSUCCESSFUL_WRITE = "Zapis se nezdaril.";

const char * OUTPUT_FILENAME = "sorted.bin";

struct array_t {
	size_t n;
	double * data;
};

bool array_scan ( array_t & arr );

bool array_load ( array_t & arr, const char * filename );

void array_print ( const array_t & arr );

bool array_save ( const array_t & arr, const char * filename );

bool array_write ( const array_t & arr, const char * filename );

void array_sort ( array_t & arr );

void array_free ( array_t & arr );

int main ()
{
	array_t arr { 0, nullptr };

	if ( !array_scan( arr ) ) {
		cout << INVALID_INPUT << endl;
		return 1;
	}

	array_sort( arr );
	if ( !array_write( arr, OUTPUT_FILENAME ) )	{
		cout << UNSUCCESSFUL_WRITE << endl;
		return 2;
	}

	array_free( arr );
	return 0;
}

/*****************************************************************************/

bool array_scan ( array_t & arr )
{
	if ( !( cin >> arr.n ) || arr.n < N_MIN ) {
		return false;
	}

	arr.data = new double [ arr.n ];
	for ( size_t i = 0; i < arr.n; ++i ) {
		if ( !( cin >> arr.data[ i ] ) ) {
			array_free( arr );
			return false;
		}
	}

	return true;
}

bool array_load ( array_t & arr, const char * filename )
{
	ifstream fin { filename };
	if ( !( fin >> arr.n ) || arr.n < N_MIN ) {
		return false;
	}

	arr.data = new double [ arr.n ];
	for ( size_t i = 0; i < arr.n; ++i ) {
		if ( !( fin >> arr.data[ i ] ) ) {
			array_free( arr );
			return false;
		}
	}

	return true;
}

void array_print ( const array_t & arr )
{
	for ( size_t i = 0; i < arr.n; ++i ) {
		if ( i != 0 )
			cout << " ";
		cout << arr.data[ i ];
	}
}


bool array_save ( const array_t & arr, const char *filename )
{
	ofstream fout { filename };
	if ( !fout )
		return false;
	
	if ( !( fout << arr.n << endl ) )
		return false;
	
	for ( size_t i = 0; i < arr.n; ++i ) {
		if ( i != 0 )
			if ( !( fout << " " ) )
				return false;
		if ( !( fout << arr.data[ i ] ) )
			return false;
	}

	if ( !( fout << endl ) )
		return false;
	
	return true;
}

bool array_write ( const array_t & arr, const char * filename )
{
	ofstream bfout { filename, ios::binary };
	if ( !bfout )
		return false;
	
	if ( !bfout.write( (const char *) &arr.n, sizeof( arr.n ) ) )
		return false;
	
	for ( size_t i = 0; i < arr.n; ++i )
		if ( !bfout.write( (const char *)( arr.data + i ), sizeof( arr.data[ i ] ) ) )
			return false;

	return true;
}

void array_sort ( array_t & arr )
{
  sort( arr.data, arr.data + arr.n );
}

void array_free ( array_t & arr )
{
	delete [] arr.data;
	arr.n = 0;
	arr.data = nullptr;
}
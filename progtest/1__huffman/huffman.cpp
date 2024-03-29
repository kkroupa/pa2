#ifndef __PROGTEST__

#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cctype>
#include <climits>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <map>
#include <vector>
#include <algorithm>
#include <set>
#include <queue>
#include <memory>
#include <functional>
#include <stdexcept>

using namespace std;
#endif /* __PROGTEST__ */

struct node {
    int data;
    node * left;
    node * right;

    explicit node ( int data = -5, node * left = nullptr, node * right = nullptr )
            : data( data ), left( left ), right( right ) { }
};

class btree {
private:
    node * root;

public:
    btree ( ) {
        this->root = new node( );
//        cout << "Binary tree root was created." << endl;
    }

    ~btree ( ) {
        delete_nodes( root );
    }

    node * get_root ( ) {
        return this->root;
    }

    static void delete_nodes ( node * link ) {
        if ( link == nullptr )
            return;

        delete_nodes( link->left );
        delete_nodes( link->right );
        delete link;
    }

    static void print_nodes ( node * link, bool ascii_mode = false ) {
        if ( link == nullptr )
            return;

        print_nodes( link->left, ascii_mode );
        if ( link->data != -5 ) {
            if ( ascii_mode )
                cout << ( char ) link->data << " ";
            else
                cout << link->data << " ";
        }
        print_nodes( link->right, ascii_mode );
    }
};

class bitstream {
private:
    std::ifstream & input;
    int buffer = 0;
    int curr_pos = 8;
    int tmp_char = 0;
    int chunk_chars_left = 0;
    int decoded_char = 0;
    string parsed_text;
public:
    explicit bitstream ( std::ifstream & _input ) : input( _input ) {
        buffer = read_char( );
    };

    ~bitstream ( ) {
        input.close( );
    }

    std::ifstream & get_stream ( ) {
        return input;
    }

    int read_char ( ) {
        char c;
        this->input.get( c );
        return c;
    }

    int read_bit ( ) {
        if ( input.eof( ) ) return -1;
        if ( curr_pos-- <= 0 ) {
            curr_pos = 7;
            buffer = read_char( );
            if ( input.eof( ) ) return -1;
            if ( input.fail( ) || input.bad( ) ) return -2;
        }
        return ( ( buffer >> curr_pos ) & 1 );
    }

    int read_sequence ( int bits = 8, bool print = false ) {
        int bit = 0, res = 0;
        for ( int i = bits - 1; i >= 0; i-- ) {
            if ( ( bit = read_bit( ) ) )
                res += ( 1 << i );
            if ( bit < 0 )
                return -1;
            if ( print ) cout << bit;
        }
        if ( print ) cout << " ";
        return res;
    }

    int read_tree_bit ( ) {
        int bit = read_bit( );
        if ( bit < 0 )
            return -1;
        if ( bit == 1 )
            if ( ( tmp_char = read_sequence( ) ) < 0 )
                return -1;
        return bit;
    }

    void deserialize_tree ( node * link, int & index ) {
        if ( index == -1 )
            return;
        if ( index == 1 ) {
            link->data = tmp_char;
            link->left = link->right = nullptr;
            tmp_char = -5;
            return;
        }
        deserialize_tree( ( link->left = new node( ) ), ( index = read_tree_bit( ) ) );
        deserialize_tree( ( link->right = new node( ) ), ( index = read_tree_bit( ) ) );
    }

    int decode_character ( node * link ) {
        if ( !link->left && !link->right && link->data >= 0 ) {
            return link->data;
        }
        int bit = read_bit( );
        if ( bit < 0 )
            return -1;
        if ( !bit )
            return decode_character( link->left );
        return decode_character( link->right );
    }

    int decode_chunks ( node * root ) {
        int bit = read_bit( );
        chunk_chars_left = bit ? 4096 : read_sequence( 12 );
        if ( chunk_chars_left == 0 )
            if ( read_bit( ) != -1 )
                return -1;
        while ( chunk_chars_left > 0 ) {
            if ( ( decoded_char = decode_character( root ) ) < 0 )
                return -1;
            parsed_text += ( char ) decoded_char;
            chunk_chars_left--;
        }
        if ( bit ) decode_chunks( root );
        return 1;
    }

    string read_parsed_text ( ) {
        return parsed_text;
    }

    bool export_parsed_text ( const char * name ) {
        ofstream ofs{ name, ios::out };
        if ( !ofs )
            return false;

        if ( !( ofs << read_parsed_text( ) ) )
            return false;

        ofs.close( );
        return true;
    }
};


bool decompressFile ( const char * inFileName, const char * outFileName ) {
    ifstream ifs{ inFileName, ios::binary | ios::in };

    if ( !ifs )
        return false;

    bitstream bfs( ifs );
    btree huff;
    int index = bfs.read_tree_bit( );

    bfs.deserialize_tree( huff.get_root( ), index );

    if ( bfs.get_stream( ).eof( ) ) {
        return false;
    }

    if ( bfs.decode_chunks( huff.get_root( ) ) < 0 ) {
        return false;
    }

    return bfs.export_parsed_text( outFileName );
}


bool compressFile ( const char * inFileName, const char * outFileName ) {
    // keep this dummy implementation (no bonus) or implement the compression (bonus)
    return false;
}

#ifndef __PROGTEST__

bool identicalFiles ( const char * fileName1, const char * fileName2 ) {
    // todo
    return false;
}

#include "progtester/tests.cpp"

int main ( void ) {

//    assert_( );

    assert( decompressFile( "tests/extra0.huf", "temp" ) );

    return 0;
}

#endif /* __PROGTEST__ */

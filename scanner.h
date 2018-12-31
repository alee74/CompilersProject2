/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                   *
 * scanner.h                                         *
 *                                                   *
 * Contains declarations for TokenCat and Opcode     *
 * enumerations, Token structure, and Scanner class, *
 * as well as all necessary import and using         *
 * statements.                                       *
 *                                                   *
 * Written by: Austin James Lee                      *
 *                                                   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * */

#pragma once

#include <iostream> // ostream, cout, endl, istream, cerr
#include <fstream>	// ifstream
#include <string>
#include <cctype>	// isdigit(), isspace(), isalpha()
#include <climits>	// INT_MIN
#include <cstdlib>	// exit(), EXIT_FAILURE
#include <cstdio>	// EOF

using std::string;
using std::ostream;
using std::ifstream;
using std::istream;
using std::cout;
using std::endl;
using std::getline;
using std::cerr;


////// Enumerations //////


/// Token Categories ///
enum TokenCat {
    Instruct,
    Reg,
    Constant,
    Arrow,
    Comma,
	INVALID = -1	// represents EOF when returned
};


/// Opcodes ///
enum Opcode {
	load,
    loadI,
    store,
    add,
    sub,
    mult,
    lshift,
    rshift,
    output,
    nop
};


////// Token structure //////

struct Token {
	Token();
	Token(TokenCat tc, int v);
    TokenCat cat;
    int value;
	// overload of << operator for simple & pretty printing
	friend ostream& operator<<(ostream& os, const Token& t);
};


////// Scanner class //////

class Scanner {
	public:
		Scanner();						// default constructor
		Scanner(string f, bool=false);	// normal constructor
		Scanner(const Scanner& s);		// copy constructor
		~Scanner();				// deconstructor, closes input file stream
		Token scanToken();		// scans and returns arbitrary Token
		Token scanInstruction();// scans and returns an instruction as Token
		Token scanRegister();	// scans and returns a register as Token
		Token scanConstant();	// scans and returns an int as Token
		Token scanArrow();		// scans and returns assignment arrow as Token
		Token scanComma();		// scans and returns a comma as Token
	private:
		string infile;			// name of input file
		ifstream input;			// input file stream
		bool print;				// indicates whether -t option was passed
		int ln;					// current line number
		int pos;				// index of character on current line
		int get();				// extension of std::ifstream::get()
		istream& get(char& c);	// extension of std::ifstream::get(char& c)
		bool ensureWS();		// returns bool indicating presences of WS
		bool ensureNL();		// returns bool indicating presence of new line
		void removeWS();		// scans and discards whitespace
		void removeComment();	// scans and discards a comment
		void error(string msg);	// prints explicit error message and terminates
		int scanNumber();		// scans and returns an int
		Token scanAlpha();		// scanToken() helper, called on alpha characters
};

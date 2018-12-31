/* * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                       *
 * parser.h                                              *
 *                                                       *
 * Contains declarations for Register and Instruction    *
 * structures and Parser class, as well as all necessary *
 * includes and using statements not already present in  *
 * scanner.h.                                            *
 *                                                       *
 * Written by: Austin James Lee                          *
 *                                                       *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#pragma once

#include "scanner.h"
#include <list>
#include <iomanip>

using std::list;
using std::setw;
using std::left;


//// Register structure ////

struct Register {
	// constructor. takes bool used to set op1
	Register(bool o);
	// constructor. takes values in order they are declared
	Register(int = INVALID, bool = false, bool = false,
			int = INVALID, int = INVALID, int = INVALID);
	int sr;		// source register
	bool isReg;	// valid register?
	bool isOp1;	// indicates if src1 (purely for printing)
	int vr;		// virtual register
	int pr;		// physical register
	int nu;		// index of next use of this register
	// allow for simple and pretty printing
	friend ostream& operator<<(ostream& os, const Register& r);
};


//// Instruction structure ////

struct Instruction {
	// constructor. takes values in order listed.
	Instruction(Opcode = (Opcode)INVALID,
		Register={true}, Register={}, Register={});
	Opcode op;				
	Register src1;
	Register src2;
	Register dest;
	// allow for simple and pretty printing
	friend ostream& operator<<(ostream& os, const Instruction& i);
};


//// Parser class ////

class Parser {
	public:
		// constructor (calls parse)
		Parser(string infile, bool = false);
		list<Instruction> intRep;	// list representing IR
	private:
		Scanner scanner;	// Scanner used to scan tokens
		void parse();		// main parse function
};

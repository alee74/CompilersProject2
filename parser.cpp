/* * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                       *
 * parser.cpp                                            *
 *                                                       *
 * Contains implementations for everything in parser.h.  *
 * Methods appear in same order as they do in parser.h   *
 * with the exception of Instruction's print function,   *
 * which is at the end of the file.                      *
 *                                                       *
 * Written by: Austin James Lee                          *
 *                                                       *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "parser.h"


//// Register constructors ////


// constructor for Instruction.src1
Register::Register(bool o)
		:sr{INVALID}, isReg{false}, isOp1{o},
		vr{INVALID}, pr{INVALID}, nu{INVALID} {}


// default constructor
// all arguments declared with defaults
Register::Register(int s, bool r, bool o, int v, int p, int n)
					:sr{s}, isReg{r}, isOp1{o}, vr{v}, pr{p}, nu{n} {}


//// Instruction constructor ////


// overloaded constructor
Instruction::Instruction(Opcode o, Register s1, Register s2, Register d)
			:op{o}, src1{s1}, src2{s2}, dest{d} {}



//// Parser methods ////


// constructor (public)
// takes file name and "scanner print" bool to construct Scanner,
Parser::Parser(string infile, bool sp) :scanner{infile, sp} {
	// parse until EOF or error
	parse();
}


// main parse function (private; called from constructor)
// requests an instruction token, then builds an
// Instruction based on its Opcode and adds it to the
// end of the intermediate representation (intRep)
void Parser::parse() {
	// scan next Instruction
	Token t = scanner.scanInstruction();
	// check for EOF (only time Invalid Token is returned)
	if (t.cat == INVALID)
		return;

	Instruction i {(Opcode)t.value};
	switch (i.op) {

		case load:
			i.src1 = Register {(scanner.scanRegister()).value, true, true};
			scanner.scanArrow();
			i.dest = Register {(scanner.scanRegister()).value, true};
			break;

		case loadI:
			i.src1 = Register {(scanner.scanConstant()).value, false, true};
			scanner.scanArrow();
			i.dest = Register {(scanner.scanRegister()).value, true};
			break;

		case store:
			i.src1 = Register {(scanner.scanRegister()).value, true, true};
			scanner.scanArrow();
			i.src2 = Register {(scanner.scanRegister()).value, true};
			break;

		case output:
			i.src1 = Register {(scanner.scanConstant()).value, false, true};
			break;

		case nop:
			break;

		// arithmetic operations
		default:
			i.src1 = Register {(scanner.scanRegister()).value, true, true};
			scanner.scanComma();
			i.src2 = Register {(scanner.scanRegister()).value, true};
			scanner.scanArrow();
			i.dest = Register {(scanner.scanRegister()).value, true};
			break;
	}

	// add Instruction to end of IR
	intRep.push_back(i);

	// continue parsing
	parse();
}



//// struct overloaded << print function ////



// Register pretty print
// first sr field for an instruction: 12 spaces
//	- up to INT_MAX with 1 space padding
// all other register fields: 6 spaces
//	- register value up to 1000 with 1 space padding
ostream& operator<<(ostream& os, const Register& r) {
	string noReg = "  -   ";
	// print source register
	if (r.sr == INVALID) {
		os << noReg;
		if (r.isOp1)
			os << "      ";
	} else if (r.isReg) {
		os << " r";
		// set field width
		if (r.isOp1)
			os << setw(10);
		else
			os << setw(4);
		os << left << r.sr;
	} else
		os << " " << setw(11) << left << r.sr;
	os << "|";

	// print virtual register
	if (r.vr == INVALID)
		os << noReg;
	else
		os << " v" << setw(4) << left << r.vr;
	os << "|";

	// print physical register
	if (r.pr == INVALID)
		os << noReg;
	else
		os << " r" << setw(4) << left << r.pr;
	os << "|";

	// print next use
	if (r.nu == INVALID)
		os << noReg;
	else if (r.nu == INT_MAX)
		os << " INF  ";
	else
		os << " " << setw(5) << left << r.nu;
	os << "||";

	return os;
}


// Instruction pretty print
// opcode field: 8
//	- max opcode is 6 chars + 1 space of padding
ostream& operator<<(ostream& os, const Instruction& i) {
	// set ostream variables
	os << " " << setw(7) << left;
	// print Opcode
	switch (i.op) {
		case load:
			os << "load";
			break;
		case loadI:
			os << "loadI";
			break;
		case store:
			os << "store";
			break;
		case add:
			os << "add";
			break;
		case sub:
			os << "sub";
			break;
		case mult:
			os << "mult";
			break;
		case lshift:
			os << "lshift";
			break;
		case rshift:
			os << "rshift";
			break;
		case output:
			os << "output";
			break;
		case nop:
			os << "nop";
			break;
		default:
			os << "no es bueno";
			break;
	}
	os << "|||";

	// print registers!
	os << i.src1 << i.src2 << i.dest;

	return os;
}


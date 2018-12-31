/* * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                       *
 * main.cpp                                              *
 *                                                       *
 * Main for Allocator implementation.                    *
 * Expects file as command line argument.                *
 * Uses file to construct Allocator, which constructs    *
 * a Parser (that constructs a Scanner) to parse and     *
 * scan file and build its intermediate representation,  *
 * which it utilizes (and modifies) in order to perform  *
 * register allocation.                                  *
 *                                                       *
 * Run with [-h --help] option for additional info.      *
 *                                                       *
 * Written by: Austin James Lee                          *
 *                                                       *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#define MIN_ARGS 2
#define MAX_ARGS 5
#define MIN_REGS 3
#define DEFAULT 5

#include "allocator.h"
#include <cstring>	// strcmp()

using std::strcmp;

// helper function prototypes
bool validFile(string filename);
void printCode(list<Instruction>& ir);


/// main ///
int main(int argc, char* argv[]) {
	string infile = "";
	int k = INVALID;
	bool printTokens = false;	// -t
	bool printDebug = false;	// -p
	string usage = "usage: reader [-t] [-p] [-h --help] [-k num] <filename>\n"
					"where: <filename> is the name of the file to be compiled\n"
					"       and brackets indicate program options.\n"
					"		invoke the help option for further details.";
	string help = "\n"
		"\'alloc\' adds naive register allocation to my implementation of\n"
		"the front end of a compiler for a subset of ILOC code. It takes a\n"
		"file containing a block of ILOC code as input and scans and parses\n"
		"its contents, generating an intermediate representation (IR). The\n"
		"IR is then passed to an allocator that allocates a specified number\n"
		"of the target machine's physical registers to the registers in the\n"
		"source code.\n\n"
		"usage: reader [-t] [-p] [-h --help] [-k num] <filename>\n\n"
		"Program arguments:\n"
		"      -t   prints a list of the tokens scanned, each on its own line.\n"
		"           tokens are of the form <TOKEN_TYPE, lexeme>\n"
		"      -p   prints the IR in tabular form with verbose register output.\n"
		"           (i.e. source, virtual, and physical registers, as well as\n"
		"           next use for each register).\n"
		"           this option is included for debugging purposes.\n"
		"      -h   help option. prints this help summary and exits the simulation.\n"
		"           --help is the verbose form of this option.\n"
		"  -k num   allows the user to specify the number of physical registers\n"
		"           to be allocated. if not specified, defaults to 10.\n"
		"filename   the name of a file containing ILOC code to be compiled.\n"
		"           unless the help option is invoked, this will always be the\n"
		"           last option.\n\n"
		"One option is permitted in addition to [-k num], but must precede it.\n"
		"If neither [-t] nor [-p] are invoked, the legal ILOC code generated\n"
		"from the IR will be printed upon completion of allocation.\n";
		

	// ensure correct number of arguments
	if (argc < MIN_ARGS) {
		cerr << "error: not enough arguments"
			<< endl << usage << endl;
		return 1;
	} else if (argc > MAX_ARGS) {
		cerr << "error: too many arguments"
			<< endl << usage << endl;
		return 1;
	// parse arguments
	} else {
		// parse -h & --help
		if (strcmp(argv[1], "-h") == 0 ||
			strcmp(argv[1], "--help") == 0) {
				cout << help << endl;
				return 0;
		// parse -t
		} else if (strcmp(argv[1], "-t") == 0)
			printTokens = true;
		// parse -p
		else if (strcmp(argv[1], "-p") == 0)
			printDebug = true;
		// parse -k num
		else if (strcmp(argv[1], "-k") == 0) {
			// parse num
			try {
				k = stoi(string(argv[2]));
				if (k < MIN_REGS)
					throw INVALID;
			} catch (...) {
				cerr << "error: invalid number of registers: "
					<< argv[2] << endl << usage << endl;
				return 1;
			}
			// ensure has another argument
			if (argc == 3) {
				cerr << "error: missing filename"
					<< endl << usage << endl;
				return 1;
			}
		// parse filename for argc == 2
		} else if (validFile(argv[1]))
			infile = argv[1];
		// bad filename
		else if (argc == 2) {
			cerr << "error: invalid filename: " 
				<< argv[1] << endl << usage << endl;
			return 1;
		// bad argument
		} else {
			cerr << "error: invalid argument: "
				<< argv[1] << endl << usage << endl;
			return 1;
		}

		// parse for argc == 3
		// argv[2] must be filename
		if (argc == 3) {
			// check if already parsed filename
			if (infile == "") {
				// parse filename
				if (validFile(argv[2]))
					infile = argv[2];
				// bad filename
				else {
					cerr << "error: invalid filename: " 
						<< argv[2] << endl << usage << endl;
					return 1;
				}
			// excess arguments
			} else {
				cerr << "error: invalid argument(s) following filename"
					<< endl << usage << endl;
				return 1;
			}
		}

		// parse for argc == 4
		// would already have parsed -k num
		if (argc == 4) {
			// ensure -k num is in fact what was already parsed
			if (printTokens || printDebug || k == INVALID) {
				cerr << "error: invalid number of arguments"
					<< endl << usage << endl;
				return 1;
			}
			// check if already parsed filename
			if (infile == "") {
				// parse filename
				if (validFile(argv[3]))
					infile = argv[3];
				// bad filename
				else {
					cerr << "error: invalid filename: " 
						<< argv[3] << endl << usage << endl;
					return 1;
				}
			// excess arguments
			} else {
				cerr << "error: invalid argument(s) following filename"
					<< endl << usage << endl;
				return 1;
			}
		}

		// parse for argc == 5
		// must be -k num filename
		if (argc == 5) {
			// check if already parsed filename
			if (infile == "") {
				// check if already parsed -k num
				if (k != INVALID) {
					cerr << "error: invalid (order of) argument(s)"
						<< endl << usage << endl;
					return 1;
				} else {
					// parse -k num
					if (strcmp(argv[2], "-k") == 0) {
						// parse num
						try {
							k = stoi(string(argv[3]));
							if (k < MIN_REGS)
								throw INVALID;
						} catch (...) {
							cerr << "error: invalid number of registers: "
								<< argv[3] << endl << usage << endl;
							return 1;
						}
					// bad argument
					} else {
						cerr << "error: invalid argument: "
							<< argv[2] << endl << usage << endl;
						return 1;
					}
				}
				// parse filename
				if (validFile(argv[4]))
					infile = argv[4];
				// bad filename
				else {
					cerr << "error: invalid filename: "
						<< argv[4] << endl << usage << endl;
					return 1;
				}
			// excess arguments
			} else {
				cerr << "error: invalid argument(s) following filename"
					<< endl << usage << endl;
				return 1;
			}
		}

	}

	// ensure k is valid number of registers
	if (k < 0)
		k = DEFAULT;

	// create Allocator
	// all allocation occurs in constructor
	Allocator allocator {infile, k, printTokens};

	// produce output
	if (printDebug && !printTokens)
		cerr << allocator;
	printCode(allocator.intRep);

	return 0;
}


// tests for valid file
bool validFile(string filename) {
	bool valid = false;
	ifstream f(filename);
	if (f) {
		valid = true;
		f.close();
	}
	return valid;
}


// print generated legal ILOC code
void printCode(list<Instruction>& ir) {
	list<Instruction>::iterator it = ir.begin();
	while (it != ir.end()) {
		// set cout vars for opcode
		cout << setw(10) << left;
		// print opcode
		switch (it->op) {
			case load:
				cout << "load";
				break;
			case loadI:
				cout << "loadI";
				break;
			case store:
				cout << "store";
				break;
			case add:
				cout << "add";
				break;
			case sub:
				cout << "sub";
				break;
			case mult:
				cout << "mult";
				break;
			case lshift:
				cout << "lshift";
				break;
			case rshift:
				cout << "rshift";
				break;
			case output:
				cout << "output";
				break;
			case nop:
				cout << "nop" << endl;
				++it;
				continue;
			default:
				cout << "uh oh... somethin' done goofed.";
				break;
		}

		// print op1 register
		if (it->src1.isReg && it->src1.pr != INVALID)
			cout << "r" << setw(9) << left << it->src1.pr;
		else if (it->src1.sr != INVALID) {
			cout << setw(10) << left << it->src1.sr;
			if (it->op == output) {
				cout << endl;
				++it;
				continue;
			}
		}

		// print op2 register
		if (it->op != store && it->src2.pr != INVALID)
			cout << ",  r" << setw(6) << left << it->src2.pr;
		else
			cout << setw(10) << "";

		// print arrow
		cout << "=>   ";

		//print op3 register
		if (it->dest.pr != INVALID)
			cout << "r" << it->dest.pr;
		if (it->op == store)
			cout << "r" << it->src2.pr;

		// print new line
		cout << endl;
		// increment iterator
		++it;
	}
}


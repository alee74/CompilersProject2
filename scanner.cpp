/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                 *
 * scanner.cpp                                                     *
 *                                                                 *
 * Contains implementations for everything declared in scanner.h.  *
 * Methods appear in the same order as they do in scanner.h with   *
 * the exception of Token's print function, which is at the end of *
 * the file.                                                       *
 *                                                                 *
 * Written by: Austin James Lee                                    *
 *                                                                 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "scanner.h"


//// Token constructors ////


// Token "default" constructor
// constructs "invalid" Tokens.
// only returned to indicate EOF.
Token::Token() :cat{INVALID}, value{INT_MIN} {}


// valid Token constructor
Token::Token(TokenCat tc, int v) :cat{tc}, value{v} {}



//// public Scanner methods ////


// Scanner default constructor
Scanner::Scanner() :infile{""}, print{false}, ln{-1}, pos{-1} {}


// Scanner constructor
// takes input file's name and opens ifstream.
// also takes bool indicating whether -t option was passed.
// initializes line to 1 and pos to 0.
Scanner::Scanner(string f, bool p) :infile{f}, print{p}, ln{1}, pos{0} {
	input.open(infile);
}


// Scanner copy constructor
Scanner::Scanner(const Scanner& s)
		:infile{s.infile}, print{s.print}, ln{s.ln}, pos{s.pos} {
	input.open(infile);
}


// Scanner deconstructor (public)
// closes ifstream before destroying Scanner object.
Scanner::~Scanner() {
	input.close();
}


// scans and returns an arbitrary Token from input.
// checks for EOF and returns Invalid Token if found
// calls Scanner::error(string msg), terminating on bad input.
Token Scanner::scanToken() {
	// remove WS, NL, and check for EOF
	removeWS();
	if (ensureNL()) {
		get();
		return scanToken();
	} else if (input.peek() == EOF)
		return Token();

	Token ret;
	switch (input.peek()) {

		case '=':
			get();
			if (get() == '>')
				ret = Token {Arrow, -1};
			else
				error("expected assignment arrow");
			break;

		case ',':
			get();
			ret = Token {Comma, -1};
			break;

		case '/':
			removeComment(); // handles error
			return scanToken();
			break;

		default:
			if (isalpha(input.peek()))
				ret = scanAlpha();
			else if (isdigit(input.peek()))
				ret = Token {Constant, scanNumber()};
			else
				error("invalid character to start Token");
			break;
	}

	// remove trailing whitespace
	removeWS();

	if (print)
		cerr << ret << endl;

	return ret;
}


// ensures operation begins on a new line, removes
// leading whitespace, scans an insturction opcode,
// ensures and removes trailing whitespace, prints
// Token if -t was passed, and returns Instruction
// Token.
// checks for EOF, returning Invalid Token if found.
// terminates on bad input via Scanner::error().
Token Scanner::scanInstruction() {
	// this block ensures all instructions begin on a new line:
	//	- removes any leading whitespace
	//	- checks for and removes comment (including new line)
	//	- checks for and removes new line
	//	- checks for EOF, returning Invalid Token if found
	//	- if it is not the first line and did not find comment
	//		or new line, then it follows other Tokens on a line
	//		because new lines are only consumed here.
	removeWS();
/*
	if (ensureNL() || input.peek() == '/') {
		do {
			if (ensureNL())
				get();
			if (input.peek() == '/')
				removeComment();
		} while (ensureNL());
	} else if (ln != 1)
		error("all ILOC operations must begin on a new line");

	// check for eof
	if (input.peek() == EOF)
		return Token();
*/
	if (ensureNL())
		get();
	else if (input.peek() == '/') {
		while (input.peek() == '/') {
			removeComment();
			get();	// consume new line
		}
	} else if (input.peek() == EOF)
		return Token();
	else if (ln != 1)
		error("all ILOC operations must begin on a new line");

	removeWS();

	// accounts for blank lines
	if (ensureNL() || input.peek() == '/')
		return scanInstruction();
/*
	if (input.peek() == '/') {
		removeComment();
		return scanInstruction();
	}
*/

	Token ret = Token {Instruct, INT_MIN};
	switch (get()) {

		case 's':
			switch (get()) {

				// "store"
				case 't':
					if (get() == 'o' && get() == 'r' && get() == 'e')
						ret.value = store;
					else
						error("expected opcode \"store\"");
					break;
					
				// "sub"
				case 'u':
					if (get() == 'b')
						ret.value = sub;
					else
						error("expected opcode \"sub\"");
					break;

				default:
					error("invalid character following 's'");
			}
			break;

		case 'l':
			switch (get()) {

				case 'o':
					if (get() == 'a' && get() == 'd') {
						// "loadI"
						if (input.peek() == 'I') {
							get();
							ret.value = loadI;
						// "load"
						} else
							ret.value = load;
					} else
						error("expected opcode \"load\" or \"loadI\"");
					break;

				case 's':
					// "lshift"
					if (get() == 'h' && get() == 'i' &&
						get() == 'f' && get() == 't')
							ret.value = lshift;
					else
						error("expected opcode \"lshift\"");
					break;

				default:
					error("invalid character following 'l'");
			}
			break;

		case 'r':
			// "rshift"
			if (get() == 's' && get() == 'h' &&
				get() == 'i' && get() == 'f' && get() == 't')
					ret.value = rshift;
			else
				error("expected opcode \"rshift\"");
			break;

		case 'm':
			// "mult"
			if (get() == 'u' && get() == 'l' && get() == 't')
				ret.value = mult;
			else
				error("expected opcode \"mult\"");
			break;

		case 'a':
			// "add"
			if (get() == 'd' && get() == 'd')
				ret.value = add;
			else
				error("expected opcode \"add\"");
			break;

		case 'n':
			// "nop"
			if (get() == 'o' && get() == 'p')
				ret.value = nop;
			else
				error("expected opcode \"nop\"");
			break;

		case 'o':
			// "output"
			if (get() == 'u' && get() == 't' &&
				get() == 'p' && get() == 'u' && get() == 't')
					ret.value = output;
			else
				error("expected opcode \"output\"");
			break;

		case '/':
			removeComment();
			return scanInstruction();

		case EOF:
			return Token();

		default:
			error("expected instruction opcode");
			break;
	}

	if (ensureWS())
		removeWS();
	else if (ret.value != nop)	// bc nop can be immediately followed by NL
		error("no whitespace following valid opcode");

	if (print)
		cerr << ret << endl;

	return ret;
}


// scans a register, removes trailing
// whitespace, prints Token if -t was passed,
// and returns a Register Token.
// terminates on bad input via Scanner::error().
//
// ret defined as invalid Token, but only valid
// Register Token will be returned due to error().
Token Scanner::scanRegister() {
	Token ret = Token();
	if (get() == 'r') {
		if (isdigit(input.peek())) {
			ret = Token {Reg, scanNumber()};
			removeWS();
		} else
			error("expected register number");
	} else
		error("expected register");
	if (print)
		cerr << ret << endl;
	return ret;
}


// scans a numerical constant, removes trailing
// whitespace, prints Token if -t was passed,
// and returns a Constant Token.
// terminates on bad input via Scanner::error().
//
// ret defined as invalid Token, but only valid
// Constant Token will be returned due to error().
Token Scanner::scanConstant() {
	Token ret = Token();
	if (isdigit(input.peek())) {
		ret = Token {Constant, scanNumber()};
		removeWS();
	} else
		error("expected numerical constant");
	if (print)
		cerr << ret << endl;
	return ret;
}


// scans an arrow, removes trailing
// whitespace, prints Token if -t was passed,
// and returns an Arrow Token.
// terminates on bad input via Scanner::error().
//
// ret defined as invalid Token, but only valid
// Arrow Token will be returned due to error().
Token Scanner::scanArrow() {
	Token ret = Token();
	if (get() == '=' && get() == '>') {
		removeWS();
		ret = Token {Arrow, -1};
	} else
		error("expected assignment arrow");
	if (print)
		cerr << ret << endl;
	return ret;
}


// scans a comma, removes trailing
// whitespace, and returns a Comma Token.
// terminates on bad input via Scanner::error().
//
// ret defined as invalid Token, but only valid
// Comma Token will be returned due to error().
Token Scanner::scanComma() {
	Token ret = Token();
	if (get() == ',') {
		removeWS();
		ret = Token {Comma, -1};
	} else
		error("expected comma to separate register arguments");
	if (print)
		cerr << ret << endl;
	return ret;
}


//// private Scanner methods ////


// extension of std::istream::get()
// adds line/position counting functionality.
int Scanner::get() {
	pos++;
	if (ensureNL()) {
		ln++;
		pos = 0;
	}
	return input.get();
}


// extension of std::istream::get(char& c)
// adds line/position counting functionality.
istream& Scanner::get(char& c) {
	c = get();
	return input;
}


// indicates whether or not next
// input character is whitespace.
bool Scanner::ensureWS() {
	int c = input.peek();
	return c == ' ' || c == '\t';
}


// indicates whether or not next input 
// character will produce a new line.
bool Scanner::ensureNL() {
	int c = input.peek();
	return c == '\n' || c == '\r' || c == '\f' || c == '\v';
}


// consumes whitespace from input.
void Scanner::removeWS() {
	while (ensureWS())
		get();
}


// checks for "//" then consumes remainder of line
void Scanner::removeComment() {
	if (get() == '/' && get() == '/') {
		while (!ensureNL() && input.peek() != EOF)
			get();
//		get();
	} else
		error("invalid '/'; epected comment");
}


// prints explicit error message to 
// console and terminates program.
// to be called when bad input is encountered.
void Scanner::error(string msg) {
	cerr << infile << ":" << ln << ":" << pos << ": ERROR: "
		<< msg << endl << "Terminating program." << endl;
	exit(EXIT_FAILURE);
}


// scans and returns numerical value from input.
// does not check that first char is in fact
// digit and does not handle error if not.
// thus, caller must perform check
int Scanner::scanNumber() {
	string num = "";
	while (isdigit(input.peek()))
		num += get();
	return stoi(num);
}


// helper to scanToken().
// scans and returns Instruction Tokens and Register Tokens.
//
// separated from scanToken() to reduce clutter
// and to allow ensuring and eliminating whitespace 
// following Instruction Tokens while avoiding errors
// arising from recursing on whitespace in default
// case of scanToken().
Token Scanner::scanAlpha() {
	int op = INT_MIN;
	switch (get()) {

		case 's':
			switch (get()) {

				// "store"
				case 't':
					if (get() == 'o' && get() == 'r' && get() == 'e')
						op = store;
					else
						error("expected opcode \"store\"");
					break;
					
				// "sub"
				case 'u':
					if (get() == 'b')
						op = sub;
					else
						error("expected opcode \"sub\"");
					break;

				default:
					error("invalid character following 's'");
			}
			break;

		case 'l':
			switch (get()) {

				case 'o':
					if (get() == 'a' && get() == 'd') {
						// "loadI"
						if (input.peek() == 'I') {
							get();
							op = loadI;
						// "load"
						} else
							op = load;
					} else
						error("expected opcode \"load\" or \"loadI\"");
					break;

				case 's':
					// "lshift"
					if (get() == 'h' && get() == 'i' &&
						get() == 'f' && get() == 't')
							op = lshift;
					else
						error("expected opcode \"lshift\"");
					break;

				default:
					error("invalid character following 'l'");
			}
			break;

		case 'r':
			// register
			if (isdigit(input.peek()))
				return Token {Reg, scanNumber()};
			// "rshift"
			else if (get() == 's' && get() == 'h' &&
				get() == 'i' && get() == 'f' && get() == 't')
					op = rshift;
			else
				error("expected valid register number or opcode \"rshift\"");
			break;

		case 'm':
			// "mult"
			if (get() == 'u' && get() == 'l' && get() == 't')
				op = mult;
			else
				error("expected opcode \"mult\"");
			break;

		case 'a':
			// "add"
			if (get() == 'd' && get() == 'd')
				op = add;
			else
				error("expected opcode \"add\"");
			break;

		case 'n':
			// "nop"
			if (get() == 'o' && get() == 'p')
				op = nop;
			else
				error("expected opcode \"nop\"");
			break;

		case 'o':
			// "output"
			if (get() == 'u' && get() == 't' &&
				get() == 'p' && get() == 'u' && get() == 't')
					op = output;
			else
				error("expected opcode \"output\"");
			break;

		default:
			error("expected valid register or instruction opcode");
			break;
	}

	if (ensureWS())
		removeWS();
	else if (op != nop)	// bc nop can be immediately followed by NL
		error("no whitespace following valid opcode");

	return Token {Instruct, op};
}


//// Token print method ////


// overload of << operator to allow for simple printing
ostream& operator<<(ostream& os, const Token& t) {
	os << "// <";
	switch (t.cat) {

		case Instruct:
			os << "INST, ";
			switch (t.value) {

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
					os << "Invalid, this should never happen";
					break;
			}
			break;

		case Reg:
			os << "REG, r" << t.value;
			break;

		case Constant:
			os << "CONST, " << t.value;
			break;

		case Arrow:
			os << "ARROW, \'=>\'";
			break;

		case Comma:
			os << "COMMA, \',\'";
			break;

		default:
			os << "Invalid, this should never happen";
			break;
	}

	os << '>';

	return os;
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                     *
 * allocator.h                                         *
 *                                                     *
 * Contains declarations for Allocator class and its   *
 * nested Class struct, as well as  all necessary      *
 * includes and using statements not already present   *
 * in parser.h and scanner.h.                          *
 *                                                     *
 * Written by: Austin James Lee                        *
 *                                                     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#pragma once

#define SPILL 32768

#include "parser.h"
#include <vector>
#include <stack>
#include <algorithm>	// find, max_element, find_if
#include <utility>		// pair

using std::vector;
using std::stack;
using std::find;
using std::max_element;
using std::pair;
using std::find_if;

// type aliases
typedef list<Instruction>::iterator lit;
typedef pair<int, int> pii;


//// Clean enum ////

enum Clean {
	remat,
	spilled,
	cleanLoad,
	dirty = -1
};


//// Allocator class ////

class Allocator {
	// struct to represent a Class of registers
	// private because precedes public keyword
	// 	(class members are private by default)
	struct Class {
		Class(int numRegs);	// constructor
		const int sz;		// k -> number of pr
		vector<bool> free;	// free[i] indicates if ri is available
		vector<int> name;	// name[i] holds vr assigned to ri
		vector<int> next;	// next[i] holds nextUse of ri
		vector<Clean> cclean;// clean[i] holds what Clean type of ri
		stack<int> stk;		// holds i of free ri
	};
	public:
		// constructor. takes k and bool for print Tokens (Scanner)
		Allocator(string infile, int = 5, bool = false);
		list<Instruction> intRep;		// intermediate representation
	private:
		int k;							// num pr available for allocation
		int nextMemAddr;				// memory address for next spill
		int maxLive;					// maximum live registers at any point
		vector<int> vr2mem;				// vr2mem[i] holds spill address of vri
//		vector<int> uses;				// uses[i] indicates num times vri is used
		vector<Clean> clean;			// indices are vri, indicates if/how clean
		void assignRegisters(Class c);			// map vr to k pr's
		int ensure(lit& it, int vr, Class& c);	// ensure pr allocated to vr
		int allocate(lit& it, int vr, Class& c);// allocates pr for vr
		int optimalPR(Class& c);				// find optimal pr to allocate
		int bestOfType(Class& c, Clean ctype, bool=false);// max next for regs of ctype
		void freeRegister(int pr, Class& c);	// frees a physical register
		void computeLastUses();					// map sr to vr && set nu
		void update(Register& op, int ind, int& vrName, int& numLive,
							vector<int>& sr2vr, vector<int>& lastUse);
		int getNumSR();		// returns number of sr
		// pretty printing of intermediate representation.
		friend ostream& operator<<(ostream& os, const Allocator& a);
};


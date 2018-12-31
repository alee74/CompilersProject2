/* * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                       *
 * allocator.cpp                                         *
 *                                                       *
 * Contains implementation of Allocator class and its    *
 * nested Class struct. All methods appear in same order *
 * as in allocator.h.                                    *
 *                                                       *
 * Because its constructor calls assignRegister(), an    *
 * Allocator need only be constructed in order to        *
 * perform register allocation.                          *
 *                                                       *
 * Written by: Austin James Lee                          *
 *                                                       *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "allocator.h"


// constructor for Class struct
// initializes each physical register properties
// to defaults of free, invalid name, infinity next
// use, and pushes onto stack
Allocator::Class::Class(int numRegs) :sz{numRegs} {
	// reverse order so registers are allocated
	// starting with lowest number
	for (int i = sz - 1; i >= 0; --i) {
		free.push_back(true);
		name.push_back(INVALID);
		next.push_back(INT_MAX);
		cclean.push_back(dirty);
		stk.push(i);
	}
}


// Allocator constructor
// initializes values; maps source registers to virtual
// registers, computes next use (live range) of each
// register, and tracks number of live registers in the
// process, all from computeLastUses(); determines if
// need to reserve register for spilling; allocates and
// assigns physical registers to live ranges (virtual registers);
Allocator::Allocator(string infile, int numRegs, bool sp)
			:intRep{Parser{infile, sp}.intRep},
			k{numRegs}, nextMemAddr{SPILL}, maxLive{0} {
	computeLastUses();
	// if we don't have enough registers,
	// reserve last register for spilling
	if (k < maxLive)
		--k;
	// allocate and assign physical registers
	assignRegisters(Class{k});
}


// allocates and assigns k physical
// registers to the virtual registers
void Allocator::assignRegisters(Class c) {
	auto it = intRep.begin();
	while (it != intRep.end()) {
		
		// assign "rx" -- ensure register is valid
		if (it->src1.isReg)
			it->src1.pr = ensure(it, it->src1.vr, c);
		// assign "ry" -- ensure register is valid
		if (it->src2.isReg)
			it->src2.pr = ensure(it, it->src2.vr, c);

		// free assigned pr's if not needed after this Instruction
		// nu will be INT_MAX if not used or INVALID for non-registers

		// "rx"
		if (it->src1.nu == INT_MAX)
			freeRegister(it->src1.pr, c);
		// "ry"
		if (it->src2.isReg && it->src2.nu == INT_MAX)
			freeRegister(it->src2.pr, c);

		// keep assigned pr's if needed after this Instruction
		// first ensure src1 has been assigned a pr

		// "rx"
		if (it->src1.pr != INVALID)
			c.next[it->src1.pr] = it->src1.nu;
		// "ry"
		if (it->src2.pr != INVALID)
			c.next[it->src2.pr] = it->src2.nu;

		// assign "rz" -- ensure register is valid
		if (it->dest.isReg) {
			it->dest.pr = allocate(it, it->dest.vr, c);
			c.next[it->dest.pr] = it->dest.nu;
		}

		++it;
	}
}


// helper for assignRegistes()
// ensures a physical register has been allocated
// to virtual register, allocating one if not.
// returns physical register to be assigned to vr.
int Allocator::ensure(lit& it, int vr, Class& c) {
	int pr;
	// if pr already allocated to vr, find and return it
	auto found = find(c.name.begin(), c.name.end(), vr);
	if (found != c.name.end())
		pr = found - c.name.begin();
	else {
	// otherwise, allocate one
		pr = allocate(it, vr, c);
		// and RESTORE
		if (clean[vr] == remat) {
			// construct loadI Instruction and insert into IR
			// loadI vr2mem[vr] => pr
			Instruction i {loadI};
//			i.src1.sr = rematerializable[vr];
			i.src1.sr = vr2mem[vr];
			i.dest.isReg = true;
			i.dest.pr = pr;
			intRep.insert(it, i);
//cerr << "ensure::remat::inserting::loadI " << i.src1.sr << " => " << i.dest.pr << endl;
		} else if (vr2mem[vr] != INVALID) {
//		} else if (clean[vr] != dirty) {
//cerr << "restoring vr" << vr << endl;
			// construct loadI Instruction and insert into IR
			// loadI vr2mem[vr] => r0
			Instruction i {loadI};
			i.src1.sr = vr2mem[vr];
			i.dest.isReg = true;
			i.dest.pr = k;
			intRep.insert(it, i);
//cerr << "ensure::!dirty::inserting::loadI " << i.src1.sr << " => " << i.dest.pr << endl;
			// construct load Instruction and insert into IR
			// load r0 => pr
			i = Instruction {load};
			i.src1.isReg = true;
			i.src1.pr = k;
			i.dest.isReg = true;
			i.dest.pr = pr;
			intRep.insert(it, i);
		}
	}
	// return vr's pr
	return pr;
}


// helper for assignRegisters() and ensure()
// allocates a physical register to virtual
// register, spilling it if already in use.
int Allocator::allocate(lit& it, int vr, Class& c) {
	int pr;
	// if pr available, return one
	if (!c.stk.empty()) {
		pr = c.stk.top();
		c.stk.pop();
	} else {
	// otherwise, find pr that won't be
	// used for longest, spill and return it
//		auto maxNext = max_element(c.next.begin(), c.next.end());
//		pr = maxNext - c.next.begin();
		pr = optimalPR(c);
		// SPILL
		if (clean[c.name[pr]] == dirty) {
//cerr << "spilling vr" << vr << endl;
			// build and insert loadI Instruction
			// loadI nextMemAddr => r0
			Instruction i {loadI};
			i.src1.sr = nextMemAddr;
			i.dest.isReg = true;
			i.dest.pr = k;
			intRep.insert(it, i);
//cerr << "allocate::inserting::loadI " << i.src1.sr << " => " << i.dest.pr << endl;
			// save address where vr's value is to be stored
			vr2mem[c.name[pr]] = nextMemAddr;
			nextMemAddr += 4;
			// build and insert store Instruction
			// store pr => r0
			i = Instruction {store};
			i.src1.isReg = true;
			i.src1.pr = pr;
			i.src2.isReg = true;
			i.src2.pr = k;
			intRep.insert(it, i);
			// mark as clean
			clean[c.name[pr]] = spilled;
		}
	}
	// set Class values to indicate pr is in use
	c.name[pr] = vr;
	c.next[pr] = INVALID;
	c.free[pr] = false;
	c.cclean[pr] = clean[vr];
	// return allocated pr
	return pr;
}


// selects optimal physical register
// to be overwritten and possibly spilled
int Allocator::optimalPR(Class& c) {
	int pr = INVALID;

	// if ramaterializable values exist, pick the one with max next use
	if (find(c.cclean.begin(), c.cclean.end(), remat) != c.cclean.end())
		pr = bestOfType(c, remat);
	// if clean registers exits, pick the one with max next use
	else if (find_if(c.cclean.begin(), c.cclean.end(),
			[](int i){ return i != dirty; }) != c.cclean.end())
		pr = bestOfType(c, dirty, true);
	// otherwise, pick register with max next use
	else {
		auto it = max_element(c.next.begin(), c.next.end());
		pr = it - c.next.begin();
	}
		

	return pr;
}


// returns physical register number
// of register with maximum next use and is clean in the manner specified by ctype
int Allocator::bestOfType(Class& c, Clean cln, bool n) {
	int pr = INVALID;
	int optNextUse = INVALID;
//	int optUses = INT_MAX;
	for (int i = 0; i < c.sz; ++i) {
		if (((n && c.cclean[i] != cln) || c.cclean[i] == cln)
									&& c.next[i] >= optNextUse) {
//			&& c.next[i] >= optNextUse && uses[c.name[i]] <= optUses) {
				pr = i;
				optNextUse = c.next[i];
//				optUses = uses[c.name[i]];
		}
	}
	return pr;
}


// frees a physical register
// sets Class values for pr to defaults, pushes onto stack.
void Allocator::freeRegister(int pr, Class& c) {
	c.name[pr] = INVALID;
	c.next[pr] = INT_MAX;
	c.free[pr] = true;
	c.cclean[pr] = dirty;
	c.stk.push(pr);
}


// compute live ranges of source registers, map
// each to distinct virtual register, set its
// next use, and track the number of live registers.
void Allocator::computeLastUses() {
	// initialize vectors (only used here and update)
	int numSR = getNumSR();
	vector<int> sr2vr (numSR, INVALID);
	vector<int> lastUse (numSR, INT_MAX);

	// for optimizations
	vector<pii> stores;	// first: src2.vr, second: address (value in src2.vr)
	// stores: [<dest vr, dest address>]
	// when encounter a store instruction, insert with its destination vr
	// when encounter loadI, check stores to find matching vr and save
	// address with the pair
	vector<pii> loads;	// first: src1.vr, second: dest.vr
	// loads: [<src vr, dest vr>]
	// when encounter a load, insert its vrs
	// when encounter loadI, check loads to find matching vr, grab its
	// addess, then see if we already encountered a store that overwrites
	// the load address. if not, the load is clean!

	int vrName = 0;
	int numLive = 0;
	int i = intRep.size();
	auto it = intRep.end();
	while (it != intRep.begin()) {
		--i;
		--it;
		// update and kill
		if (it->dest.isReg) {
			update(it->dest, i, vrName, numLive, sr2vr, lastUse);
			sr2vr[it->dest.sr] = INVALID;
			lastUse[it->dest.sr] = INT_MAX;
			// track number of live registers
			--numLive;
			// track store addresses (for clean load optimization)
			auto sit = find_if(stores.begin(), stores.end(),
					[&] (pii& s) { return s.first == it->dest.vr; });
			// if find a store that doesn't have an associated address
			if (sit != stores.end() && sit->second == INVALID) {
				if (it->op == loadI)
					sit->second = it->src1.sr;
				// if current instruction is not loadI, address was
				// modified --> remove store from consideration
				else
					stores.erase(sit);
			}
		}
		// update one use
		if (it->src1.isReg)
			update(it->src1, i, vrName, numLive, sr2vr, lastUse);
		// and the other use
		if (it->src2.isReg)
			update(it->src2, i, vrName, numLive, sr2vr, lastUse);

		// rematerializable optimization
		if (it->op == loadI) {
			clean[it->dest.vr] = remat;
			vr2mem[it->dest.vr] = it->src1.sr;
		}

		//// clean loads optimization ////

		// remember stores and loads
		if (it->op == store)
			stores.push_back(pii(it->src2.vr, INVALID));
		if (it->op == load)
			loads.push_back(pii(it->src1.vr, it->dest.vr));

		// when encounter loadI, check loads
		if (it->op == loadI) {
			int addr = it->src1.sr;
			// check loads
			for (auto lit = loads.begin(); lit != loads.end(); ++lit) {
//cerr << "next lit" << endl;
//			for (pii l : loads) {
				if (lit->first == it->dest.vr) {
//				if (l.first == it->dest.vr) {
					auto sit = find_if(stores.begin(), stores.end(),
								[&] (pii& s) { return s.second == addr; });
					if (sit == stores.end()) {
						clean[lit->second] = cleanLoad;
						vr2mem[lit->second] = addr;
						loads.erase(lit);
						break;
					}
				}
			}
		}

	}

/*
	// mark loads as clean if no store overrides
	for (auto l : loads) {
		auto sit = find_if(stores.begin(), stores.end(),
					[&] (pii& s) { return s.second == vr2mem[l.first]; });
		if (sit == stores.end()) {
			clean[l.second] = cleanLoad;
			vr2mem[l.second] = vr2mem[l.first];
		}
	}
*/

}


// helper function for computeLastUses()
// updates a Register by setting its virtual register and next use.
// updates vectors used to track live ranges and their last use.
void Allocator::update(Register& op, int ind, int& vrName, int& numLive,
								vector<int>& sr2vr, vector<int>& lastUse) {
	// if not in use, update sr2vr to next vr
	if (sr2vr[op.sr] == INVALID) {
		sr2vr[op.sr] = vrName++;
		// track number of live registers
		++numLive;
		if (numLive > maxLive)
			maxLive = numLive;
		// add live range to vectors
		vr2mem.push_back(INVALID);
//		uses.push_back(0);
		clean.push_back(dirty);
	}
	// map Register sr to vr
	op.vr = sr2vr[op.sr];
	// set Register nextUse
	op.nu = lastUse[op.sr];
	// update last use of vr to now
	lastUse[op.sr] = ind;
	// increment uses
//	++uses[op.vr];
}


// iterates through IR once and 
// returns larget source register number + 1
int Allocator::getNumSR() {
	int highSR = INVALID;
	auto it = intRep.begin();
	while (it != intRep.end()) {
		// in "reverse" order to reduce number of assignments
		if (it->dest.isReg && it->dest.sr > highSR)
			highSR = it->dest.sr;
		if (it->src2.isReg && it->src2.sr > highSR)
			highSR = it->src2.sr;
		if (it->src1.isReg && it->src1.sr > highSR)
			highSR = it->src1.sr;
		++it;
	}
	return highSR + 1;
}


// pretty tabular IR printing (for debug)
ostream& operator<<(ostream& os, const Allocator& a) {
	// print first line of table header
	os << "// ";
	os << "|index| opcode ||"
		"|               op1               |"
		"|            op2            |"
		"|            dest           ||" << endl;
	// print second line of table header
	os << "// ";
	os << "|     |        ||"
		"|     sr     |  vr  |  pr  |  nu  |" 
		"|  sr  |  vr  |  pr  |  nu  |"
		"|  sr  |  vr  |  pr  |  nu  || clean ||" << endl;
	// print rest of IR as table
	int ind = 0;
	auto it = a.intRep.begin();
	while (it != a.intRep.end()) {
		// print index
		os << "// |" << setw(5) << left << ind << "|";
		// print rest of line
		os << *it;
// print clean
		os << " ";
		switch (a.clean[it->dest.vr]) {
			case remat:
				os << "remat";
				break;
			case cleanLoad:
				os << "cload";
				break;
			case spilled:
				os << "spill";
				break;
			case dirty:
				os << "dirty";
				break;
			default:
				os << " wtf ";
				break;
		}
		os << " ||" << endl;

		++it;
		++ind;
	}

	return os;
}


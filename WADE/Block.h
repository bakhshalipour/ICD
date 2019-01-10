/*
The source code of "Reducing Writebacks Through In-Cache Displacement" paper, which is accepted in ACM TODAES 2019.

In this repo, we provide the source codes that are used in our ACM TODAES 2019 paper. This includes the implementation of our proposal, named ICD, as well as the implementation of competitor methods like WADE and HAP. We also provide the source code which acts as an interface between our simulator and Simics. This so-called Simics Interface extends the interface of Flexus simulator with Simics and gets the value of memory accesses, into the bargain.

Please cite the following paper when using the provided source codes:

M. Bakhshalipour, A. Faraji, A. Vakil-Ghahani, F. Samandi, P. Lotfi-Kamran, and H. Sarbazi-Azad, "Reducing Writebacks Through In-Cache Displacement," in ACM Transactions on Design Automation of Electronic Systems (TODAES), 2019.
*/

#ifndef BLOCK_H_
#define BLOCK_H_

#include "util.h"

#define BLOCK_VALID_BIT 1
#define BLOCK_DIRTY_BIT 2

class Block {
public:
    ll savedAddr;
	ll tag;
	byte status;	// valid bit
	std::string value;

	Block(ll tag = 0);
	virtual ~Block();

	bool is_dirty();
};

#endif /* BLOCK_H_ */

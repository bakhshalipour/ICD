/*
The source code of "Reducing Writebacks Through In-Cache Displacement" paper, which is accepted in ACM TODAES 2019.

In this repo, we provide the source codes that are used in our ACM TODAES 2019 paper. This includes the implementation of our proposal, named ICD, as well as the implementation of competitor methods like WADE and HAP. We also provide the source code which acts as an interface between our simulator and Simics. This so-called Simics Interface extends the interface of Flexus simulator with Simics and gets the value of memory accesses, into the bargain.

Please cite the following paper when using the provided source codes:

M. Bakhshalipour, A. Faraji, A. Vakil-Ghahani, F. Samandi, P. Lotfi-Kamran, and H. Sarbazi-Azad, "Reducing Writebacks Through In-Cache Displacement," in ACM Transactions on Design Automation of Electronic Systems (TODAES), 2019.
*/

#ifndef SET_H_
#define SET_H_

#include "util.h"
#include "Block.h"

#include <queue>
#include <vector>

class Set {
	int ways_num;
	std::vector<Block*> frequent_blocks;
	std::vector<Block*> nonfrequent_blocks;
	REP_POLICY *replacement_policy;

	bool even_write;

public:
	Set(int ways_num, REP_POLICY* replacement_policy);
	virtual ~Set();

	int index;

	void add_to_list(Block* block);
	bool lookup(ll tag, AccessType type, ll savedAddr, std::string value);
	void add(ll tag, AccessType type, ll savedAddr, std::string value);
};

#endif /* SET_H_ */

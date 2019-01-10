/*
The source code of "Reducing Writebacks Through In-Cache Displacement" paper, which is accepted in ACM TODAES 2019.

In this repo, we provide the source codes that are used in our ACM TODAES 2019 paper. This includes the implementation of our proposal, named ICD, as well as the implementation of competitor methods like WADE and HAP. We also provide the source code which acts as an interface between our simulator and Simics. This so-called Simics Interface extends the interface of Flexus simulator with Simics and gets the value of memory accesses, into the bargain.

Please cite the following paper when using the provided source codes:

M. Bakhshalipour, A. Faraji, A. Vakil-Ghahani, F. Samandi, P. Lotfi-Kamran, and H. Sarbazi-Azad, "Reducing Writebacks Through In-Cache Displacement," in ACM Transactions on Design Automation of Electronic Systems (TODAES), 2019.
*/

#ifndef UTIL_H_
#define UTIL_H_

#include <iostream>
#include <cstdint>
#include <cstdlib>
#include <fstream>

typedef long long ll;
typedef uint8_t byte;

#define INF 1000*1000*1000
#define EPS 1e-5

#define EXIT_FAILURE 1

#define FREQUENT_WRITEBACK 1
#define NONFREQUENT_WRITEBACK 0

#define PRINT(X) std::cerr << #X << " : " << X << "\n";

class MemAccess {
public:
	MemAccess(ll tag, ll index) : tag(tag), index(index) {}
	ll tag;
	ll index;
};

inline int logarithm2(ll x) {
	int cnt = 0;
	while(x > 1) {
		cnt++;
		x >>= 1;
	}
	return cnt;
}

enum AccessType {
	READ_REQ = 1,
	WRITE_REQ = 2,
	FETCH_REQ = 3,
	UPGRADE_REQ = 4,
	EVICTION_CLEAN = 5,
	EVICTION_DIRTY = 6,
	EVICTION_WRITABLE = 7
};

enum REP_POLICY {
	WADE_SEQ_0 = 1,
	WADE_SEQ_4 = 2,
	WADE_SEQ_8 = 3,
	WADE_SEQ_12 = 4,
	WADE_SEQ_16 = 5,
	WADE_SEQ_16_BYPASS = 6,
	WADE_DYNAMIC = 7,
	FOLLOWER = 8
};

class RepData {
public:
	RepData() {
		this->num_of_segment_set = 1;
		this->m = 16;
		this->n = 4;
		this->y = 4;
		this->fwp_ways_num = 6;
		this->l = 0;
		this->seed = 1;
	}
	int seed;
	int num_of_segment_set;
	int m;
	int n;
	int y;
	int fwp_ways_num;
	double l;
};

#endif /* UTIL_H_ */

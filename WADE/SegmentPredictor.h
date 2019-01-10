/*
The source code of "Reducing Writebacks Through In-Cache Displacement" paper, which is accepted in ACM TODAES 2019.

In this repo, we provide the source codes that are used in our ACM TODAES 2019 paper. This includes the implementation of our proposal, named ICD, as well as the implementation of competitor methods like WADE and HAP. We also provide the source code which acts as an interface between our simulator and Simics. This so-called Simics Interface extends the interface of Flexus simulator with Simics and gets the value of memory accesses, into the bargain.

Please cite the following paper when using the provided source codes:

M. Bakhshalipour, A. Faraji, A. Vakil-Ghahani, F. Samandi, P. Lotfi-Kamran, and H. Sarbazi-Azad, "Reducing Writebacks Through In-Cache Displacement," in ACM Transactions on Design Automation of Electronic Systems (TODAES), 2019.
*/

#ifndef SRC_SEGMENTPREDICTOR_H_
#define SRC_SEGMENTPREDICTOR_H_

#include "util.h"

#define MAX_PSEL_VALUE ((1 << 11) - 1)
#define MIN_PSEL_VALUE (-(1 << 11))

class SegmentPredictor {
public:
	int PSEL[5];
	double p;

	SegmentPredictor();
	virtual ~SegmentPredictor();

	void add_miss_penalty(REP_POLICY rep_policy, int value);
	REP_POLICY get_predicted_frequent_size();
	REP_POLICY get_predicted_dynamic_segment_size();

	bool is_p_double();

	void increment(int index, int value);
	void decrement(int index, int value);
};

#endif /* SRC_SEGMENTPREDICTOR_H_ */

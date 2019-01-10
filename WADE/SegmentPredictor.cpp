/*
The source code of "Reducing Writebacks Through In-Cache Displacement" paper, which is accepted in ACM TODAES 2019.

In this repo, we provide the source codes that are used in our ACM TODAES 2019 paper. This includes the implementation of our proposal, named ICD, as well as the implementation of competitor methods like WADE and HAP. We also provide the source code which acts as an interface between our simulator and Simics. This so-called Simics Interface extends the interface of Flexus simulator with Simics and gets the value of memory accesses, into the bargain.

Please cite the following paper when using the provided source codes:

M. Bakhshalipour, A. Faraji, A. Vakil-Ghahani, F. Samandi, P. Lotfi-Kamran, and H. Sarbazi-Azad, "Reducing Writebacks Through In-Cache Displacement," in ACM Transactions on Design Automation of Electronic Systems (TODAES), 2019.
*/

#include "SegmentPredictor.h"
#include "Cache.h"
#include <cmath>
#include <cassert>

SegmentPredictor::SegmentPredictor() {
	for(int i = 0; i < 5; i++)
		PSEL[i] = 0;
	this->p = 1.5 + 0.5 * Cache::rep_data->l;
}

SegmentPredictor::~SegmentPredictor() {
}

void SegmentPredictor::increment(int index, int value) {
	if(PSEL[index] + value <= MAX_PSEL_VALUE)
		PSEL[index] += value;
}

void SegmentPredictor::decrement(int index, int value) {
	if(PSEL[index] - value >= MIN_PSEL_VALUE)
		PSEL[index] -= value;
}

void SegmentPredictor::add_miss_penalty(REP_POLICY rep_policy, int value) {
	if(rep_policy == REP_POLICY::WADE_DYNAMIC)
		rep_policy = get_predicted_dynamic_segment_size();
	switch(rep_policy) {
	case REP_POLICY::WADE_SEQ_0:
		decrement(2, value);
		break;
	case REP_POLICY::WADE_SEQ_4:
		decrement(2, value);
		break;
	case REP_POLICY::WADE_SEQ_8:
		decrement(1, value);
		if(PSEL[1] > 0) {
			// 8 is winner in first race
			increment(2, value);
		}
		break;
	case REP_POLICY::WADE_SEQ_12:
		assert(PSEL[1] <= 0);
		decrement(2, value);
		break;
	case REP_POLICY::WADE_SEQ_16:
		increment(1, value);
		if(PSEL[1] <= 0) {
			// 16 is winner in first race
			increment(2, value);
			if(PSEL[2] <= 0) {
				// 16 is winner in second race
				increment(3, value);
			}
		}
		break;
	case REP_POLICY::WADE_SEQ_16_BYPASS:
		if(PSEL[1] <= 0 && PSEL[2] <= 0) {
			// 16 is winner in first and second race and in race with 16_bypass
			decrement(3, value);
		}
		break;
	}
//	std::cerr << int(rep_policy) << " " << value << "\t\t";
//	std::cerr << "PSEL : ";
//	for(int i = 1; i < 4; i++)
//		std::cerr << PSEL[i] << "\t";
//	std::cerr << "\n";
}

REP_POLICY SegmentPredictor::get_predicted_frequent_size() {
	if(PSEL[1] > 0) {
		// 8 is winner
		if(PSEL[2] <= 0) {
			// 8 is winner
			return REP_POLICY::WADE_SEQ_8;
		}
		else {
			// segment 4 never return!
			return this->get_predicted_dynamic_segment_size();
		}
	}
	else {
		// 16 is winner
		if(PSEL[2] <= 0) {
			// 16 is winner
			if(PSEL[3] <= 0) {
				// 16 is winner
				return REP_POLICY::WADE_SEQ_16;
			}
			else {
				// 16-bypass is winner
				return REP_POLICY::WADE_SEQ_16_BYPASS;
			}
		}
		else {
			// 12 is winner
			return REP_POLICY::WADE_SEQ_12;
		}
	}
}

REP_POLICY SegmentPredictor::get_predicted_dynamic_segment_size() {
	if(PSEL[1] > 0) {
		// 8 is winner
		if(PSEL[2] < 0) {
			// 8 is winner
			return REP_POLICY::WADE_SEQ_4;
		}
		else {
			return REP_POLICY::WADE_SEQ_0;
		}
	}
	else {
		// 16 is winner
		return REP_POLICY::WADE_SEQ_12;
	}
}

bool SegmentPredictor::is_p_double() {
	if(p - floor(p) > EPS)
		return true;
	return false;
}

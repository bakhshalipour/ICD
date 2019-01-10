/*
The source code of "Reducing Writebacks Through In-Cache Displacement" paper, which is accepted in ACM TODAES 2019.

In this repo, we provide the source codes that are used in our ACM TODAES 2019 paper. This includes the implementation of our proposal, named ICD, as well as the implementation of competitor methods like WADE and HAP. We also provide the source code which acts as an interface between our simulator and Simics. This so-called Simics Interface extends the interface of Flexus simulator with Simics and gets the value of memory accesses, into the bargain.

Please cite the following paper when using the provided source codes:

M. Bakhshalipour, A. Faraji, A. Vakil-Ghahani, F. Samandi, P. Lotfi-Kamran, and H. Sarbazi-Azad, "Reducing Writebacks Through In-Cache Displacement," in ACM Transactions on Design Automation of Electronic Systems (TODAES), 2019.
*/

#include "FWPEntry.h"
#include "Cache.h"

FWPEntry::FWPEntry(int tag, int set_index) : tag(tag) {
	if(set_index < 0 || set_index >= 32) {
		throw ("unknown fwp index for creating fwp entry");
	}
	this->frequency_counter = 0;
	this->flags = (1 << set_index);
//	this->lru_recency = 0;
}

FWPEntry::~FWPEntry() {
}

void FWPEntry::update_set_flag(int set_index) {
	if(set_index < 0 || set_index >= Cache::rep_data->m) {
		throw ("unknown fwp index for updating set flag");
	}
	this->flags |= (1 << set_index);
}

bool FWPEntry::get_set_flag(int set_index) {
	if(set_index < 0 || set_index >= Cache::rep_data->m) {
		throw ("unknown fwp index for getter of set flag");
	}
	if(this->flags & (1 << set_index))
		return true;
	return false;
}

int FWPEntry::get_victim_value(int lru_recency) {
	return this->frequency_counter + Cache::rep_data->y * lru_recency;
}

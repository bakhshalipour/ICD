/*
The source code of "Reducing Writebacks Through In-Cache Displacement" paper, which is accepted in ACM TODAES 2019.

In this repo, we provide the source codes that are used in our ACM TODAES 2019 paper. This includes the implementation of our proposal, named ICD, as well as the implementation of competitor methods like WADE and HAP. We also provide the source code which acts as an interface between our simulator and Simics. This so-called Simics Interface extends the interface of Flexus simulator with Simics and gets the value of memory accesses, into the bargain.

Please cite the following paper when using the provided source codes:

M. Bakhshalipour, A. Faraji, A. Vakil-Ghahani, F. Samandi, P. Lotfi-Kamran, and H. Sarbazi-Azad, "Reducing Writebacks Through In-Cache Displacement," in ACM Transactions on Design Automation of Electronic Systems (TODAES), 2019.
*/

#include "FWPSet.h"
#include "Cache.h"

FWPSet::FWPSet(int ways_num) : ways_num(ways_num) {

}

FWPSet::~FWPSet() {
	for(int i = 0; i < entries.size(); i++)
		delete(entries[i]);
}

bool FWPSet::lookup(ll tag, int set_index) {
	tag &= 0xFFFF;
	for(int i = 0; i < this->entries.size(); i++)
		if(entries[i]->tag == tag)
			return this->entries[i]->get_set_flag(set_index % Cache::rep_data->m);
	return false;
}

void FWPSet::update(ll tag, int set_index) {

//	for(int i = 0; i < this->entries.size(); i++)
//		this->entries[i]->lru_recency++;

	tag &= 0xFFFF;

	for(int i = 0; i < this->entries.size(); i++)
		if(entries[i]->tag == tag) {
			// fwp hit
			FWPEntry* tmp = entries[i];
			entries.erase(entries.begin());
			tmp->frequency_counter++;
			tmp->update_set_flag(set_index % Cache::rep_data->m);
			entries.push_back(tmp);
//			entries[i]->lru_recency = 0;
			return;
		}

	// fwp miss
	if(this->entries.size() < this->ways_num) {
		FWPEntry* new_entry = new FWPEntry(tag, set_index % Cache::rep_data->m);
		this->entries.push_back(new_entry);
		return;
	}

	int victim_index = this->get_victim_index();
	this->entries.erase(this->entries.begin() + victim_index);

	FWPEntry* new_entry = new FWPEntry(tag, set_index % Cache::rep_data->m);
	this->entries.push_back(new_entry);
}

int FWPSet::get_victim_index() {
	int min_value = INF, index = -1;
	for(int i = 0; i < this->entries.size(); i++) {
		int tmp = this->entries[i]->get_victim_value(i);
		if(min_value > tmp) {
			min_value = tmp;
			index = i;
		}
	}
	return index;
}

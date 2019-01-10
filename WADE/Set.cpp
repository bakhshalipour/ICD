/*
The source code of "Reducing Writebacks Through In-Cache Displacement" paper, which is accepted in ACM TODAES 2019.

In this repo, we provide the source codes that are used in our ACM TODAES 2019 paper. This includes the implementation of our proposal, named ICD, as well as the implementation of competitor methods like WADE and HAP. We also provide the source code which acts as an interface between our simulator and Simics. This so-called Simics Interface extends the interface of Flexus simulator with Simics and gets the value of memory accesses, into the bargain.

Please cite the following paper when using the provided source codes:

M. Bakhshalipour, A. Faraji, A. Vakil-Ghahani, F. Samandi, P. Lotfi-Kamran, and H. Sarbazi-Azad, "Reducing Writebacks Through In-Cache Displacement," in ACM Transactions on Design Automation of Electronic Systems (TODAES), 2019.
*/

#include <assert.h>
#include "Set.h"
#include "Cache.h"
#include <cmath>
#include <algorithm>
#include <cstring>

extern long double timer;
extern long double clkStep;
extern std::ofstream out;

Set::Set(int ways_num, REP_POLICY* replacement_policy) :
		ways_num(ways_num), replacement_policy(
				replacement_policy) {
	index = -1;
	even_write = false;
}

Set::~Set() {
	for(int i = 0; i < frequent_blocks.size(); i++)
		delete(frequent_blocks[i]);
	for(int i = 0; i < nonfrequent_blocks.size(); i++)
		delete(nonfrequent_blocks[i]);
	delete(replacement_policy);
}

void Set::add_to_list(Block* block) {
//	if(index == 1806)
//		std::cerr << "! 1\n";
	MemAccess fwp_address = Cache::convert_set_access_to_fwp_access(MemAccess(block->tag, index));
//	if(index == 1806)
//		std::cerr << "! 2\n";
	bool fbit = Cache::fwp_sets[fwp_address.index]->lookup(fwp_address.tag, index);
//	if(index == 1806)
//		std::cerr << "! 3\n";
	if(fbit == true)
		frequent_blocks.push_back(block);
	else
		nonfrequent_blocks.push_back(block);
//	if(index == 1806)
//		std::cerr << "! 4\n";
}

bool Set::lookup(ll tag, AccessType type, ll savedAddr, std::string value) {
//	if(index == 1806)
//		std::cerr << "1\n";
	for (int i = 0; i < this->frequent_blocks.size(); i++)
		if (this->frequent_blocks[i]->tag == tag) {
			// hit
			Block* tmp = this->frequent_blocks[i];
			frequent_blocks.erase(frequent_blocks.begin() + i);
            tmp->value = value; 
			if (type == AccessType::EVICTION_DIRTY) {
				tmp->status |= BLOCK_DIRTY_BIT;
				add_to_list(tmp);
			}
			else
				frequent_blocks.push_back(tmp);
//			if(index == 1806)
//				std::cerr << "hit in frequent blocks\n";
			return true;
		}
	for (int i = 0; i < this->nonfrequent_blocks.size(); i++)
		if (this->nonfrequent_blocks[i]->tag == tag) {
			// hit
			Block* tmp = this->nonfrequent_blocks[i];
			nonfrequent_blocks.erase(nonfrequent_blocks.begin() + i);
            tmp->value = value; 
			if (type == AccessType::EVICTION_DIRTY) {
				tmp->status |= BLOCK_DIRTY_BIT;
				add_to_list(tmp);
			}
			else
				nonfrequent_blocks.push_back(tmp);
//			if(index == 1806)
//				std::cerr << "hit in non-frequent blocks\n";
			return true;
		}
//	if(index == 1806)
//		std::cerr << "miss\n";
	// miss
	this->add(tag, type, savedAddr, value);
	return false;
}

void Set::add(ll tag, AccessType type, ll savedAddr, std::string value){
	if(this->frequent_blocks.size() + this->nonfrequent_blocks.size() < this->ways_num) {
//		std::cerr << "still not full\n";
		Block* new_block = new Block(tag);
        new_block->savedAddr = savedAddr;
		new_block->status = BLOCK_VALID_BIT;
		new_block->value = value;
		if(type == AccessType::EVICTION_DIRTY)
			new_block->status |= BLOCK_DIRTY_BIT;
		add_to_list(new_block);
		return;
	}
	REP_POLICY set_rep_policy = *replacement_policy;
	if(*replacement_policy == REP_POLICY::FOLLOWER) {
		set_rep_policy = Cache::segment_predictor->get_predicted_frequent_size();
	}
	else if(*replacement_policy == REP_POLICY::WADE_DYNAMIC) {
		set_rep_policy = Cache::segment_predictor->get_predicted_dynamic_segment_size();
	}
//	int predicted_segment_size = int(set_rep_policy) * 4 - 4; //for 16 way
    int predicted_segment_size = int(set_rep_policy) * 2 - 2; //for 8 way
//	PRINT(predicted_segment_size);
	if(set_rep_policy == REP_POLICY::WADE_SEQ_16_BYPASS) {
		if(type != AccessType::EVICTION_DIRTY) {
//			if(*replacement_policy != REP_POLICY::FOLLOWER)
//				Cache::segment_predictor->add_miss_penalty(*(this->replacement_policy), 1);
			return;
		}
//		predicted_segment_size = 16; //for 16 way
        predicted_segment_size = 8; //for 8 way
	}

//	assert(predicted_segment_size <= 16 && predicted_segment_size >= 0);
    assert(predicted_segment_size <= 8 && predicted_segment_size >= 0);

	Block* evicted_block;
	if(this->frequent_blocks.size() == this->ways_num) {
		evicted_block = this->frequent_blocks[0];
		this->frequent_blocks.erase(this->frequent_blocks.begin());
	}
	else {
		if(this->nonfrequent_blocks.size() == this->ways_num) {
			evicted_block = this->nonfrequent_blocks[0];
			this->nonfrequent_blocks.erase(this->nonfrequent_blocks.begin());
		}
		else {
			if (this->frequent_blocks.size() > predicted_segment_size) {
				assert(this->frequent_blocks.size() > 0);
				evicted_block = this->frequent_blocks[0];
				this->frequent_blocks.erase(this->frequent_blocks.begin());
			} else {
				assert(this->nonfrequent_blocks.size() > 0);
				evicted_block = this->nonfrequent_blocks[0];
				this->nonfrequent_blocks.erase(this->nonfrequent_blocks.begin());
			}
		}
	}

	if(*(this->replacement_policy) != REP_POLICY::FOLLOWER) {
		if(evicted_block->is_dirty()) {
			if(Cache::segment_predictor->is_p_double()) {
				if(this->even_write == true)
					Cache::segment_predictor->add_miss_penalty(*(this->replacement_policy), 2 * Cache::segment_predictor->p);
				this->even_write = !this->even_write;
			}
			else {
				Cache::segment_predictor->add_miss_penalty(*(this->replacement_policy), Cache::segment_predictor->p);
			}
		}
		else
			Cache::segment_predictor->add_miss_penalty(*(this->replacement_policy), 1);
	}

	if(*this->replacement_policy == REP_POLICY::FOLLOWER && evicted_block->is_dirty()) {
		Cache::writebacks++;

#ifdef GENERATE_TIME_TRACE
        out << evicted_block->savedAddr << " W " << std::fixed << timer << " " << evicted_block->value << std::endl;
#endif

		MemAccess fwp_address = Cache::convert_set_access_to_fwp_access(MemAccess(evicted_block->tag, index));
//		std::cerr << std::hex << fwp_address.tag << " " << this->index << "\n" << std::dec;
		Cache::fwp_sets[fwp_address.index]->update(fwp_address.tag, this->index);
	}
	delete(evicted_block);

	Block* new_block = new Block(tag);
    new_block->savedAddr = savedAddr;
    new_block->value = value; 
	new_block->status = BLOCK_VALID_BIT;
	if(type == AccessType::EVICTION_DIRTY)
		new_block->status |= BLOCK_DIRTY_BIT;
	add_to_list(new_block);
}

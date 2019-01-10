/*
The source code of "Reducing Writebacks Through In-Cache Displacement" paper, which is accepted in ACM TODAES 2019.

In this repo, we provide the source codes that are used in our ACM TODAES 2019 paper. This includes the implementation of our proposal, named ICD, as well as the implementation of competitor methods like WADE and HAP. We also provide the source code which acts as an interface between our simulator and Simics. This so-called Simics Interface extends the interface of Flexus simulator with Simics and gets the value of memory accesses, into the bargain.

Please cite the following paper when using the provided source codes:

M. Bakhshalipour, A. Faraji, A. Vakil-Ghahani, F. Samandi, P. Lotfi-Kamran, and H. Sarbazi-Azad, "Reducing Writebacks Through In-Cache Displacement," in ACM Transactions on Design Automation of Electronic Systems (TODAES), 2019.
*/

#ifndef CACHE_H_
#define CACHE_H_

#include "util.h"
#include "Set.h"
#include "Block.h"
#include "FWPSet.h"
#include "SegmentPredictor.h"

class SegmentPredictorStatistics {
public:
	SegmentPredictorStatistics() {
		for(int i = 0; i < 10; i++)
			count[i] = 0;
	}
	int count[10];
};

class Cache {
public:

	static int sets_num;
	static int fwp_sets_num;
	int cache_size;
	int ways_num;
	int block_size;
	Set** sampler_sets;
	Set** sets;
	static FWPSet** fwp_sets;

	static int total_reads;
	static int read_hits;

    static long total_accs;
    static long accs_hits;

	static int writebacks;
	static SegmentPredictorStatistics segment_predictor_statistics;
	static RepData* rep_data;
	static SegmentPredictor* segment_predictor;

	void update_statistics(AccessType type, bool res);

	Cache(int way, int block_size, int cache_size, RepData* rep_data);
	virtual ~Cache();
	void lookup(ll mem_addr, AccessType type, std::string value);
	static MemAccess convert_set_access_to_fwp_access(MemAccess mem_access);
};

#endif /* CACHE_H_ */

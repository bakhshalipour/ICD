/*
The source code of "Reducing Writebacks Through In-Cache Displacement" paper, which is accepted in ACM TODAES 2019.

In this repo, we provide the source codes that are used in our ACM TODAES 2019 paper. This includes the implementation of our proposal, named ICD, as well as the implementation of competitor methods like WADE and HAP. We also provide the source code which acts as an interface between our simulator and Simics. This so-called Simics Interface extends the interface of Flexus simulator with Simics and gets the value of memory accesses, into the bargain.

Please cite the following paper when using the provided source codes:

M. Bakhshalipour, A. Faraji, A. Vakil-Ghahani, F. Samandi, P. Lotfi-Kamran, and H. Sarbazi-Azad, "Reducing Writebacks Through In-Cache Displacement," in ACM Transactions on Design Automation of Electronic Systems (TODAES), 2019.
*/

#include "Cache.h"
#include "util.h"
#include <algorithm>
#include <cstring>

extern long double timer;
extern long double clkStep;
extern std::ofstream out;

int Cache::sets_num = 0;
int Cache::fwp_sets_num = 0;
int Cache::total_reads = 0;
int Cache::read_hits = 0;
int Cache::writebacks = 0;
long Cache::total_accs = 0;
long Cache::accs_hits = 0;

RepData* Cache::rep_data = NULL;
SegmentPredictor* Cache::segment_predictor = NULL;
FWPSet** Cache::fwp_sets = NULL;
SegmentPredictorStatistics Cache::segment_predictor_statistics;

Cache::Cache(int ways_num, int block_size, int cache_size, RepData* rep_data) :
		cache_size(cache_size), ways_num(
				ways_num), block_size(block_size) {
	sets_num = cache_size / (ways_num * block_size);
	Cache::rep_data = rep_data;
	fwp_sets_num = (sets_num / rep_data->m) * rep_data->n;

	sampler_sets = new Set*[4 * rep_data->num_of_segment_set];

	for(int i = 0; i < rep_data->num_of_segment_set; i++) {
		sampler_sets[i] = new Set(ways_num, new REP_POLICY(REP_POLICY::WADE_SEQ_8));
		sampler_sets[i]->index = i;
	}

	for(int i = 1 * rep_data->num_of_segment_set; i < 2 * rep_data->num_of_segment_set; i++) {
		sampler_sets[i] = new Set(ways_num, new REP_POLICY(REP_POLICY::WADE_SEQ_16));
		sampler_sets[i]->index = i % (rep_data->num_of_segment_set);
//		sampler_sets[i]->index = i;
	}

	for(int i = 2 * rep_data->num_of_segment_set; i < 3 * rep_data->num_of_segment_set; i++) {
		sampler_sets[i] = new Set(ways_num, new REP_POLICY(REP_POLICY::WADE_DYNAMIC));
		sampler_sets[i]->index = i % (rep_data->num_of_segment_set);
//		sampler_sets[i]->index = i;
	}

	for(int i = 3 * rep_data->num_of_segment_set; i < 4 * rep_data->num_of_segment_set; i++) {
		sampler_sets[i] = new Set(ways_num, new REP_POLICY(REP_POLICY::WADE_SEQ_16_BYPASS));
		sampler_sets[i]->index = i % (rep_data->num_of_segment_set);
//		sampler_sets[i]->index = i;
	}

	sets = new Set*[sets_num];
	for(int i = 0; i < sets_num; i++)
		sets[i] = new Set(ways_num, new REP_POLICY(REP_POLICY::FOLLOWER));

//	srand(rep_data->seed);
//	std::random_shuffle(sets, sets + sets_num);
	for(int i = 0; i < sets_num; i++)
		sets[i]->index = i;

	fwp_sets = new FWPSet*[fwp_sets_num];
	for(int i = 0; i < fwp_sets_num; i++)
		fwp_sets[i] = new FWPSet(rep_data->fwp_ways_num);

	segment_predictor = new SegmentPredictor();
}

Cache::~Cache() {
	for(int i = 0; i < sets_num; i++)
		delete(sets[i]);
	delete[] sets;
}

void Cache::lookup(ll mem_addr, AccessType type, std::string value) {

	ll index = mem_addr >> logarithm2(block_size);
	index &= sets_num - 1;
	ll tag = mem_addr >> logarithm2(block_size * sets_num);
//	std::cerr << index << "\n";

	if(index < rep_data->num_of_segment_set) {
//		std::cerr << "lookup in sampler sets\n";
		for(int i = 0; i < 4; i++)
			sampler_sets[index + i * rep_data->num_of_segment_set]->lookup(tag, type, mem_addr, value);
//		std::cerr << "end of lookup in sampler sets\n";
	}

//	if(index < 4 * rep_data->num_of_segment_set) {
//		sampler_sets[index]->lookup(tag, type);
//	}

	bool res = sets[index]->lookup(tag, type, mem_addr, value);

#ifdef GENERATE_TIME_TRACE
    if (!res) {
        //if missed
        out << mem_addr << " R " << std::fixed << timer << " " << value << std::endl;
    }

#endif

	update_statistics(type, res);
//	std::cerr << "!! " << index << "\n";
	return;
}

void Cache::update_statistics(AccessType type, bool res) {
	REP_POLICY rep_policy = this->segment_predictor->get_predicted_frequent_size();
	Cache::segment_predictor_statistics.count[int(rep_policy)]++;
	if(type == AccessType::READ_REQ || type == AccessType::FETCH_REQ || type == AccessType::WRITE_REQ) {
		Cache::total_reads++;
		Cache::read_hits += res;
	}

    Cache::total_accs++;
    Cache::accs_hits += res;
}

MemAccess Cache::convert_set_access_to_fwp_access(MemAccess mem_access) {
	ll index = mem_access.index >> (logarithm2(rep_data->m));
	ll k = mem_access.tag & (rep_data->n - 1);
	index |= k << (logarithm2(Cache::sets_num) - logarithm2(rep_data->m));

	ll tag = mem_access.tag >> logarithm2(rep_data->n);

//	std::cerr << std::hex << mem_access.tag << " " << mem_access.index << "\t" << tag << " " << index << "\n" << std::dec;

	return MemAccess(tag, index);
}

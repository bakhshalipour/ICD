/*
The source code of "Reducing Writebacks Through In-Cache Displacement" paper, which is accepted in ACM TODAES 2019.

In this repo, we provide the source codes that are used in our ACM TODAES 2019 paper. This includes the implementation of our proposal, named ICD, as well as the implementation of competitor methods like WADE and HAP. We also provide the source code which acts as an interface between our simulator and Simics. This so-called Simics Interface extends the interface of Flexus simulator with Simics and gets the value of memory accesses, into the bargain.

Please cite the following paper when using the provided source codes:

M. Bakhshalipour, A. Faraji, A. Vakil-Ghahani, F. Samandi, P. Lotfi-Kamran, and H. Sarbazi-Azad, "Reducing Writebacks Through In-Cache Displacement," in ACM Transactions on Design Automation of Electronic Systems (TODAES), 2019.
*/

#include <iostream>
#include <cstdio>
#include <fstream>
#include <sstream>

#include "util.h"
#include <cstdlib>
#include <cstring>
#include "Cache.h"

using namespace std;

Cache *cache;
long double timer;
long double clkStep;
ofstream out;

AccessType get_access_type(char* str1, char* str2) {
	if(strcmp(str1, "Eviction") == 0) {
		if(strcmp(str2, "(clean)") == 0)
			return AccessType::EVICTION_CLEAN;
		else if(strcmp(str2, "(dirty)") == 0)
			return AccessType::EVICTION_DIRTY;
		else if(strcmp(str2, "(writable)") == 0)
			return AccessType::EVICTION_WRITABLE;
		else {
			std::cerr << str1 << " " << str2 << "\n";
			throw ("invalid access type");
		}
	}
	else if(strcmp(str1, "Write") == 0)
		return AccessType::WRITE_REQ;
	else if(strcmp(str1, "Read") == 0)
		return AccessType::READ_REQ;
	else if(strcmp(str1, "Fetch") == 0)
		return AccessType::FETCH_REQ;
	else if(strcmp(str1, "Upgrade") == 0)
		return AccessType::UPGRADE_REQ;
	else {
		std::cerr << str1 << " " << str2 << "\n";
		throw ("invalid access type");
	}
}

int main(int argc, const char* argv[]) {
#ifdef GENERATE_TIME_TRACE

    if (argc != 3) {
        cout << "Runtime Argument Bad Format\n";
        exit(EXIT_FAILURE);
    }

#endif

//	std::cerr << "Enter input file, then warmup_value, cache_size, random_seed, number of segment sets, l, clkStep, and output file\n";

	srand(0);
//	string file_name;
//	cin >> file_name;

//	int warmup_value;
//	cin >> warmup_value;

	int cache_size = 512; //KiB
    int cache_assoc = 8;
//	cin >> cache_size;

//	FILE* input_file = fopen(file_name.c_str(), "r");
//	ifstream fin(file_name.c_str()); 
//	if(input_file == nullptr) {
//		std::cerr << "Invalid path file\n";
//		exit(1);
//	}

	RepData rep_data;
//	cin >> rep_data.seed >> rep_data.num_of_segment_set >> rep_data.l;
    rep_data.seed = 1;
    rep_data.num_of_segment_set = 16;
    rep_data.l = 1;

#ifdef GENERATE_TIME_TRACE
//    cin >> clkStep;
    clkStep = atof(argv[1]);
    out.open(argv[2]);

    if (!out.is_open()) {
        cout << "Specified File Cannot Be Opened\n";
        exit(EXIT_FAILURE);
    }

#endif

	cache = new Cache(cache_assoc, 64, cache_size * (1<<10), &rep_data); 
    long long unsigned pc; 
	ll mem_addr;
	char str[10000];
    string value;
	char str1[250], str2[250];
	int total = 0;
//	while(fscanf(input_file, "%lld%lld", &pc, &mem_addr) != EOF) {
//	while(fscanf(input_file, "%lld%lld%s%s\n", &pc, &mem_addr, str1, str2) != EOF) {

	while(cin.getline(str, 10000)) {
		if(isdigit(str[0]) == false)
			continue;
		stringstream ss;
		ss << str;
		ss >> pc >> mem_addr >> str1 >> str2;
        getline(ss, value);
//		if(total > 23500000) {
//			std::cerr << total << "\t" << pc << "\t" << mem_addr << "\t" << str1 << " " << str2 << "\n";
//		}
//		std::cerr << "get access type start\n";
		AccessType type = get_access_type(str1, str2);
//		std::cerr << "done_getting\n";
		if(type == AccessType::UPGRADE_REQ)
			continue;
//		if(type == AccessType::EVICTION_CLEAN || type == AccessType::EVICTION_WRITABLE)
//			continue; 
//		total++; 
		if(total == 0) {
			std::cerr << "-------------------- ";
			std::cerr << "end of warmup --------------------\n";
			Cache::writebacks = 0;
			Cache::read_hits = 0;
			Cache::total_reads = 0;
            Cache::total_accs = 0;
            Cache::accs_hits = 0;
			for(int i = 1; i <= 6; i++)
				Cache::segment_predictor_statistics.count[i] = 0;
		}
		if(total % (1000*1000) == 0) {
			std::cerr << total / 1000000 << "\t" << pc << "\t" << mem_addr << "\t" << str1 << " " << str2 << "\n";
			for(int i = 1; i <= 3; i++)
				std::cerr << cache->segment_predictor->PSEL[i] << " ";
			std::cerr << cache->segment_predictor->get_predicted_dynamic_segment_size() << "\t"
					<< cache->segment_predictor->get_predicted_frequent_size() << "\n";
		}
        total++; 
		cache->lookup(mem_addr, type, value);

#ifdef GENERATE_TIME_TRACE
        if ( (type == AccessType::READ_REQ) || (type == AccessType::WRITE_REQ) ) {
            timer += clkStep;
        }
#endif
        
	}
	double read_hit_ratio = double(Cache::read_hits)/double(Cache::total_reads);
    double total_hit_ratio = double(Cache::accs_hits)/double(Cache::total_accs);
	std::cout << "read_hits : " << Cache::read_hits << "\n";
	std::cout << "total_reads : " << Cache::total_reads << "\n";
	std::cout << "read_hit_ratio : " << read_hit_ratio << "\n";
    std::cout << "number of writebacks : " << Cache::writebacks << "\n";
    
    std::cout << "accs_hits : " << Cache::accs_hits << "\n";
    std::cout << "total_accs : " << Cache::total_accs << "\n";
    std::cout << "total_hit_ratio : " << total_hit_ratio << "\n";
    
	std::cout << "total accesses : " << total << "\n";
	for(int i = 1; i <= 6; i++)
		std::cout << Cache::segment_predictor_statistics.count[i] << "\t";
	std::cout << "\n";

//	fclose(input_file);
	delete(cache);
	return 0;
}

/*
The source code of "Reducing Writebacks Through In-Cache Displacement" paper, which is accepted in ACM TODAES 2019.

In this repo, we provide the source codes that are used in our ACM TODAES 2019 paper. This includes the implementation of our proposal, named ICD, as well as the implementation of competitor methods like WADE and HAP. We also provide the source code which acts as an interface between our simulator and Simics. This so-called Simics Interface extends the interface of Flexus simulator with Simics and gets the value of memory accesses, into the bargain.

Please cite the following paper when using the provided source codes:

M. Bakhshalipour, A. Faraji, A. Vakil-Ghahani, F. Samandi, P. Lotfi-Kamran, and H. Sarbazi-Azad, "Reducing Writebacks Through In-Cache Displacement," in ACM Transactions on Design Automation of Electronic Systems (TODAES), 2019.
*/

#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <cmath>
#include <map>
#include "CompleteCache.h"
#include "Coalesce.h"
#include <assert.h>

#define CACHE_SIZE 512 //KiB
#define ASSOCIATIVITY 4
#define BLOCK_SIZE 64 //B

#define TILE_COUNT_LOG2 0
#define COHERENCE_UNIT_LOG2 6


 #define LRU_RUN
 // #define COUNT_EXACT2WBS
// #define CUCKOO_RUN
// #define GENERATE_TIME_TRACE
// #define COALESCE


#define VERSION "2.55 (Final Traces)"

#define LINE_WIDTH 48
#define PRINT_VERSION cout << "rev" << VERSION << endl
#define PRINT_MULT(char, count) (cout << setfill(char) << setw(count) << "")

using namespace std;

enum AccessType {READ, WRITE, FETCH, EVICT_CLEAN, EVICT_DIRTY, EVICT_WRTIABLE, UPGRADE};

struct DataElement {
    lli address;
    AccessType accessType;
    string value;
};

// struct DataStruct {
//     bool dirty;
//     bool affectReadHitRatio;
//
// };

time_t start;
string fileName;
lli ignoreEnd;
ofstream out;
long double timer;
long double clkStep;
CompleteCache* l2AndCuckoo;
Cache* lru;
Coalesce* coalesce;

#ifdef COUNT_EXACT2WBS
map<lli, unsigned int> countExact2WBs;
#endif

// vector<pair<lli, DataStruct>> data;
// vector<pair<lli, AccessType>> data;
// vector<DataElement> data;
int maxBlockOffsetNum, blockOffset, maxIndex, setCount, indexSize, setCount_log2;

void calculateRange();
void postReport();
void preReport();

int main(int argc, const char* argv[]) {

#if defined(LRU_RUN) && defined(CUCKOO_RUN)
    cout << "Cannot run both CUCKOO_RUN and LRU_RUN\n";
    exit(EXIT_FAILURE);
#endif

#if defined(GENERATE_TIME_TRACE) && defined(LRU_RUN)
    if (argc != 3) {
        cout << "Runtime Argument Bad Format\n";
        exit(EXIT_FAILURE);
    }

    out.open(argv[2]);

    if (!out.is_open()) {
        cout << "Specified File Cannot Be Opened\n";
        exit(EXIT_FAILURE);
    }

    clkStep = atof(argv[1]);
#elif defined(LRU_RUN)
    if (argc != 1) {
        cout << "Runtime Argument Bad Format\n";
        exit(EXIT_FAILURE);
    }
#endif

#if defined(GENERATE_TIME_TRACE) && defined(CUCKOO_RUN)
     if (argc != 5) {
         cout << "Runtime Argument Bad Format\n";
         exit(EXIT_FAILURE);
     }

     int cuckooWayCount = atoi(argv[1]);
     int threshold = atoi(argv[2]);
     out.open(argv[4]);

     if (!out.is_open()) {
        cout << "Specified File Cannot Be Opened\n";
        exit(EXIT_FAILURE);
     }

     clkStep = atof(argv[3]);
#elif defined(CUCKOO_RUN)
    if (argc != 3) {
        cout << "Runtime Argument Bad Format\n";
        exit(EXIT_FAILURE);
    }

    int cuckooWayCount = atoi(argv[1]);
    int threshold = atoi(argv[2]);
#endif

    // fileName = argv[1];
    // cin.open(argv[1]);
    //
    // if (!cin.is_open()) {
    //     cout << "Specified File Cannot Be Opened\n";
    //     exit(EXIT_FAILURE);
    // }
    
    preReport();
    // calculateRange();
    start = time(NULL);
    maxBlockOffsetNum = (BLOCK_SIZE / 4) - 1;
    blockOffset = 0;
    maxIndex = (((CACHE_SIZE * 1024) / BLOCK_SIZE) / ASSOCIATIVITY) - 1;
    setCount = maxIndex + 1;
    indexSize = 0;
    setCount_log2 = log2(setCount);
    
    while (maxBlockOffsetNum != 0) {
        blockOffset++;
        maxBlockOffsetNum = maxBlockOffsetNum >> 1;
    }
    
    while (maxIndex != 0) {
        indexSize++;
        maxIndex = maxIndex >> 1;
    }

#ifdef COALESCE
    coalesce = new Coalesce;
#endif

    //initialize cache object
#ifdef LRU_RUN
    PRINT_MULT('-', LINE_WIDTH / 2 - 5);
    cout << " LRU RUN ";
    PRINT_MULT('-', LINE_WIDTH / 2 - 4);
    cout << endl;

#ifdef GENERATE_TIME_TRACE
    timer = 0;
    PRINT_MULT('-', LINE_WIDTH / 2 - 11);
    cout << " GENERATE TIME TRACE ";
    PRINT_MULT('-', LINE_WIDTH / 2 - 10);
    cout << endl;
#endif
    lru = new Cache(setCount, ASSOCIATIVITY, blockOffset, indexSize);
#endif

    //initialize cache object
#ifdef CUCKOO_RUN
    PRINT_MULT('-', LINE_WIDTH / 2 - 6);
    cout << " CUCKOO RUN ";
    PRINT_MULT('-', LINE_WIDTH / 2 - 6);
    cout << endl;

#ifdef GENERATE_TIME_TRACE
    timer = 0;
    PRINT_MULT('-', LINE_WIDTH / 2 - 11);
    cout << " GENERATE TIME TRACE ";
    PRINT_MULT('-', LINE_WIDTH / 2 - 10);
    cout << endl;
#endif
    l2AndCuckoo = new CompleteCache(setCount, ASSOCIATIVITY - cuckooWayCount, setCount, setCount_log2, cuckooWayCount, blockOffset, threshold, indexSize, setCount_log2, TILE_COUNT_LOG2, COHERENCE_UNIT_LOG2);
#endif
    
#ifdef COALESCE
    PRINT_MULT('-', LINE_WIDTH / 2 - 5);
    cout << " COALESCE ";
    PRINT_MULT('-', LINE_WIDTH / 2 - 5);
    cout << endl;
#endif

#ifdef COUNT_EXACT2WBS
    PRINT_MULT('-', LINE_WIDTH / 2 - 9);
    cout << " COUNT_EXACT2WBS ";
    PRINT_MULT('-', LINE_WIDTH / 2 - 8);
    cout << endl;
#endif

    lli inp;
    //FORMAT: %d   %d   %s   %s   %s

    while (true) {
        cin >> inp; //read next first number

        if (cin.eof()) {
            break;
        } else if (!cin.fail()) {
            break;
        } else {
            string dump;
            cin.clear();
            getline(cin, dump);
        }

    }

    string first, second, value;
    bool ignore;

    while (!cin.eof()) {
        
        if (cin.fail()) {
            cin.clear();
            assert(false && "bad 'first n lines were ignored'");
            break;
        }
        
        ignore = false;
        cin >> inp; //dump first number & get the address
        cin >> first; //dump first string
        cin >> second; //dump second string
        getline(cin, value);
        
//        if (first[0] == 'U') {
//            cin >> inp;
//            continue; //ignore Upgrade
//        }

        // DataStruct newVal;
        // newVal.dirty = ( (first == "Eviction") && (second == "(dirty)") ) ? (true) : (false);
        // newVal.affectReadHitRatio = (first != "Eviction");
        AccessType newType;

        switch(first[0]) {
            case 'F':
                newType = FETCH;
                break;

            case 'R':
                newType = READ;
                break;

            case 'W':
                newType = WRITE;
                break;

            case 'E':

                if (second[1] == 'w') {
                    newType = EVICT_WRTIABLE;
                } else if (second[1] == 'c') {
                    newType = EVICT_CLEAN;
                } else if (second[1] == 'd') {
                    newType = EVICT_DIRTY;
                } else {
                    assert(false && "bad trace");
                }

                break;

            case 'U':
                ignore = true;
                break;

            default:
                assert(false && "bad trace");
                break;
        }
        
        if (!ignore) {
            //data.push_back(make_pair(inp, newVal));
            // DataElement newElement;
            // newElement.address = inp;
            // newElement.accessType = newType;
            // newElement.value = value;
            // data.push_back(newElement);

            //send query
#ifdef LRU_RUN            
            bool dirty = (newType == EVICT_DIRTY);
            bool affectReadHitRatio = ((newType == READ) || (newType == WRITE) || (newType == FETCH));
            auto ret = lru->request(inp, dirty, affectReadHitRatio, value);

#ifdef COUNT_EXACT2WBS
            if (!ret.first) {
                //following if must be inside because if ret.first is true, ret.second is nullptr
                if (ret.second->isDirty()) {
                    //dirty eviction
                    lli blockID = ret.second->getContent().addr >> (blockOffset + 2);
                    countExact2WBs[blockID]++;
                }

            }
#endif

#ifdef COALESCE
            if (!ret.first) {
                lli qCounter = lru->getHitCount() + lru->getMissCount() - 1;
                lli blockAddr = inp >> (blockOffset + 2); // (block offset + byte offset)

                if (dirty) { //if missed (not in cache) and it is a write
                    coalesce->writeQuery(blockAddr, qCounter);
                }

                if (ret.second->isDirty()) { //dirty eviction
                    coalesce->dirtyEviction(ret.second->getContent().addr >> (blockOffset + 2), qCounter);
                }

            }
#endif

#ifdef GENERATE_TIME_TRACE
            if (!ret.first) {
                //miss
                out << inp << " R " << fixed << timer << " " << value << endl;

                if (ret.second->isDirty()) { //dirty eviction
                    out << ret.second->getContent().addr << " W " << fixed << timer << " " << ret.second->getContent().value << endl;
                }

            }

            if ( (newType == READ) || (newType == WRITE) ) {
                timer += clkStep;
            }
#endif
#endif
            //send query
#ifdef CUCKOO_RUN
            bool dirty = (newType == EVICT_DIRTY);
            bool affectReadHitRatio = ((newType == READ) || (newType == WRITE) || (newType == FETCH));
            auto result = l2AndCuckoo->query(inp, dirty, affectReadHitRatio, value);

#ifdef COALESCE
            lli qCounter = l2AndCuckoo->getHitCount() + l2AndCuckoo->getMissCount() - 1;
            lli blockAddr = inp >> (blockOffset + 2); // (block offset + byte offset)

            if (!result.hit) {

                if (dirty) { //if missed (not in cache) and it is a write
                    coalesce->writeQuery(blockAddr, qCounter);
                }

                if (result.dirtyEviction) { //dirty eviction
                    coalesce->dirtyEviction(result.evictedContent.addr >> (blockOffset + 2), qCounter);
                }

            }
#endif

#ifdef GENERATE_TIME_TRACE
        if (!result.hit) {
            //miss
            out << inp << " R " << fixed << timer << " " << value << endl;
        }

        if (result.dirtyEviction) {
            out << result.evictedContent.addr << " W " << fixed << timer << " " << result.evictedContent.value << endl;
        }

        if ( (newType == READ) || (newType == WRITE) ) {
            timer += clkStep;
        }
#endif
#endif
            
        }
        
        while (true) {
            cin >> inp; //read next first number
            
            if (cin.eof()) {
                break;
            } else if (!cin.fail()) {
                break;
            } else {
                string dump;
                cin.clear();
                getline(cin, dump);
            }
            
        }
        
    }
    
    string dump;
    ignoreEnd = 0;
    
    while (!cin.eof()) {
        getline(cin, dump);
        ignoreEnd++;
    }
    
    // cin.close();
    
    cout << "(LAST " << ignoreEnd << " LINES WERE IGNORED)" << endl;
    PRINT_MULT('=', LINE_WIDTH);
    cout << endl << endl;

    //print results
#ifdef LRU_RUN
    PRINT_MULT('_', LINE_WIDTH - 7 - 6);
    lli hits, misses, writeBacks;
    hits = lru->getHitCount();
    misses = lru->getMissCount();
    writeBacks = lru->getWriteBackCount();
    cout << "NORMAL\n";
    cout << "MISS RATE: \t\t" << fixed << setprecision(5) << 1.0 * misses / (hits + misses) << "\t";
    cout << fixed << setprecision(3) << 100.0 * misses / (hits + misses) << "%" << endl;
    cout << "HIT RATE: \t\t" << fixed << setprecision(5) << 1.0 * hits / (hits + misses) << "\t";
    cout << fixed << setprecision(3) << 100.0 * hits / (hits + misses) << "%" <<  endl;
    cout << "READ HIT COUNT: \t\t" << fixed << setprecision(5) << lru->readHits << endl;
    cout << "TOTAL READ ACCESS COUNT: \t\t" << lru->totalReadAccess << endl;
    cout << "READ HIT RATIO: \t\t" << fixed << setprecision(5) << 1.0 * lru->readHits / lru->totalReadAccess << endl;
    cout << "WRITE BACK COUNT: \t" << writeBacks << endl;
    cout << "WRITEBACKLOG SIZE: \t" << lru->writeBackLog.size() << endl;
    cout << "NON RECURRING WRITEBACK COUNT: \t" << 1.0 * lru->writeBackLog.size() / writeBacks << endl;
    cout << "TOTAL QUERIES: \t" << hits + misses << endl;
    PRINT_MULT('_', LINE_WIDTH - 7);
    cout << endl;
    time_t end = time(NULL);
    time_t total = end - start;
    cout << "TOTAL TIME: \t\t" << setfill('0') << setw(2) << total / 3600 << ":" << setw(2) << (total % 3600) / 60 << ":" << setw(2) << (total % 60) << endl;
    PRINT_MULT('_', LINE_WIDTH - 7);
    cout << endl;
#endif

    //print results
#ifdef CUCKOO_RUN
    cout << " ";
    PRINT_MULT('-', 25);
    cout << setfill(' ') << endl;
    cout << "| Cuckoo Set Count: " << right << setw(5) << l2AndCuckoo->componentCuckoo.rowCount << " |\n";
    cout << "| Cache Set Count:  " << right << setw(5) << l2AndCuckoo->componentNormal.rowCount << " |\n";
    cout << "| Cuckoo Way Count: " << right << setw(5) << l2AndCuckoo->componentCuckoo.associativity << " |\n";
    cout << "| Cache Way Count:  " << right << setw(5) << l2AndCuckoo->componentNormal.associativity << " |\n";
    cout << "| Threshold:        " << right << setw(5) << threshold << " |\n";
    cout << " ";
    PRINT_MULT('-', 25);
    cout << endl;
    postReport();
    cout << endl << endl;
#endif

    //print the results
#ifdef COALESCE
    cout << "COALESCE\n";

    for (int i = 0; i < coalesce->lt10s.size(); i++) {
        cout << "ABS_LT(10^" << i + 1 << "): " << coalesce->lt10s.at(i) << endl;
    }

    for (int i = 0; i < coalesce->lt2s.size(); i++) {
        cout << "ABS_LT(2^" << i + 1 << "): " << coalesce->lt2s.at(i) << endl;
    }

    cout << "ABS_TOTAL: " << coalesce->total << endl;

    for (int i = 0; i < coalesce->lt10s.size(); i++) {
        cout << "REL_LT(10^" << i + 1 << "): " << 1.0 * coalesce->lt10s.at(i) / coalesce->total << endl;
    }

    for (int i = 0; i < coalesce->lt2s.size(); i++) {
        cout << "REL_LT(2^" << i + 1 << "): " << 1.0 * coalesce->lt2s.at(i) / coalesce->total << endl;
    }

#endif

#ifdef COUNT_EXACT2WBS
    cout << "Count Exact 2 Write-Backs\n";
    cout << "map size: " << countExact2WBs.size() << endl;

    lli counter = 0;

    for (auto it = countExact2WBs.begin(); it != countExact2WBs.end(); it++) {

        if (it->second == 2) {
            counter++;
        }

    }

    cout << "blocks with exactly 2 writebacks out of LLC: " << counter << endl;
    cout << "resulting in a percentage of " << 100.0 * counter / countExact2WBs.size() << endl;
#endif
    
    return 0;
}

void preReport() {
    PRINT_VERSION;
    PRINT_MULT('=', LINE_WIDTH - 9);
    cout << "CONSTANTS";
    cout << endl;
    cout << "Cache Size = \t\t" << CACHE_SIZE << " (KiB)\n";
    PRINT_MULT('_', LINE_WIDTH);
    cout << endl;
    cout << "Associativity = \t" << ASSOCIATIVITY << endl;
    PRINT_MULT('_', LINE_WIDTH);
    cout << endl;
    cout << "Block Size = \t\t" << BLOCK_SIZE << " (B)\n";
    PRINT_MULT('=', LINE_WIDTH);
    cout << endl << endl;
    PRINT_MULT('=', LINE_WIDTH - 5);
    cout << "INPUT";
    cout << endl;
    size_t search = fileName.find_last_of('/');
    
    if (search == string::npos) {
        search = 0; //no slashes found; just print the whole thing!
    } else {
        search++;
    }
    cout << fileName.substr(search) << endl;
}

void postReport() {
    lli hits, misses, writeBacks;
    hits = l2AndCuckoo->getHitCount();
    misses = l2AndCuckoo->getMissCount();
    writeBacks = l2AndCuckoo->getWriteBackCount();
    
    PRINT_MULT('=', LINE_WIDTH - 7);
    cout << "RESULTS\n";
    PRINT_MULT('_', LINE_WIDTH - 7 - 8);
    cout << "COMPLETE\n";
    cout << "MISS RATE: \t\t" << fixed << setprecision(5) << 1.0 * misses / (hits + misses) << "\t";
    cout << fixed << setprecision(3) << 100.0 * misses / (hits + misses) << "%" << endl;
    cout << "HIT RATE: \t\t" << fixed << setprecision(5) << 1.0 * hits / (hits + misses) << "\t";
    cout << fixed << setprecision(3) << 100.0 * hits / (hits + misses) << "%" <<  endl;
    cout << "READ HIT COUNT: \t\t" << fixed << setprecision(5) << l2AndCuckoo->readHits << endl;
    cout << "TOTAL READ ACCESS COUNT: \t\t" << l2AndCuckoo->totalReadAccess << endl;
    cout << "READ HIT RATIO: \t\t" << fixed << setprecision(5) << 1.0 * l2AndCuckoo->readHits / l2AndCuckoo->totalReadAccess << endl;
    cout << "WRITE BACK COUNT: \t" << writeBacks << endl;
    cout << "TOTAL QUERIES: \t" << hits + misses << endl;
    PRINT_MULT('_', LINE_WIDTH - 7);
    cout << endl;
    
    PRINT_MULT('_', LINE_WIDTH - 7 - 6);
    hits = l2AndCuckoo->getComponentNormal().getHitCount();
    misses = l2AndCuckoo->getComponentNormal().getMissCount();
    writeBacks = l2AndCuckoo->getComponentNormal().getWriteBackCount();
    cout << "NORMAL\n";
    cout << "MISS RATE: \t\t" << fixed << setprecision(5) << 1.0 * misses / (hits + misses) << "\t";
    cout << fixed << setprecision(3) << 100.0 * misses / (hits + misses) << "%" << endl;
    cout << "HIT RATE: \t\t" << fixed << setprecision(5) << 1.0 * hits / (hits + misses) << "\t";
    cout << fixed << setprecision(3) << 100.0 * hits / (hits + misses) << "%" <<  endl;
    cout << "WRITE BACK COUNT: \t" << writeBacks << endl;
    cout << "WRITEBACKLOG SIZE: \t" << l2AndCuckoo->getComponentNormal().writeBackLog.size() << endl;
    cout << "NON RECURRING WRITEBACK COUNT: \t" << 1.0 * l2AndCuckoo->getComponentNormal().writeBackLog.size() / writeBacks << endl;
    cout << "TOTAL QUERIES: \t" << hits + misses << endl;
    PRINT_MULT('_', LINE_WIDTH - 7);
    cout << endl;
    
    PRINT_MULT('_', LINE_WIDTH - 7 - 6);
    hits = l2AndCuckoo->getComponentCuckoo().getHitCount();
    misses = l2AndCuckoo->getComponentCuckoo().getMissCount();
    writeBacks = l2AndCuckoo->getComponentCuckoo().getWriteBackCount();
    cout << "CUCKOO\n";
    cout << "MISS RATE: \t\t" << fixed << setprecision(5) << 1.0 * misses / (hits + misses) << "\t";
    cout << fixed << setprecision(3) << 100.0 * misses / (hits + misses) << "%" << endl;
    cout << "HIT RATE: \t\t" << fixed << setprecision(5) << 1.0 * hits / (hits + misses) << "\t";
    cout << fixed << setprecision(3) << 100.0 * hits / (hits + misses) << "%" <<  endl;
    cout << "WRITE BACK COUNT: \t" << writeBacks << endl;
    cout << "DISPLACEMENT AVERAGE PER INSERTION VALUE: \t" << l2AndCuckoo->getComponentCuckoo().dispAvgPerIns.value << endl;
    cout << "DISPLACEMENT AVERAGE PER INSERTION COUNTER: \t" << l2AndCuckoo->getComponentCuckoo().dispAvgPerIns.counter << endl;
    cout << "DISPLACEMENT AVERAGE PER INSERTION: \t" << 1.0 * l2AndCuckoo->getComponentCuckoo().dispAvgPerIns.value / l2AndCuckoo->getComponentCuckoo().dispAvgPerIns.counter << endl;
    cout << "DISPLACEMENT AVERAGE PER ACCESS VALUE: \t" << l2AndCuckoo->getComponentCuckoo().dispAvgPerAcc.value << endl;
    cout << "DISPLACEMENT AVERAGE PER ACCESS COUNTER: \t" << l2AndCuckoo->getComponentCuckoo().dispAvgPerAcc.counter << endl;
    cout << "DISPLACEMENT AVERAGE PER ACCESS: \t" << 1.0 * l2AndCuckoo->getComponentCuckoo().dispAvgPerAcc.value / l2AndCuckoo->getComponentCuckoo().dispAvgPerAcc.counter << endl;
    cout << "TOTAL QUERIES: \t" << hits + misses << endl;
    PRINT_MULT('_', LINE_WIDTH - 7);
    cout << endl;
    
    time_t end = time(NULL);
    time_t total = end - start;
    cout << "TOTAL TIME: \t\t" << setfill('0') << setw(2) << total / 3600 << ":" << setw(2) << (total % 3600) / 60 << ":" << setw(2) << (total % 60) << endl;
    PRINT_MULT('=', LINE_WIDTH);
    cout << endl;
}

void calculateRange() {
    lli ignoreBegin = 0;
    string raw;
    getline(cin, raw);
    
    while (isdigit(raw.at(0)) == 0) { //while first character is not a digit
        ignoreBegin++;
        getline(cin, raw); //skip the line
    }
    
    streampos ans = cin.tellg();
    ans -= raw.size();
    ans -= 1; //the newline character which is not considered in raw.size()
    cin.seekg(ans); //current line should be read formatted!
    cout << "(FIRST " << ignoreBegin << " LINES WERE IGNORED)" << endl;
}

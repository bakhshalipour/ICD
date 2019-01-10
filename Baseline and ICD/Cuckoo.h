/*
The source code of "Reducing Writebacks Through In-Cache Displacement" paper, which is accepted in ACM TODAES 2019.

In this repo, we provide the source codes that are used in our ACM TODAES 2019 paper. This includes the implementation of our proposal, named ICD, as well as the implementation of competitor methods like WADE and HAP. We also provide the source code which acts as an interface between our simulator and Simics. This so-called Simics Interface extends the interface of Flexus simulator with Simics and gets the value of memory accesses, into the bargain.

Please cite the following paper when using the provided source codes:

M. Bakhshalipour, A. Faraji, A. Vakil-Ghahani, F. Samandi, P. Lotfi-Kamran, and H. Sarbazi-Azad, "Reducing Writebacks Through In-Cache Displacement," in ACM Transactions on Design Automation of Electronic Systems (TODAES), 2019.
*/

#ifndef Cuckoo_h
#define Cuckoo_h

#include "CuckooWay.h"
#include <vector>

using namespace std;

class Cuckoo {
private:
    vector<CuckooWay> content;
    int blockOffset;
    unsigned int fIndex;
    lli writeBackCount;
    unsigned int threshold;
    lli hits;
    lli misses;
    
public:
    int rowCount;
    int associativity;
    
    struct {
        lli counter;
        lli value;
    } dispAvgPerIns, dispAvgPerAcc;
    
    /**
     * Constrcutor
     */
    Cuckoo(int initRowCount, int wayCount, int initBlockOffset, vector<lli (*)(lli)> initF, unsigned int initThreshold);
    
    /**
     * Destructor; nothing to be done
     */
    ~Cuckoo();
    
    /**
     * @return <hit?, former data>
     * @return <true, former data> / <false, nullptr>
     */
    pair<bool, const CuckooBlock*> remove(lli addr);
    
    /**
     * Insert new element with counter 0; and rotates data until reaching an empty place or reaching counter threshold
     * @return <dirty eviction?, former data> --> ACTIVE
     * @return <true, nullptr> / <false, nullptr> --> DEACTIVED
     * @return <true, former data> / <false, nullptr> --> RESULT; DELETEing former data after using it is caller's responsibility
     ************************************************************************************************************************************
     */
    pair<bool, const CuckooBlock*> insert(lli addr, string value);
    
    /**
     * @return writeBackCount
     */
    lli getWriteBackCount() const;
    
    /**
     * @return hitCount
     */
    lli getHitCount() const;
    
    /**
     * @return missCount
     */
    lli getMissCount() const;
};

#endif /* Cuckoo_h */

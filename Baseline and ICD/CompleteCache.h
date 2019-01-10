/*
The source code of "Reducing Writebacks Through In-Cache Displacement" paper, which is accepted in ACM TODAES 2019.

In this repo, we provide the source codes that are used in our ACM TODAES 2019 paper. This includes the implementation of our proposal, named ICD, as well as the implementation of competitor methods like WADE and HAP. We also provide the source code which acts as an interface between our simulator and Simics. This so-called Simics Interface extends the interface of Flexus simulator with Simics and gets the value of memory accesses, into the bargain.

Please cite the following paper when using the provided source codes:

M. Bakhshalipour, A. Faraji, A. Vakil-Ghahani, F. Samandi, P. Lotfi-Kamran, and H. Sarbazi-Azad, "Reducing Writebacks Through In-Cache Displacement," in ACM Transactions on Design Automation of Electronic Systems (TODAES), 2019.
*/

#ifndef CompleteCache_h
#define CompleteCache_h

#include "Cuckoo.h"
#include "Cache.h"

struct QueryRet {
    bool hit;
    bool dirtyEviction;
    BlockContent evictedContent;
};

class CompleteCache {
public:
    vector<lli (*)(lli)> fVec;
    int associativity;
    int blockOffset;
    unsigned int threshold;
    static int cuckooRowCount;
    static int cuckooRowCount_log2;
    static int tileCount_log2;
    static int coherenceUnit_log2;
    static int cuckooWayCount;
    lli hits;
    lli misses;
    lli writeBackCount;
    lli readHits;
    lli totalReadAccess;
    /* former hash function (supports at most 4 ways)
    static inline lli H(lli key) {
        return ((((cuckooRowCount >> 1) & key) ^ (key << (cuckooRowCount_log2 - 1))) + (key >> 1)) % cuckooRowCount;
    }
    
    static inline lli H_inv(lli key) {
        return (((key >> (cuckooRowCount_log2 - 1)) ^ (((cuckooRowCount >> 2) & key) >> (cuckooRowCount_log2 - 2))) + (key << 1)) % cuckooRowCount;
    }
    
    static lli skew(lli addr, int way) {
        lli blockAddr;
        blockAddr = addr >> (tileCount_log2 + coherenceUnit_log2);
        
        unsigned int f_index;
        unsigned int A1, A2, A2_index;
        
        A1 = blockAddr % cuckooRowCount;
        
        A2_index = (way / cuckooWayCount) + 1;
        A2 = (blockAddr >> (cuckooRowCount_log2 * A2_index)) % cuckooRowCount;
        
        f_index = way % cuckooWayCount;
        
        if (f_index == 0)
            return ((H(A1) ^ H_inv(A2) ^ A2) % cuckooRowCount);
        else if (f_index == 1)
            return ((H(A1) ^ H_inv(A2) ^ A1) % cuckooRowCount);
        else if (f_index == 2)
            return ((H(A2) ^ H_inv(A1) ^ A2) % cuckooRowCount);
        else
            return ((H(A2) ^ H_inv(A1) ^ A1) % cuckooRowCount);
    }
    */

    //new hash function (supports more than 4 ways)
    static inline unsigned int skew(int64_t addr, unsigned int way) {
        uint64_t blockAddr;

        blockAddr = addr >> 6;

        blockAddr+=way*144;

        blockAddr += (blockAddr << 12);
        blockAddr ^= (blockAddr >> 22);
        blockAddr += (blockAddr << 4);
        blockAddr ^= (blockAddr >> 9);
        blockAddr += (blockAddr << 10);
        blockAddr ^= (blockAddr >> 2);
        blockAddr += (blockAddr << 7);
        blockAddr ^= (blockAddr >> 12);

        return (blockAddr % cuckooRowCount);
    }
    
    static inline lli f0(lli addr) {
        return skew(addr, 0);
    }
    
    static inline lli f1(lli addr) {
        return skew(addr, 1);
    }
    
    static inline lli f2(lli addr) {
        return skew(addr, 2);
    }
    
    static inline lli f3(lli addr) {
        return skew(addr, 3);
    }

    static inline lli f4(lli addr) {
        return skew(addr, 4);
    }

    static inline lli f5(lli addr) {
        return skew(addr, 5);
    }

    static inline lli f6(lli addr) {
        return skew(addr, 6);
    }

    static inline lli f7(lli addr) {
        return skew(addr, 7);
    }

    static inline lli f8(lli addr) {
        return skew(addr, 8);
    }

    static inline lli f9(lli addr) {
        return skew(addr, 9);
    }

    static inline lli f10(lli addr) {
        return skew(addr, 10);
    }

    static inline lli f11(lli addr) {
        return skew(addr, 11);
    }

    static inline lli f12(lli addr) {
        return skew(addr, 12);
    }

    static inline lli f13(lli addr) {
        return skew(addr, 13);
    }

    static inline lli f14(lli addr) {
        return skew(addr, 14);
    }

    static inline lli f15(lli addr) {
        return skew(addr, 15);
    }

public:
    Cuckoo componentCuckoo;
    Cache componentNormal;
    /**
     * Constructor
     */
    CompleteCache(int initRowCount, int initAssociativity, int cuckooSetCount, int cuckooSetCount_log2, int initCuckooWayCount, int initBlockOffset, unsigned int initThreshold, int initIndexSize, int initRowCount_log2, int initTileCount_log2, int initCoherenceUnit_log2);
    
    /**
     * Destructor; nothing to be done
     */
    ~CompleteCache();
    
    /**
     * Sends a query containing an address and indicator of L1 Dirty Eviction
     * @return <dirty eviction?, former data>
     * @return <true, former data> / <false, nullptr> //DELETEing former data is caller's responsibility
     */
    QueryRet query(lli addr, bool l1EvictDirty, bool affectReadHitRatio, string value);
    
    /**
     * @return hits
     */
    lli getHitCount() const;
    
    /**
     * @return misses
     */
    lli getMissCount() const;
    
    /*
     * @return writeBackCount
     */
    lli getWriteBackCount() const;
    
    /*
     * @return componentCuckoo
     */
    const Cuckoo& getComponentCuckoo() const;
    
    /*
     * @return componentNormal
     */
    const Cache& getComponentNormal() const;

};

#endif /* CompleteCache_h */

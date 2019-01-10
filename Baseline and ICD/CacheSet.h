/*
The source code of "Reducing Writebacks Through In-Cache Displacement" paper, which is accepted in ACM TODAES 2019.

In this repo, we provide the source codes that are used in our ACM TODAES 2019 paper. This includes the implementation of our proposal, named ICD, as well as the implementation of competitor methods like WADE and HAP. We also provide the source code which acts as an interface between our simulator and Simics. This so-called Simics Interface extends the interface of Flexus simulator with Simics and gets the value of memory accesses, into the bargain.

Please cite the following paper when using the provided source codes:

M. Bakhshalipour, A. Faraji, A. Vakil-Ghahani, F. Samandi, P. Lotfi-Kamran, and H. Sarbazi-Azad, "Reducing Writebacks Through In-Cache Displacement," in ACM Transactions on Design Automation of Electronic Systems (TODAES), 2019.
*/

#ifndef CacheSet_h
#define CacheSet_h

#include <vector>
#include <iostream>
#include "CacheBlock.h"

using namespace std;

class CacheSet {
private:
    vector<CacheBlock> blockArr;
    vector<int> usageHistory;
    vector<int> empty;
    const int associativity;
    
    /**
     * Insert a new block
     * @return Former data
     */
    const CacheBlock* insert(lli newAddr, bool dirty, string value);
    
public:
    
    /**
     * Constructor
     * @param initBlockOffset Number of rightmost bits ignored for each block
     */
    CacheSet(int initAssociativity, int initBlockOffset);
    
    /**
     * Destructor; nothing to be done
     */
    ~CacheSet();
    
    /**
     * @return pair<hit?, former data>
     */
    pair<bool, const CacheBlock*> request(lli queryAddr, bool queryDirty, string value);
};


#endif /* CacheSet_h */

/*
The source code of "Reducing Writebacks Through In-Cache Displacement" paper, which is accepted in ACM TODAES 2019.

In this repo, we provide the source codes that are used in our ACM TODAES 2019 paper. This includes the implementation of our proposal, named ICD, as well as the implementation of competitor methods like WADE and HAP. We also provide the source code which acts as an interface between our simulator and Simics. This so-called Simics Interface extends the interface of Flexus simulator with Simics and gets the value of memory accesses, into the bargain.

Please cite the following paper when using the provided source codes:

M. Bakhshalipour, A. Faraji, A. Vakil-Ghahani, F. Samandi, P. Lotfi-Kamran, and H. Sarbazi-Azad, "Reducing Writebacks Through In-Cache Displacement," in ACM Transactions on Design Automation of Electronic Systems (TODAES), 2019.
*/

#ifndef CuckooBlock_h
#define CuckooBlock_h

#include "Block.h"

class CuckooBlock: public Block {
friend class Cuckoo;
protected:
    int counter;
    CuckooBlock* former;

public:
    /**
     * Constructor
     * @param initBlockOffset Number of rightmost bits that should be ignored because of blocking
     */
    CuckooBlock(int initBlockOffset);
    
    /**
     * Copy constructor
     * @param src Source
     */
    CuckooBlock(const CuckooBlock& src);
    
    /**
     * Destructor; delete former in case
     */
    virtual ~CuckooBlock();
    
    /**
     * @return Former data
     */
    const CuckooBlock* set(lli newAddr, int newCounter, std::string value);
    
    /**
     * copy everything to this object
     */
    CuckooBlock operator=(const CuckooBlock& right);
};


#endif /* CuckooBlock_h */

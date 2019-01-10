/*
The source code of "Reducing Writebacks Through In-Cache Displacement" paper, which is accepted in ACM TODAES 2019.

In this repo, we provide the source codes that are used in our ACM TODAES 2019 paper. This includes the implementation of our proposal, named ICD, as well as the implementation of competitor methods like WADE and HAP. We also provide the source code which acts as an interface between our simulator and Simics. This so-called Simics Interface extends the interface of Flexus simulator with Simics and gets the value of memory accesses, into the bargain.

Please cite the following paper when using the provided source codes:

M. Bakhshalipour, A. Faraji, A. Vakil-Ghahani, F. Samandi, P. Lotfi-Kamran, and H. Sarbazi-Azad, "Reducing Writebacks Through In-Cache Displacement," in ACM Transactions on Design Automation of Electronic Systems (TODAES), 2019.
*/

#ifndef CacheBlock_h
#define CacheBlock_h

#include "Block.h"

class CacheBlock: public Block {
friend class CompleteCache;
protected:
    bool dirty;
    CacheBlock* former;
    
public:
    /**
     * Constructor
     * @param blockOffset Number of rightmost bits that should be ignored
     */
    CacheBlock(int blockOffset);
    
    /**
     * Copy constructor
     * @param src Source
     */
    CacheBlock(const CacheBlock& src);
    
    /**
     * Destructor; delete former in case
     */
    virtual ~CacheBlock();
    
    /**
     * Set isDirty to True
     */
    void setDirty();
    
    /**
     * @return dirty
     */
    bool isDirty() const;
    
    /**
     * @return Former data
     */
    const CacheBlock* set(lli newAddr, bool newDirty, std::string value);

    void setVal(std::string value);
};


#endif /* CacheBlock_h */

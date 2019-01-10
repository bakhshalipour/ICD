/*
The source code of "Reducing Writebacks Through In-Cache Displacement" paper, which is accepted in ACM TODAES 2019.

In this repo, we provide the source codes that are used in our ACM TODAES 2019 paper. This includes the implementation of our proposal, named ICD, as well as the implementation of competitor methods like WADE and HAP. We also provide the source code which acts as an interface between our simulator and Simics. This so-called Simics Interface extends the interface of Flexus simulator with Simics and gets the value of memory accesses, into the bargain.

Please cite the following paper when using the provided source codes:

M. Bakhshalipour, A. Faraji, A. Vakil-Ghahani, F. Samandi, P. Lotfi-Kamran, and H. Sarbazi-Azad, "Reducing Writebacks Through In-Cache Displacement," in ACM Transactions on Design Automation of Electronic Systems (TODAES), 2019.
*/

#ifndef Block_h
#define Block_h

#include <string>

typedef long long unsigned int lli;

struct BlockContent {
    lli addr;
    std::string value;
};

class Block {
friend class Cache;
protected:
    BlockContent content;
    bool isValid;
    int blockOffset;
    int shiftCount;

public:
    /**
     * Constructor
     * @param initBlockOffset Number of rightmost bits that should be ignored
     */
    Block(int initBlockOffset);
    
    /**
     * Copy constructor
     * @param src Source
     */
    Block(const Block& src);
    
    /**
     * Destructor; nothing to be done
     */
    virtual ~Block();
    
    /**
     * @param right The address of the query
     * @return True if the block consists the byte located at arg[0] and false otherwise
     */
    bool operator==(lli right);
    
    /**
     * @return Negated result of ==
     */
    bool operator!=(lli right);
    
    /**
     * copy everything to this object
     */
    Block operator=(const Block& right);
    
    /**
     * Invalidate the block
     */
    void evict();

    BlockContent getContent() const;
};


#endif /* Block_h */

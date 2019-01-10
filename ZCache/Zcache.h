/*
The source code of "Reducing Writebacks Through In-Cache Displacement" paper, which is accepted in ACM TODAES 2019.

In this repo, we provide the source codes that are used in our ACM TODAES 2019 paper. This includes the implementation of our proposal, named ICD, as well as the implementation of competitor methods like WADE and HAP. We also provide the source code which acts as an interface between our simulator and Simics. This so-called Simics Interface extends the interface of Flexus simulator with Simics and gets the value of memory accesses, into the bargain.

Please cite the following paper when using the provided source codes:

M. Bakhshalipour, A. Faraji, A. Vakil-Ghahani, F. Samandi, P. Lotfi-Kamran, and H. Sarbazi-Azad, "Reducing Writebacks Through In-Cache Displacement," in ACM Transactions on Design Automation of Electronic Systems (TODAES), 2019.
*/

#ifndef Zcache_h
#define Zcache_h

#include "Def.h"
#include <set>
#include <map>

using namespace std;

struct Block {
    lli blockAddr;
    lli lastAccess;
    bool isValid;
    bool dirty;

    Block(lli lastAccess);
};

struct Node {
    int way;
    int index;
    Node *parent;

    Node(int index, int way, Node *parent);
};

struct NodeCMP {
    
    bool operator()(const Node &lhs, const Node &rhs) const {
        
        if (lhs.way != rhs.way) {
            return lhs.way < rhs.way;
        } else if (lhs.index != rhs.index) {
            return lhs.index < rhs.index;
        } else if ( (lhs.parent != NULL) && (rhs.parent != NULL) ) {
                return operator()(*lhs.parent, *rhs.parent);
        } else if (lhs.parent == NULL) {
            return true;
        } else {
            return false;
        }
        
    }
    
};

struct Element {
    lli lastAccess;
    Node *candidate;
    int lvl;
};

struct ElementCMP {

    bool operator()(const Element &lhs, const Element &rhs) const {
        
        if (lhs.lastAccess != rhs.lastAccess) {
            return lhs.lastAccess < rhs.lastAccess;
        } else if (lhs.lvl != rhs.lvl) {
            return lhs.lvl < rhs.lvl;
        } else {
            NodeCMP comparator;
            return comparator.operator()(*lhs.candidate, *rhs.candidate);
        }
        
    }

};

struct Zcache {
    vector<vector<Block>> content;
    lli hitCount;
    lli missCount;
    lli readCount;
    lli readHitCount;
    lli writebackCount;
    lli queryCounter;
    lli tabas_globalCounter;
    BitLen bitLen;
    
    int setCount;
    int associativity;
    
    // set<Element, ElementCMP> lateAccessed;
    map<lli, Node *> lateAccessed;
    vector<Node *> garbageCan;
    
    Zcache(int size, int associativity);
    void lookup(lli blockAddr, bool isDirty, bool affectRead);
    void track(lli addr, int lvl, int way, Node *parent);
    long H(long key);
    long H_inv(long key);
    long skew(lli blockAddr, long way);
    // unsigned int skew(int64_t addr, unsigned int way);
};

#endif /* Zcache_h */
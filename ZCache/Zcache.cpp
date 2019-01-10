/*
The source code of "Reducing Writebacks Through In-Cache Displacement" paper, which is accepted in ACM TODAES 2019.

In this repo, we provide the source codes that are used in our ACM TODAES 2019 paper. This includes the implementation of our proposal, named ICD, as well as the implementation of competitor methods like WADE and HAP. We also provide the source code which acts as an interface between our simulator and Simics. This so-called Simics Interface extends the interface of Flexus simulator with Simics and gets the value of memory accesses, into the bargain.

Please cite the following paper when using the provided source codes:

M. Bakhshalipour, A. Faraji, A. Vakil-Ghahani, F. Samandi, P. Lotfi-Kamran, and H. Sarbazi-Azad, "Reducing Writebacks Through In-Cache Displacement," in ACM Transactions on Design Automation of Electronic Systems (TODAES), 2019.
*/

#include "Zcache.h"
#include <climits>
#include <cmath>
#include <iostream>

Block::Block(lli lastAccess) {
    this->blockAddr = 0;
    this->lastAccess = lastAccess;
    this->isValid = false;
    this->dirty = false;
}

Node::Node(int index, int way, Node *parent) {
    this->index = index;
    this->parent = parent;
    this->way = way;
}

Zcache::Zcache(int size, int associativity) {
    this->associativity = associativity;
    this->setCount = (size * 1024) / associativity / 64;
    
    for (int i = 0; i < associativity; i++) {
        vector<Block> copyVec;
        this->content.push_back(copyVec);
        
        for (int j = 0; j < setCount; j++) {
            Block copyBlock(LONG_MIN + this->setCount * i + j);
            this->content.at(i).push_back(copyBlock);
        }
        
    }
    
    this->hitCount = 0;
    this->missCount = 0;
    this->readCount = 0;
    this->readHitCount = 0;
    this->writebackCount = 0;
    this->queryCounter = 0;
    this->tabas_globalCounter = 0;
    
    this->bitLen.block = log2(BLOCK_SIZE);
    this->bitLen.sets = log2(this->setCount);
}

void Zcache::lookup(lli addr, bool isDirty, bool affectRead) {
    Node *victimNode, *victimParentNode;
    Block *victimBlock, *victimParentBlock, *hitCandidate;
    int index;
    lli blockAddr = addr >> bitLen.block;
    
    this->queryCounter++;
    
    if ((this->queryCounter % K_CONST) == 0) {
        this->tabas_globalCounter++;
    }
    
    if (affectRead) {
        this->readCount++;
    }
    
    for (int i = 0; i < this->associativity; i++) {
        index = (int) skew(blockAddr, i);
        hitCandidate = &this->content.at(i).at(index);
        
        if ( (hitCandidate->blockAddr == blockAddr) && (hitCandidate->isValid) ) {
            hitCandidate->lastAccess = this->tabas_globalCounter;
            
            if (isDirty) {
                hitCandidate->dirty = isDirty;
            }
            
            this->hitCount++;
            
            if (affectRead) {
                this->readHitCount++;
            }
            return;
        }
            
    }
    
    track(blockAddr, 0, -1, NULL);
    
    victimNode = lateAccessed.begin()->second;
    
    while (victimNode->parent != NULL) {
        victimParentNode = victimNode->parent;
        
        victimBlock = &this->content.at(victimNode->way).at(victimNode->index);
        victimParentBlock = &this->content.at(victimParentNode->way).at(victimParentNode->index);
        
        victimBlock->blockAddr = victimParentBlock->blockAddr;
        victimBlock->lastAccess = victimParentBlock->lastAccess;
        victimBlock->isValid = victimParentBlock->isValid;
        victimBlock->dirty = victimParentBlock->dirty;
        
        victimNode = victimParentNode;
    }
    
    victimBlock = &this->content.at(victimNode->way).at(victimNode->index);
    
    if (victimBlock->dirty) {
        this->writebackCount++;
    }
    
    victimBlock->blockAddr = blockAddr;
    victimBlock->lastAccess = this->tabas_globalCounter;
    victimBlock->isValid = true;
    victimBlock->dirty = isDirty;
    
    for (auto it = garbageCan.begin(); it != garbageCan.end(); it++) {
        delete (*it);
    }
    
    lateAccessed.clear();
    garbageCan.clear();
    
    this->missCount++;
}

void Zcache::track(lli addr, int lvl, int way, Node *parent) {
    Block *victimCandidate;
    int row;
    Node *tmp;
    
    if (lvl == DEPTH) {
        return;
    }
    
    for (int w = 0; w < this->associativity; w++) {
        
        if (w == way) {
            continue;
        }
        
        row = (int) skew(addr, w);
        victimCandidate = &this->content.at(w).at(row);
        
        auto search = lateAccessed.find(victimCandidate->lastAccess);
       
        if (search == lateAccessed.end()) {
            // not already in map
            tmp = new Node(row, w, parent);
            garbageCan.push_back(tmp);
            lateAccessed[victimCandidate->lastAccess] = tmp;
        } else {
            tmp = search->second;
        }
        
        track(victimCandidate->blockAddr, lvl + 1, w, tmp);
    }
    
}

long Zcache::H(long key) {
    return ((((this->setCount >> 1) & key) ^ (key << (this->bitLen.sets - 1))) + (key >> 1)) % this->setCount;
}

long Zcache::H_inv(long key) {
    return (((key >> (this->bitLen.sets - 1)) ^ (((this->setCount >> 2) & key) >> (this->bitLen.sets - 2))) + (key << 1)) % this->setCount;
}

long Zcache::skew(long blockAddr, long way) {
    long f_index;
    long A1, A2, A2_index;

    A1 = blockAddr % this->setCount;

    A2_index = (way / 4) + 1;
    A2 = (blockAddr >> (this->bitLen.sets * A2_index)) % this->setCount;

    f_index = way % 4;

    if (f_index == 0)
        return ((H(A1) ^ H_inv(A2) ^ A2) % this->setCount);
    else if (f_index == 1)
        return ((H(A1) ^ H_inv(A2) ^ A1) % this->setCount);
    else if (f_index == 2)
        return ((H(A2) ^ H_inv(A1) ^ A2) % this->setCount);
    else
        return ((H(A2) ^ H_inv(A1) ^ A1) % this->setCount);

}

// long Zcache::skew(lli addr, long way) {
//     uint64_t blockAddr;
//
//     blockAddr = addr >> 6;
//
//     blockAddr+=way*144;
//
//     blockAddr += (blockAddr << 12);
//     blockAddr ^= (blockAddr >> 22);
//     blockAddr += (blockAddr << 4);
//     blockAddr ^= (blockAddr >> 9);
//     blockAddr += (blockAddr << 10);
//     blockAddr ^= (blockAddr >> 2);
//     blockAddr += (blockAddr << 7);
//     blockAddr ^= (blockAddr >> 12);
//
//     return (blockAddr % this->setCount);
// }
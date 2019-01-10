/*
The source code of "Reducing Writebacks Through In-Cache Displacement" paper, which is accepted in ACM TODAES 2019.

In this repo, we provide the source codes that are used in our ACM TODAES 2019 paper. This includes the implementation of our proposal, named ICD, as well as the implementation of competitor methods like WADE and HAP. We also provide the source code which acts as an interface between our simulator and Simics. This so-called Simics Interface extends the interface of Flexus simulator with Simics and gets the value of memory accesses, into the bargain.

Please cite the following paper when using the provided source codes:

M. Bakhshalipour, A. Faraji, A. Vakil-Ghahani, F. Samandi, P. Lotfi-Kamran, and H. Sarbazi-Azad, "Reducing Writebacks Through In-Cache Displacement," in ACM Transactions on Design Automation of Electronic Systems (TODAES), 2019.
*/

#include "Block.h"

Block::Block(int initBlockOffset): isValid(false), blockOffset(initBlockOffset), shiftCount(initBlockOffset + 2) {
    //shiftCount = blockOffset + byteOffset
    content.addr = 0;
}

Block::Block(const Block& src): isValid(src.isValid), blockOffset(src.blockOffset), shiftCount(src.shiftCount) {
    content.addr = src.content.addr;
    content.value = src.content.value;
}

bool Block::operator==(lli right) {
    
    if (isValid == false) {
        return false;
    }
    
    right = right >> shiftCount;
    lli checkAddr = content.addr >> shiftCount;
    
    if (right == checkAddr) {
        return true;
    } else {
        return false;
    }
    
}

bool Block::operator!=(lli right) {
    return !operator==(right);
}

void Block::evict() {
    isValid = false;
}

Block::~Block() {
    
}

Block Block::operator=(const Block &right) {
    content.addr = right.content.addr;
    content.value = right.content.value;
    isValid = right.isValid;
    blockOffset = right.blockOffset;
    shiftCount = right.shiftCount;
    return (*this);
}

BlockContent Block::getContent() const {
    return content;
}
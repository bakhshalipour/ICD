/*
The source code of "Reducing Writebacks Through In-Cache Displacement" paper, which is accepted in ACM TODAES 2019.

In this repo, we provide the source codes that are used in our ACM TODAES 2019 paper. This includes the implementation of our proposal, named ICD, as well as the implementation of competitor methods like WADE and HAP. We also provide the source code which acts as an interface between our simulator and Simics. This so-called Simics Interface extends the interface of Flexus simulator with Simics and gets the value of memory accesses, into the bargain.

Please cite the following paper when using the provided source codes:

M. Bakhshalipour, A. Faraji, A. Vakil-Ghahani, F. Samandi, P. Lotfi-Kamran, and H. Sarbazi-Azad, "Reducing Writebacks Through In-Cache Displacement," in ACM Transactions on Design Automation of Electronic Systems (TODAES), 2019.
*/

#include "CSet.h"
#include <cassert>

CSet::CSet(int associativity): associativity(associativity), lNVM(0), isSample(false), type(-1), accessCounter(0), costCounter(0) {
    
}

QRet CSet::query(CBlock newBlock) {
    QRet ret;
    ret.hit = true;
    ret.dirtyEviction = false;
    
    if (isSample) {
        accessCounter++;
    }
    
    //check hit
    int hitIndex = -1;
    
    for (int i = 0; i < content.size(); i++) {
        
        if (content.at(i).tag == newBlock.tag) {
            hitIndex = i;
            break;
        }
        
    }
    
    if (hitIndex != -1) {
        //hit
        content.erase(content.begin() + hitIndex); //remove from former position in queue
        content.push_back(newBlock); //inesrt into MRU position
        return ret;
    }

    //miss
    ret.hit = false;
    assert(content.size() <= associativity);
    
    if (isSample) {
        costCounter += ( (newBlock.isNVM) ? (L_RATIO) : (1) );
    }

    if (content.size() == associativity) {
        //there are no empty places; eviction
    
        if ( (lNVM > threshold->hi) && (newBlock.isNVM) ) {
            //case 1; evict LRU of NVM
            bool victimChosen = false;
            
            do {
                
                for (int i = 0; i < content.size(); i++) {
            
                    if (!content.at(i).isNVM) {
                        continue; //skip DRAM blocks
                    }
            
                    if ( (content.at(i).isDirty) && (content.at(i).isChance) ) {
                        content.at(i).isChance = false;
                        continue; //give second chance
                    } else {
                        //found LRU of NVM
                        ret.dirtyEviction = content.at(i).isDirty;
                        ret.evictedBlock = content.at(i);
                        lNVM--;
                        content.erase(content.begin() + i); //evict
                        victimChosen = true;
                        break;
                    }
            
                }
            
            } while (!victimChosen); //in case all blocks had second chances
        
        } else if ( (lNVM < threshold->lo) && (!newBlock.isNVM) ) {
            //case 2; evict LRU of DRAM
            
            for (int i = 0; i < content.size(); i++) {
                
                if (content.at(i).isNVM) {
                    continue; //skip NVM blocks
                }
                
                ret.dirtyEviction = content.at(i).isDirty;
                ret.evictedBlock = content.at(i);
                content.erase(content.begin() + i);
                break;
            }
            
        } else {
            //case 3; evict LRU
            bool victimChosen = false;
            
            do {
                
                for (int i = 0; i < content.size(); i++) {
            
                    if ( (content.at(i).isNVM) && (content.at(i).isDirty) && (content.at(i).isChance) ) {
                        content.at(i).isChance = false;
                        continue; //give second chance
                    } else {
                        //found LRU
                        ret.dirtyEviction = content.at(i).isDirty;
                        ret.evictedBlock = content.at(i);
                        
                        if (content.at(i).isNVM) {
                            lNVM--;
                        }
                        
                        content.erase(content.begin() + i); //evict
                        victimChosen = true;
                        break;
                    }
            
                }
            
            } while (!victimChosen); //in case all blocks had second chances
            
        }
        
    }
    
    content.push_back(newBlock); //push onto MRU position
    
    if (newBlock.isNVM) {
        lNVM++;
    }
    
    return ret;
}
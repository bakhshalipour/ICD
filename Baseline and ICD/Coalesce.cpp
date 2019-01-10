/*
The source code of "Reducing Writebacks Through In-Cache Displacement" paper, which is accepted in ACM TODAES 2019.

In this repo, we provide the source codes that are used in our ACM TODAES 2019 paper. This includes the implementation of our proposal, named ICD, as well as the implementation of competitor methods like WADE and HAP. We also provide the source code which acts as an interface between our simulator and Simics. This so-called Simics Interface extends the interface of Flexus simulator with Simics and gets the value of memory accesses, into the bargain.

Please cite the following paper when using the provided source codes:

M. Bakhshalipour, A. Faraji, A. Vakil-Ghahani, F. Samandi, P. Lotfi-Kamran, and H. Sarbazi-Azad, "Reducing Writebacks Through In-Cache Displacement," in ACM Transactions on Design Automation of Electronic Systems (TODAES), 2019.
*/

#include "Coalesce.h"

Coalesce::Coalesce(): total(0) {

    for (int i = 0; i < 8; i++) {
        lt10s.push_back(0);
    } //keep track until 10^8

    for (int i = 0; i < 26; i++) {
        lt2s.push_back(0);
    } //keep track until 2^26

}

void Coalesce::writeQuery(lli addr, lli qCounter) {
    auto it = history.find(addr);

    if (it == history.end()) {
        return;
    }

    lli diff = qCounter - it->second;
    total++;
    lli comparand = 10;

    for (int i = 0; i < lt10s.size(); i++) {

        if (diff < comparand) {
            lt10s.at(i)++;
        }

        comparand *= 10;
    }

    comparand = 2;

    for (int i = 0; i < lt2s.size(); i++) {

        if (diff < comparand) {
            lt2s.at(i)++;
        }

        comparand *= 2;
    }

}

void Coalesce::dirtyEviction(lli addr, lli qCounter) {
    history[addr] = qCounter;
}
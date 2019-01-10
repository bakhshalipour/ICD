/*
The source code of "Reducing Writebacks Through In-Cache Displacement" paper, which is accepted in ACM TODAES 2019.

In this repo, we provide the source codes that are used in our ACM TODAES 2019 paper. This includes the implementation of our proposal, named ICD, as well as the implementation of competitor methods like WADE and HAP. We also provide the source code which acts as an interface between our simulator and Simics. This so-called Simics Interface extends the interface of Flexus simulator with Simics and gets the value of memory accesses, into the bargain.

Please cite the following paper when using the provided source codes:

M. Bakhshalipour, A. Faraji, A. Vakil-Ghahani, F. Samandi, P. Lotfi-Kamran, and H. Sarbazi-Azad, "Reducing Writebacks Through In-Cache Displacement," in ACM Transactions on Design Automation of Electronic Systems (TODAES), 2019.
*/

#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <cmath>
#include "Zcache.h"
#include <cassert>

#define VERSION "ZCache v0.9"

#define LINE_WIDTH 48
#define PRINT_VERSION cout << VERSION << endl
#define PRINT_MULT(char, count) (cout << setfill(char) << setw(count) << "")

using namespace std;

time_t start;
lli ignoreEnd;
Zcache *cache;

void calculateRange();
void postReport();
void preReport();

int main(int argc, const char* argv[]) {

    // fileName = argv[1];
    // cin.open(argv[1]);
    //
    // if (!cin.is_open()) {
    //     cout << "Specified File Cannot Be Opened\n";
    //     exit(EXIT_FAILURE);
    // }
    
    preReport();
    calculateRange();
    start = time(NULL);

    //initialize cache object
    PRINT_MULT('-', LINE_WIDTH / 2 - 7);
    cout << " BUCKETED LRU ";
    PRINT_MULT('-', LINE_WIDTH / 2 - 7);
    cout << endl;
    cache = new Zcache(SIZE, ASSOCIATIVITY);

    lli inp;
    //FORMAT: %d   %d   %s   %s   %s

    while (true) {
        cin >> inp; //read next first number

        if (cin.eof()) {
            break;
        } else if (!cin.fail()) {
            break;
        } else {
            string dump;
            cin.clear();
            getline(cin, dump);
        }

    }

    string first, second, value;
    bool ignore;
    lli histo = 0;

    while (!cin.eof()) {
        
        if (cin.fail()) {
            cin.clear();
            assert(false && "bad 'first n lines were ignored'");
            break;
        }
        
        ignore = false;
        cin >> inp; //dump first number & get the address
        cin >> first; //dump first string
        cin >> second; //dump second string
        getline(cin, value);

        switch(first[0]) {
            case 'F':
            case 'R':
            case 'W':
                break;

            case 'E':

                if ( (second[1] != 'w') && (second[1] != 'c') && (second[1] != 'd') ) {
                    assert(false && "bad trace");
                }

                break;

            case 'U':
                ignore = true;
                break;

            default:
                assert(false && "bad trace");
                break;
        }
        
        if (!ignore) {
            //send query
            bool isDirty = ((first[0] == 'E') && (second[1] == 'd'));
            bool affectReadHitRatio = ((first[0] == 'R') || (first[0] == 'W') || (first[0] == 'F'));
            cache->lookup(inp, isDirty, affectReadHitRatio);
            
            if (cache->queryCounter - histo >= LOG_EVERY_QUERY_COUNT) {
                cout << setfill(' ') << "QUERY NO " << setw(10) << cache->queryCounter << " FINISHED! ";
                cout.flush();
                cout << "\n";
                histo = cache->queryCounter;
            }
            
        }
        
        while (true) {
            cin >> inp; //read next first number
            
            if (cin.eof()) {
                break;
            } else if (!cin.fail()) {
                break;
            } else {
                string dump;
                cin.clear();
                getline(cin, dump);
            }
            
        }
        
    }
    
    string dump;
    ignoreEnd = 0;
    
    while (!cin.eof()) {
        getline(cin, dump);
        ignoreEnd++;
    }
    
    // cin.close();
    
    cout << "(LAST " << ignoreEnd << " LINES WERE IGNORED)" << endl;
    PRINT_MULT('=', LINE_WIDTH);
    cout << endl << endl;

    //print results
    postReport();
    return 0;
}

void preReport() {
    PRINT_VERSION;
    PRINT_MULT('=', LINE_WIDTH - 9);
    cout << "CONSTANTS";
    cout << endl;
    cout << "Cache Size = \t\t" << SIZE << " (KiB)\n";
    PRINT_MULT('_', LINE_WIDTH);
    cout << endl;
    cout << "Associativity = \t" << ASSOCIATIVITY << endl;
    PRINT_MULT('_', LINE_WIDTH);
    cout << endl;
    cout << "Block Size = \t\t" << BLOCK_SIZE << " (B)\n";
    PRINT_MULT('_', LINE_WIDTH);
    cout << endl;
    cout << "Depth = \t\t" << DEPTH << endl;
    PRINT_MULT('_', LINE_WIDTH);
    cout << endl;
    cout << "K_const = \t\t" << K_CONST << endl;
    PRINT_MULT('=', LINE_WIDTH);
    cout << endl;
}

void postReport() {
    lli hits, misses, reads, readHits, writebacks;
    hits = cache->hitCount;
    misses = cache->missCount;
    reads = cache->readCount;
    readHits = cache->readHitCount;
    writebacks = cache->writebackCount;
    
    PRINT_MULT('=', LINE_WIDTH - 7);
    cout << "RESULTS\n";
    PRINT_MULT('_', LINE_WIDTH - 7 - 8);
    cout << endl;
    cout << "MISS RATE: \t\t" << fixed << setprecision(5) << 1.0 * misses / (hits + misses) << "\t";
    cout << fixed << setprecision(3) << 100.0 * misses / (hits + misses) << "%" << endl;
    cout << "HIT RATE: \t\t" << fixed << setprecision(5) << 1.0 * hits / (hits + misses) << "\t";
    cout << fixed << setprecision(3) << 100.0 * hits / (hits + misses) << "%" <<  endl;
    cout << "READ HIT COUNT: \t\t" << fixed << setprecision(5) << readHits << endl;
    cout << "TOTAL READ ACCESS COUNT: \t\t" << reads << endl;
    cout << "READ HIT RATIO: \t\t" << fixed << setprecision(5) << 1.0 * readHits / reads << endl;
    cout << "WRITE BACK COUNT: \t" << writebacks << endl;
    cout << "TOTAL QUERIES: \t" << hits + misses << endl;
    PRINT_MULT('_', LINE_WIDTH - 7);
    cout << endl;
    
    time_t end = time(NULL);
    time_t total = end - start;
    cout << "TOTAL TIME: \t\t" << setfill('0') << setw(2) << total / 3600 << ":" << setw(2) << (total % 3600) / 60 << ":" << setw(2) << (total % 60) << endl;
    PRINT_MULT('=', LINE_WIDTH);
    cout << endl;
}

void calculateRange() {
    lli ignoreBegin = 0;
    string raw;
    getline(cin, raw);
    
    while (isdigit(raw.at(0)) == 0) { //while first character is not a digit
        ignoreBegin++;
        getline(cin, raw); //skip the line
    }
    
    streampos ans = cin.tellg();
    ans -= raw.size();
    ans -= 1; //the newline character which is not considered in raw.size()
    cin.seekg(ans); //current line should be read formatted!
    cout << "(FIRST " << ignoreBegin << " LINES WERE IGNORED)" << endl;
}

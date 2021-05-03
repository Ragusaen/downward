#ifndef SYMBOLIC_SYM_UTIL_H
#define SYMBOLIC_SYM_UTIL_H

#include "cuddObj.hh"

#include <math.h>
#include <memory>
#include <iostream>
#include <fstream>
#include <sstream>
#include <utility>
#include <vector>
#include <string>
#include <map>
#include "../utils/timer.h"

#include <math.h>
#include <memory>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <map>
#include "../utils/timer.h"

#include "../algorithms/dynamic_bitset.h"

namespace op_mutex {
struct BDDError {};

void exceptionError(const std::string);

std::shared_ptr<Cudd> init_bdd_manager(int num_vars);

template <class T>
T getData(std::string line, const std::string &separator, const std::string &separator_end) {
    if (!separator.empty()) {
        line.erase(line.begin(), line.begin() + line.find(separator) + 1);
    }

    if (!separator_end.empty()) {
        line.erase(line.begin() + line.find(separator_end), line.end());
    }

    T res;
    std::stringstream ss;
    ss << line;
    if (!(ss >> res)) {
        std::cout << std::endl << "ERROR, could not parse: " << line << std::endl;
        exit(1);
    }
    return res;
}

template <class T>
T getData(std::ifstream &file, const std::string &separator) {
    std::string line;
    getline(file, line);
    T res = getData<T>(line, separator, "");
    return res;
}

template <class T, class FunctionMerge>
void mergeAux(std::vector<T> &elems, FunctionMerge f, int maxTime, int maxSize) {
    std::vector <T> result;
    if (maxSize <= 1 || elems.size() <= 1) {
        return;
    }
    utils::Timer merge_timer;
    //  cout << "Merging " << elems.size() << ", maxSize: " << maxSize << endl;

    //Merge Elements
    std::vector<T>aux;
    while (elems.size() > 1 && (maxTime == 0 || merge_timer() * 1000 < maxTime)) {
        if (elems.size() % 2 == 1) { //Ensure an even number
            int last = elems.size() - 1;
            try{
                T res = f(elems[last - 1], elems[last], maxSize);
                elems[last - 1] = res;
            }catch (BDDError e) {
                result.push_back(elems[last]);
            }
            elems.erase(elems.end() - 1);
        }
        //    cout << "Iteration: " << elems.size() << endl;
        for (size_t i = 1; i < elems.size(); i += 2) {
            try{
                T res = f(elems[i - 1], elems[i], maxSize);
                aux.push_back(res);
            }catch (BDDError e) {
                if (elems[i].nodeCount() < elems[i - 1].nodeCount()) {
                    result.push_back(elems[i - 1]);
                    aux.push_back(elems[i]);
                } else {
                    result.push_back(elems[i]);
                    aux.push_back(elems[i - 1]);
                }
            }
        }
        aux.swap(elems);
        std::vector<T>().swap(aux);
    }
    //  cout << "Finished: " << elems.size() << endl;
    if (!elems.empty()) {
        result.insert(result.end(), elems.begin(), elems.end());
    }
    result.swap(elems);
    //Add all the elements remaining in aux
    if (!aux.empty()) {
        elems.insert(elems.end(), aux.begin(), aux.end());
    }
    /*for(int i = 0; i < aux.size(); i++){
      elems.push_back(aux[i]);
      }*/

    //  cout << "Merged to " << elems.size() << ". Took "<< merge_timer << " seconds" << endl;
}

/*
 * Method that merges some elements,
 * Relays on several methods: T, int T.size() and bool T.merge(T, maxSize)
 */
template <class T, class FunctionMerge>
void merge(Cudd &manager,std::vector<T> &elems, FunctionMerge f, int maxTime, int maxSize) {
    manager.SetTimeLimit(maxTime);
    manager.ResetStartTime();
    mergeAux(elems, f, maxTime, maxSize);
    manager.UnsetTimeLimit();
}

struct BitBDD {
    BDD bdd;
    dynamic_bitset::DynamicBitset<> used_vars;

    BitBDD(const BitBDD& bit_bdd) = default;
    BitBDD(BitBDD& bit_bdd) = default;
    explicit BitBDD(std::size_t num_vars) : used_vars(num_vars) {
    };

    BitBDD(BDD bdd, dynamic_bitset::DynamicBitset<> used_vars)
        : used_vars(std::move(used_vars)) {
        this->bdd = bdd;
    }

    BitBDD operator*(const BitBDD& other) {
        return BitBDD(bdd * other.bdd, used_vars | other.used_vars);
    }
};

template <class FunctionMerge>
void mergeAux2(std::vector<BitBDD> &elems, FunctionMerge f, int maxTime, int maxSize) {
    std::vector <BitBDD> result;
    if (maxSize <= 1 || elems.size() <= 1) {
        return;
    }
    utils::Timer merge_timer;
    //  cout << "Merging " << elems.size() << ", maxSize: " << maxSize << endl;

    //Merge Elements
    std::vector<BitBDD>aux;
    while (elems.size() > 1 && (maxTime == 0 || merge_timer() * 1000 < maxTime)) {
        if (elems.size() % 2 == 1) { //Ensure an even number
            int last = elems.size() - 1;
            try{
                elems[last - 1] = BitBDD(f(elems[last - 1].bdd, elems[last].bdd, maxSize), elems[last].used_vars | elems[last].used_vars);
            }catch (BDDError e) {
                result.push_back(elems[last]);
            }
            elems.erase(elems.end() - 1);
        }
        //    cout << "Iteration: " << elems.size() << endl;
        for (size_t i = 1; i < elems.size(); i += 2) {
            try{
                aux.push_back(BitBDD(f(elems[i - 1].bdd, elems[i].bdd, maxSize), elems[i - 1].used_vars | elems[i].used_vars));
            }catch (BDDError e) {
                if (elems[i].bdd.nodeCount() < elems[i - 1].bdd.nodeCount()) {
                    result.push_back(elems[i - 1]);
                    aux.push_back(elems[i]);
                } else {
                    result.push_back(elems[i]);
                    aux.push_back(elems[i - 1]);
                }
            }
        }
        aux.swap(elems);
        std::vector<BitBDD>().swap(aux);
    }
    //  cout << "Finished: " << elems.size() << endl;
    if (!elems.empty()) {
        result.insert(result.end(), elems.begin(), elems.end());
    }
    result.swap(elems);
    //Add all the elements remaining in aux
    if (!aux.empty()) {
        elems.insert(elems.end(), aux.begin(), aux.end());
    }
    /*for(int i = 0; i < aux.size(); i++){
      elems.push_back(aux[i]);
      }*/

    //  cout << "Merged to " << elems.size() << ". Took "<< merge_timer << " seconds" << endl;
}

/*
 * Same as merge, but maintains a dynamic bitset of used variables.
 */
template <class FunctionMerge>
void merge2(Cudd &manager,std::vector<BitBDD> &elems, FunctionMerge f, int maxTime, int maxSize) {
    manager.SetTimeLimit(maxTime);
    manager.ResetStartTime();
    mergeAux2(elems, f, maxTime, maxSize);
    manager.UnsetTimeLimit();
}

/*
 * Method that merges some elements,
 * Relays on several methods: T, int T.size() and bool T.merge(T, maxSize)
 */
template <class T, class FunctionMerge>
void merge(std::vector<T> &elems, FunctionMerge f, int maxSize) {
    mergeAux(elems, f, 0, maxSize);
}

BDD mergeAndBDD(const BDD &bdd, const BDD &bdd2, int maxSize);
BDD mergeOrBDD(const BDD &bdd, const BDD &bdd2, int maxSize);

ADD mergeSumADD(const ADD &add, const ADD &add2, int );
ADD mergeMaxADD(const ADD &add, const ADD &add2, int );


inline std::string dirname(bool fw) {
    return fw ? "fw" : "bw";
}
}
#endif

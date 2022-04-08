//
// Created by Gael Aglin on 2019-12-23.
//

#ifndef RSBS_RCOVER_H
#define RSBS_RCOVER_H

#include <stack>
#include <vector>
#include <bitset>
#include <iostream>
#include <utility>
#include "globals.h"
#include "dataManager.h"
#include <cmath>
#include <algorithm>

using namespace std;

#define M 64

class RCover {

public:
    stack<bitset<M>>* coverWords;
    int* validWords;
    stack<int> limit;
    int nWords;
    DataManager* dm;
    Supports sup_class = nullptr;
    int support = -1;

    RCover(DataManager* dmm, vector<float>* weights = nullptr);

    RCover(RCover&& cover) noexcept ;

    virtual ~RCover(){
        delete[] coverWords;
        delete[] validWords;
        delete [] sup_class;
    }

    virtual void intersect(Attribute attribute, bool positive = true) = 0;

    virtual pair<Supports, Support> temporaryIntersect(Attribute attribute, bool positive = true) = 0;

    Support temporaryIntersectSup(Attribute attribute, bool positive = true);

    Supports minusMe(bitset<M>* cover1);

    SupportClass countDif(bitset<M>* cover1);

    bitset<M>* getTopBitsetArray() const;

    Support getSupport();

    virtual Supports getSupportPerClass() = 0;

    virtual SupportClass countSupportClass(bitset<M>& coverWord, int wordIndex) = 0;

    virtual Supports getSupportPerClass(bitset<M>** cover, int nValidWords, int* validIndexes) = 0;

    void backtrack();

    void print();

    string outprint();

    class iterator {
    public:
        typedef iterator self_type;
        typedef int value_type;
        typedef int &reference;
        typedef int *pointer;
        typedef std::input_iterator_tag iterator_category;
        typedef int difference_type;

        explicit iterator(RCover *container_, size_t index = 0, bool trans = false) : container(container_) {
            if (trans){
                trans_loop = true;
                if (index == -1) {
                    wordIndex = container->limit.top() - 1;
                    ordered = true;
                } else if (index == 0) {
                    wordIndex = index;
                    ordered = false;
                } 

                pos = 0;
                transInd = 0;
                first = true;
                word = container->coverWords[container->validWords[wordIndex]].top();
                setNextTransID();

            } else{
                trans_loop = false;
                if (index == -1)
                    wordIndex = container->dm->getNClasses();
                else if (index == 0){
                    wordIndex = index;
                    //sup = container->getSupportPerClass().first;
                }
            }
        }

        explicit iterator() : wordIndex(-1), container(nullptr) {}

        int getFirstSetBitPos(long n)
        {
            return log2(n & -n) + 1;
        }

        void setNextTransID() {
            if ((wordIndex >= 0 && ordered) || (wordIndex < container->limit.top() && !ordered)) {
                int indexForTransactions = container->nWords - (container->validWords[wordIndex]+1);
                int pos = getFirstSetBitPos(word.to_ulong());

                if (pos >= 1){
                    if (first){
                        transInd = pos - 1;
                        first = false;
                    }
                    else
                        transInd += pos;

                    value = indexForTransactions * M + transInd;
                    word = (word >> pos);
                } else{
                    if (ordered)
                        --wordIndex;
                    else 
                        ++wordIndex;
                    transInd = 0;
                    first = true;
                    if ((wordIndex >= 0 && ordered) || (wordIndex < container->limit.top() && !ordered)){
                        word = container->coverWords[container->validWords[wordIndex]].top();
                        setNextTransID();
                    }
                }
            }

        }

        value_type operator*() const {
            if (trans_loop){
                if (wordIndex >= container->limit.top()){
                    throw std::out_of_range("Out of Range Exception!");
                }
                else {
                    return value;
                }
            } else{
                if (wordIndex >= container->dm->getNClasses()){
                    throw std::out_of_range("Out of Range Exception!");
                }
                else {
//                    cout << "word ind " << wordIndex << endl;
                    return container->sup_class[wordIndex];
                }
            }

        }

        self_type operator++() {
            //cout << "bbbbarrive" << endl;
            if (trans_loop)
                setNextTransID();
            else
                ++wordIndex;
            return *this;
        }

        bool operator==(const self_type rhs) {
            return container + trans_loop + wordIndex == rhs.container + rhs.trans_loop + rhs.wordIndex;
        }

        bool operator!=(const self_type rhs) {
            return container + trans_loop + wordIndex != rhs.container + rhs.trans_loop + rhs.wordIndex;
        }

        RCover *container;
        int wordIndex;
        int pos;
        int value;
        int transInd;
        bool first;
        bitset<M> word;
        bool trans_loop;
        bool ordered = false;
        //int ntrans = -1;
        //int alltransInd;

    };

    iterator begin(bool trans_loop = false)
    {
        return iterator(this, 0, trans_loop);
    }

    iterator end(bool trans_loop = false)
    {
        return iterator(this, -1, trans_loop);
    }

};


#endif //RSBS_RCOVER_H

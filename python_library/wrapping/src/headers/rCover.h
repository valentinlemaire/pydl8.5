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

    RCover(DataManager* dmm);

    RCover(RCover&& cover) noexcept ;

//    RCover();

    RCover(bitset<M> *bitset1, int nword);

    RCover();

    virtual ~RCover(){
        delete[] coverWords;
        delete[] validWords;
        delete [] sup_class;
    }

//    void intersect1(Attribute attribute, bool positive = true); //not implemented and not used ==> must be removed

    virtual void intersect(Attribute attribute, const vector<float>* weights, bool positive = true) = 0;
    //the two next function must be replace by a virtual abstract function intersect
//    void intersect(Attribute attribute, bool positive = true); //implemented and used for totalfreq intersection
//    void weightedIntersect(Attribute attribute, const vector<float>& weights, bool positive = true); //implemented and useful for weighted intersection

    virtual pair<Supports, Support> temporaryIntersect(Attribute attribute, const vector<float>* weights, bool positive = true) = 0;
    //the two next function must be replace by a virtual abstract function temporaryIntersect
//    pair<Supports, Support> temporaryIntersect(Attribute attribute, bool positive = true); //implemented and used for totalfreq intersection to get support without modifying the cover
//    pair<Supports, Support> temporaryWeightedIntersect(Attribute attribute, const vector<float>& weights, bool positive = true);  //implemented and useful for weighted intersection to get support without modifying the cover

    Support temporaryIntersectSup(Attribute attribute, bool positive = true); //implemented and used for intersection to get support without modifying the cover (base)

//    void intersectAndFillAll(Supports* row, vector<Attribute>& attributes, int start); // implemented but not used. Must be commented

//    void minus(bitset<M>* cover1); // implemented but not used. Must be commented

//    Support minusMee(bitset<M>* cover1); // implemented but not used. Must be commented

    Supports minusMe(bitset<M>* cover1, const vector<float>* weights); //implmeented and used to compute minus operation. must be kept in the base class

    bitset<M>* getTopBitsetArray() const; // must be kept in the base class

    Support getSupport(); // must be kept in the base class

    virtual Supports getSupportPerClass(const vector<float>* weights) = 0;
    //the two next function must be replace by a virtual abstract function getSupportPerClass
//    Supports getSupportPerClass(); // must be kept in the base class
//    Supports getWeightedSupportPerClass(const vector<float>& weights);  // must be present in weighted
//    Supports getClassSupport(); //getSupports of each class. Can be removed. Available with dm->getSupports

//    vector<int> getTransactionsID(bitset<M>& word, int real_word_index); // must be present in weighted

//    vector<int> getTransactionsID();  // must be present in weighted

    void backtrack(); // must be kept in the base class

    void print(); // must be kept in the base class

    string outprint(); // must be kept in the base class

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
                if (index == -1)
                    wordIndex = container->limit.top();
                else if (index == 0){
                    wordIndex = index;
                    pos = 0;
                    transInd = 0;
                    first = true;
                    word = container->coverWords[container->validWords[0]].top();
                    setNextTransID();
                }

            } else{
                trans_loop = false;
                if (index == 0){
                    wordIndex = index;
                    //sup = container->getSupportPerClass().first;
                }
                else if (index == -1)
                    wordIndex = container->dm->getNClasses();
            }
        }

        explicit iterator() : wordIndex(-1), container(nullptr) {}

        int getFirstSetBitPos(long n)
        {
            return log2(n & -n) + 1;
        }

        void setNextTransID() {
            if (wordIndex < container->limit.top()) {
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
                    ++wordIndex;
                    transInd = 0;
                    first = true;
                    if (wordIndex < container->limit.top()){
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

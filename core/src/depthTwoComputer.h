//
// Created by Gael Aglin on 26/09/2020.
//
#ifndef DL85_DEPTHTWOCOMPUTER_H
#define DL85_DEPTHTWOCOMPUTER_H

#include "rCover.h"
#include "trie.h"
#include "query.h"
#include "query_best.h"
#include <chrono>
#include <utility>

using namespace std::chrono;

TrieNode* computeDepthTwo(RCover*, Error, Array<Attribute>, Attribute, Array<Item>, TrieNode*, Query*, Error, Trie*);

struct TreeTwo{
    QueryData_Best* root_data;

    TreeTwo(int n_quantiles){
        root_data = new QueryData_Best(n_quantiles);
    }

    void replaceTree(TreeTwo* cpy){
        free();
        root_data = cpy->root_data;
    }

    void free(){
        if (root_data->lefts[0] || root_data->rights[0]){
            if (root_data->lefts[0]->lefts[0] || root_data->lefts[0]->rights[0]){
                delete root_data->lefts[0]->lefts[0];
                delete root_data->lefts[0]->rights[0];
            }
            if (root_data->rights[0]->lefts[0] || root_data->rights[0]->rights[0]){
                delete root_data->rights[0]->lefts[0];
                delete root_data->rights[0]->rights[0];
            }
            delete root_data->lefts[0];
            delete root_data->rights[0];
        }
        delete root_data;
    }


    ~TreeTwo(){
        free();
    }
};

#endif //DL85_DEPTHTWOCOMPUTER_H

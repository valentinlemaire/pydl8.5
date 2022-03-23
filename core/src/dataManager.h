//
// Created by Gael Aglin on 2019-12-23.
//

#ifndef RSBS_DATAMANAGER_H
#define RSBS_DATAMANAGER_H

#include <bitset>
#include "globals.h"

using namespace std;

#define M 64


class DataManager {

public:
    int nWords;

    DataManager(Supports supports, int ntransactions, int nattributes, int nclasses, int *b, int *c, double *y, int backup_error, float q);

    ~DataManager(){
        for (int i = 0; i < nattributes; ++i) {
            delete[] b[i];
        }
        delete[]b;
        for (int j = 0; j < nclasses; ++j) {
            delete[] c[j];
        }
        delete[]c;
        // deleteSupports(supports);
        delete[] y;
    }

    bitset<M> * getAttributeCover(int attr);

    bitset<M> * getClassCover(int clas);

    float getQ() const {return q;}

    double getY(int idx) const {
        return y[idx];
    }

    /// get number of transactions
    int getNTransactions () const { return ntransactions; }

    /// get number of features
    int getNAttributes () const { return nattributes; }

    /// get number of transactions
    int getNClasses () const { return nclasses; }

    /// get array of support of each class
    Supports getSupports () const { return supports; }

    int getBackupError() const {return backup_error;}

private:
    bitset<M> **b; /// matrix of data
    bitset<M> **c; /// vector of target
    double *y; /// vector of float target values
    Transaction ntransactions; /// number of transactions
    Attribute nattributes; /// number of features
    Class nclasses; /// number of classes
    Supports supports; /// array of support for each class
    int backup_error; // code of the backup error to use
    float q; // value of the quantile when using quantile error

};


#endif //RSBS_DATAMANAGER_H

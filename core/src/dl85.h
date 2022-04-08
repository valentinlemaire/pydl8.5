//
// Created by Gael Aglin on 2019-10-06.
//

#ifndef DL85_DL85_H
#define DL85_DL85_H

#include <iostream>
#include <cstdlib>
#include <cmath>
#include <string>
#include <sstream>
#include <map>
#include <vector>
#include <utility>
#include <functional>
#include <chrono>
#include "globals.h"
#include "dataManager.h"
#include "rCoverTotalFreq.h"
#include "rCoverWeighted.h"
#include "lcm_pruned.h"
#include "query_totalfreq.h"
//#include "query_weighted.h"

using namespace std;

/** search - the starting function that calls all the other to comp
 *
 * @param supports - array of support per class for the whole dataset
 * @param ntransactions - the number of transactions in the dataset
 * @param nattributes - the number of attributes in the dataset
 * @param nclasses - the number of classes in the dataset
 * @param data - a pointer of pointer representing the matrix of data (features values only)
 * @param target - array of targets of the dataset as ints for classification purposes
 * @param float_target - array of targets of the dataset as floats for regression purposes
 * @param maxError - the maximum error that cannot be reached. default value is 0 means that there is no bound
 * @param stopAfterError - boolean variable to state that the search must stop as soon as an error better than "maxError" is reached. Default value is false
 * @param iterative - boolean variable to express whether the search performed will be IDS or not; the default being DFS. Default value is false
 * @param tids_error_class_callback - a callback function from python taking transactions ID of a node as param and returning the error and the class of the node. Default value is null.
 * @param supports_error_class_callback - a callback function from python taking supports per class of a node as param and returning the error and the class of the node. Default value is null.
 * @param tids_error_callback - a callback function from python taking transactions ID of a node as param and returning the error of the node. Default value is null.
 * @param tids_error_class_is_null - a flag caused by cython to handle whether tids_error_class_callback is null or not. Default is true
 * @param supports_error_class_is_null - a flag caused by cython to handle whether supports_error_class_callback is null or not. Default is true
 * @param tids_error_is_null - a flag caused by cython to handle whether tids_error_callback is null or not. Default is true
 * @param maxdepth - the maximum depth of the desired tree. Default value is 1
 * @param minsup - the minimum number of transactions covered by each leaf of the desired tree. Default value is 1
 * @param infoGain - boolean variable to set whether the information gain will be used or not as heuristic to sort the branch to explore. Default is false
 * @param infoAsc - boolean variable to set whether the sort based on information gain will be done increasingly or not. Default is true
 * @param repeatSort - boolean variable to set whether the sort is done at each node or not. If not, it is performed only at the beginning of the search. Default value is false
 * @param backup_error - integer value representing the default error to use if no user coded error is provided. Default is misclassification error.
 * @param q - float value representing the quantile when the quantile loss is optimized.
 * @param timeLimit - the maximum time allocated for the search, expressed in seconds. Default value 0 means that there is no time limit
 * @param continuousMap - a value planned to handle continuous datasets. It is not used currently. Must be set to null
 * @param save - a value planned to handle continuous datasets. It is not used currently. Must be set to false
 * @param verbose_param - a boolean value to set whether the search must be verbose or not. Default value is false
 * @return a string representing a serialized form of the found tree is returned
 */
string search(Supports supports,
              int ntransactions,
              int nattributes,
              int nclasses,
              Bool *data,
              Class *target,
              double *float_target,
              int maxdepth = 1,
              int minsup = 1,
              float* maxError = nullptr,
              bool* stopAfterError = nullptr,
//              bool iterative = false,
              //get a pointer on cover as param and return a vector of float. Due to iterator behaviour of RCover
              // object and the wrapping done in cython, this pointer in python is seen as a list of tids in the cover
              function<vector<float>(RCover *)> tids_error_class_callback = nullptr,
              //get a pointer on cover as param and return a vector of float. Due to iterator behaviour of RCover object
              // and the wrapping done in cython, this pointer in python is seen as a list of support per class of the cover
              function<vector<float>(RCover *)> supports_error_class_callback = nullptr,
              //get a pointer on cover as param and return a float. Due to iterator behaviour of RCover object and the
              // wrapping done in cython, this pointer in python is seen as a list of tids in the cover
              function<float(RCover *)> tids_error_callback = nullptr,
              float *in_weights = nullptr,
              bool tids_error_class_is_null = true,
              bool supports_error_class_is_null = true,
              bool tids_error_is_null = true,
              bool infoGain = false,
              bool infoAsc = true,
              bool repeatSort = false,
              int backup_error = MISCLASSIFICATION_ERROR,
              float * quantiles = new float[1]{0.5},
              int nquantiles = 1,
              int timeLimit = 0,
              bool verbose_param = false);

#endif //DL85_DL85_H

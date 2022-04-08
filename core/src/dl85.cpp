#include "dl85.h"

using namespace std::chrono;

//bool verbose = false;

string search(Supports supports,
              Transaction ntransactions,
              Attribute nattributes,
              Class nclasses,
              Bool *data,
              Class *target,
              double *float_target,
              int maxdepth,
              int minsup,
              float* maxError,
              bool* stopAfterError,
              function<vector<float>(RCover *)> tids_error_class_callback,
              function<vector<float>(RCover *)> supports_error_class_callback,
              function<float(RCover *)> tids_error_callback,
              float* in_weights,
              bool tids_error_class_is_null,
              bool supports_error_class_is_null,
              bool tids_error_is_null,
              bool infoGain,
              bool infoAsc,
              bool repeatSort,
              int backup_error,
              float* quantiles,
              int nquantiles,
              int timeLimit,
              bool verbose_param) {

    //as cython can't set null to function, we use a flag to set the appropriated functions to null in c++
    function<vector<float>(RCover *)> *tids_error_class_callback_pointer = &tids_error_class_callback;
    if (tids_error_class_is_null) tids_error_class_callback_pointer = nullptr;

    function<vector<float>(RCover *)> *supports_error_class_callback_pointer = &supports_error_class_callback;
    if (supports_error_class_is_null) supports_error_class_callback_pointer = nullptr;

    function<float(RCover *)> *tids_error_callback_pointer = &tids_error_callback;
    if (tids_error_is_null) tids_error_callback_pointer = nullptr;

    verbose = verbose_param;
    string out = "";

    if (stopAfterError == nullptr) {
        stopAfterError = new bool[nquantiles];
        for (int i = 0; i< nquantiles; i++) 
            stopAfterError[i] = false;
    }

    if (maxError == nullptr) {
        maxError = new float[nquantiles];
        for (int i = 0; i< nquantiles; i++) 
            maxError[i] = 0;
    }

    auto *dataReader = new DataManager(supports, ntransactions, nattributes, nclasses, data, target, float_target, backup_error, quantiles, nquantiles);

    vector<float> weights;
    if (in_weights) weights = vector<float>(in_weights, in_weights + ntransactions);

    // create an empty trie to store the search space
    Trie *trie = new Trie;

    Query *query = new Query_TotalFreq(minsup, maxdepth, trie, dataReader, timeLimit,
                                       tids_error_class_callback_pointer, supports_error_class_callback_pointer,
                                       tids_error_callback_pointer, maxError, stopAfterError);

    out = "TrainingDistribution: ";
    forEachClass(i) out += std::to_string(dataReader->getSupports()[i]) + " ";
    out += "\n";
    out = "(nItems, nTransactions) : ( " + to_string(dataReader->getNAttributes() * 2) + ", " + to_string(dataReader->getNTransactions()) + " )\n";

    // init variables
    // use the correct cover depending on whether a weight array is provided or not
    RCover *cover;
    if (in_weights) cover = new RCoverWeighted(dataReader, &weights); // weighted cover
    else cover = new RCoverTotalFreq(dataReader); // non-weighted cover


    auto lcm = new LcmPruned(cover, query, infoGain, infoAsc, repeatSort);
    auto start_tree = high_resolution_clock::now();
    ((LcmPruned *) lcm)->run(); // perform the search
    auto stop_tree = high_resolution_clock::now();
    Tree **out_trees = new Tree*[cover->dm->getNQuantiles()];
    for (int i = 0; i < cover->dm->getNQuantiles(); i++) {
        out_trees[i] = new Tree();
        query->printResult(out_trees[i], i);
        out_trees[i]->latSize = ((LcmPruned *) lcm)->latticesize;
        out_trees[i]->searchRt = duration<double>(stop_tree - start_tree).count();
    }

    //query->printResult(tree_out); // build the tree model

    out += "Trees: \n";
    for (int i = 0; i < cover->dm->getNQuantiles(); i++) {
        out += out_trees[i]->to_str();
        out += "\n";

    }

    delete trie;
    delete query;
    delete dataReader;
    delete cover;
    delete lcm;

    for (int i = 0; i < cover->dm->getNQuantiles(); i++) {
        delete out_trees[i];
    }
    delete[] out_trees;

//    auto stop = high_resolution_clock::now();
//    cout << "Durée totale de l'algo : " << duration<double>(stop - start).count() << endl;

    return out;
}

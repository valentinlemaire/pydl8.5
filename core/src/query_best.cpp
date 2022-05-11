#include "query_best.h"
#include <iostream>

using namespace std;

Query_Best::Query_Best(Support minsup,
                       Depth maxdepth,
                       Trie *trie,
                       DataManager *data,
                       int timeLimit,
                       function<vector<float>(RCover *)> *tids_error_class_callback,
                       function<vector<float>(RCover *)> *supports_error_class_callback,
                       function<float(RCover *)> *tids_error_callback,
                       float *maxError,
                       bool *stopAfterError)
    : Query(minsup,
            maxdepth,
            trie,
            data,
            timeLimit,
            tids_error_class_callback,
            supports_error_class_callback,
            tids_error_callback,
            maxError,
            stopAfterError)
{
}

Query_Best::~Query_Best() {}

QueryData_Best::~QueryData_Best() {
    if (freed)
        return;
    if (tests != nullptr)
        delete[] tests;
    if (leafErrors != nullptr)
        delete[] leafErrors;
    if (errors != nullptr)
        delete[] errors;
    if (lowerBounds != nullptr)
        delete[] lowerBounds;
    if (sizes != nullptr)
        delete[] sizes;

    for (int i = 0; i < n_quantiles; i++)
    {
        if (lefts != nullptr && lefts[i] != nullptr)
        {
            //delete lefts[i];
            lefts[i] = nullptr;
        }
        if (rights != nullptr && rights[i] != nullptr)
        {
            //delete rights[i];
            rights[i] = nullptr;
        }
    }

    delete[] lefts;
    delete[] rights;
    freed = true;

    
    std::cout << "deleted trie query data" << std::endl;
}

void Query_Best::printResult(Tree *tree, int quantile_idx)
{
    printResult((QueryData_Best *)realroot->data, tree, quantile_idx);
}

void Query_Best::printResult(QueryData_Best *data, Tree *tree, int quantile_idx)
{
    int depth;
    if (data->sizes[quantile_idx] == 0 || (data->sizes[quantile_idx] == 1 && floatEqual(data->errors[quantile_idx], FLT_MAX)))
    {
        tree->expression = "(No such tree)";
        if (timeLimitReached)
            tree->timeout = true;
    }
    else
    {
        tree->expression = "";
        depth = printResult(data, 1, tree, quantile_idx);
        tree->expression += "}";
        tree->size = data->sizes[quantile_idx];
        tree->depth = depth - 1;
        tree->trainingError = data->errors[quantile_idx];
        tree->accuracy = 1 - tree->trainingError / float(dm->getNTransactions());
        if (timeLimitReached)
            tree->timeout = true;
    }
}

int Query_Best::printResult(QueryData_Best *data, int depth, Tree *tree, int quantile_idx)
{
    if (!data->lefts[quantile_idx])
    { // leaf
        if (tids_error_callback)
        {
            tree->expression += R"({"value": "undefined", "error": )" + std::to_string(data->errors[quantile_idx]);
        }
        else
        {
            tree->expression += "{\"value\": " + std::to_string(data->tests[quantile_idx]) + ", \"error\": " + std::to_string(data->errors[quantile_idx]);
        }

        return depth;
    }
    else
    {
        tree->expression += "{\"feat\": " + std::to_string(data->tests[quantile_idx]) + ", \"left\": ";

        // perhaps strange, but we have stored the positive outcome in right, generally, people think otherwise... :-)
        int left_depth = printResult(data->rights[quantile_idx], depth + 1, tree, quantile_idx);
        tree->expression += "}, \"right\": ";
        int right_depth = printResult(data->lefts[quantile_idx], depth + 1, tree, quantile_idx);
        tree->expression += "}";
        return max(left_depth, right_depth);
    }
}

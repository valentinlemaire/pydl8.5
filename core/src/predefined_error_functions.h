#ifndef PREDEFINED_ERROR_FUNCTIONS_H
#define PREDEFINED_ERROR_FUNCTIONS_H

#include <math.h>
#include "rCover.h"

class QuantileLossComputer {
    int n_quantiles;
    double *h = nullptr;
    int *h_low = nullptr;
    int *h_up = nullptr;
    double *y_low = nullptr;
    double *y_pred = nullptr;
    double *under = nullptr;
    double *above = nullptr;



    public: 
        QuantileLossComputer(int n_quantiles);

        ~QuantileLossComputer() {
            delete[] h;
            delete[] h_low;
            delete[] h_up;
            delete[] y_low;
            delete[] y_pred;
            delete[] under;
            delete[] above;
        }

        Error * quantile_tids_errors(RCover* cover);
        Error * quantile_tids_errors_fast(RCover* cover);
        Error * quantile_tids_errors_slow(RCover* cover);
};


float sse_tids_error(RCover* cover);


#endif
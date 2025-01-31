#include "predefined_error_functions.h"

float sse_tids_error(RCover *cover)
{
    RCover::iterator it;

    // Computing the mean of the targets
    float sum = 0.;
    int count = 0;
    for (it = cover->begin(true); it.wordIndex < cover->limit.top(); ++it)
    {
        int idx = it.value;
        sum += cover->dm->getY(idx);
        count += 1;
    }

    float centroid = sum / count;
    float sse = 0.;

    // summing up squared errors to the centroid
    for (it = cover->begin(true); it.wordIndex < cover->limit.top(); ++it)
    {
        int idx = it.value;
        float delta = cover->dm->getY(idx) - centroid;
        sse += delta * delta;
    }

    return sse;
}

QuantileLossComputer::QuantileLossComputer(int n_quantiles) : n_quantiles(n_quantiles)
{
    h = new double[n_quantiles];
    h_low = new int[n_quantiles];
    h_up = new int[n_quantiles];
    y_low = new double[n_quantiles];
    under = new double[n_quantiles];
    above = new double[n_quantiles];
    y_pred = new double[n_quantiles];
}

Error *QuantileLossComputer::quantile_tids_errors_fast(RCover *cover)
{
    RCover::iterator it;

    int N = cover->getSupport();

    int n_quantiles = cover->dm->getNQuantiles();

    float *errors = new float[n_quantiles];

    float h_tmp;
    int i;
    float q;

    for (i = 0; i < n_quantiles; i++)
    {
        q = cover->dm->getQuantile(i);
        h_tmp = (N - 1) * q;
        h[i] = h_tmp;

        if (cover->dm->getQuantileMode() == LINEAR)
        {
            h_low[i] = floor(h_tmp);
            h_up[i] = ceil(h_tmp);
        }
        else if (cover->dm->getQuantileMode() == OPTIMAL)
        {
            if (q < 0.5)
            {
                h_low[i] = ceil(h_tmp);
                h_up[i] = ceil(h_tmp);
            }
            else if (q == 0.5)
            {
                h_low[i] = floor(h_tmp);
                h_up[i] = ceil(h_tmp);
            }
            else if (q > 0.5)
            {
                h_low[i] = floor(h_tmp);
                h_up[i] = floor(h_tmp);
            }
        }

        y_low[i] = -1;
        y_pred[i] = -1;

        under[i] = 0.;
        above[i] = 0.;
    }

    // Indices going from 0 to N-1 (pseudo indices of the cover)
    int sub_idx = 0;

    // True indices of the full y array
    int idx;

    int idx_for_low_val = 0;
    int idx_for_up_val = 0;

    int idx_for_low_sums = 0;
    int idx_for_up_sums = -1;

    double y_cur;
    double y_prev = -1e15;

    // for (it = cover->begin(true); it.wordIndex < cover->limit.top(); ++it) {
    for (it = cover->end(true); it.wordIndex >= 0; ++it)
    {
        idx = it.value;
        y_cur = cover->dm->getY(idx);

        if (y_prev > y_cur)
        {
            std::cout << "Error: array not sorted" << std::endl;
            std::cout << idx << std::endl;
        }
        y_prev = y_cur;

        // Add the current element to the sum of the elements above the current quantile
        if (idx_for_up_sums >= 0)
        {
            // If for this quantile h_up == h_low and the current index is equal to those values, we do not add the current element to the above count as it was already counted in the below count
            if (h_up[idx_for_up_sums] == h_low[idx_for_up_sums])
            {
                if (sub_idx > h_up[idx_for_up_sums])
                    above[idx_for_up_sums] += y_cur;
            }
            else if (sub_idx >= h_up[idx_for_up_sums])
            {
                above[idx_for_up_sums] += y_cur;
            }
        }

        if (idx_for_low_sums < n_quantiles)
        {
            under[idx_for_low_sums] += y_cur;

            // If we get to the quantile value we switch to the next one
            while (sub_idx == h_low[idx_for_low_sums])
            {
                idx_for_low_sums += 1;
                idx_for_up_sums += 1;

                // If for this quantile h_up == h_low and the current index is equal to those values, we do not add the current element to the above count as it was already counted in the below count
                if (h_up[idx_for_up_sums] == h_low[idx_for_up_sums])
                {
                    if (sub_idx > h_up[idx_for_up_sums])
                        above[idx_for_up_sums] += y_cur;
                }
                else if (sub_idx >= h_up[idx_for_up_sums])
                {
                    above[idx_for_up_sums] += y_cur;
                }

                // The sum of the elements below the next quantiles is the sum of the below the previous quantile + the elements in between
                if (idx_for_low_sums < n_quantiles)
                    under[idx_for_low_sums] += under[idx_for_up_sums];
            }
        }

        if (idx_for_low_val < n_quantiles)
        {
            // If the current value is equal to h_low for the quantile, we save it and switch to the next quantile
            while (sub_idx == h_low[idx_for_low_val])
            {
                y_low[idx_for_low_val] = y_cur;

                idx_for_low_val += 1;
            }
        }

        if (idx_for_up_val < n_quantiles)
        {
            // If the current value is equal to h_up for the quantile, we compute y_pred and switch to the next quantile
            while (sub_idx == h_up[idx_for_up_val])
            {
                y_pred[idx_for_up_val] = y_low[idx_for_up_val] + (h[idx_for_up_val] - h_low[idx_for_up_val]) * (y_cur - y_low[idx_for_up_val]);

                idx_for_up_val += 1;
            }
        }

        sub_idx += 1;
    }

    double sum = 0.;
    for (int i = n_quantiles - 1; i >= 0; i--)
    {
        // In reverse order, we must add the sum of the elements above the quantile above itself to the above count as they were not accounted in the previous loop
        above[i] += sum;
        sum = above[i];

        // We compute sum(y_p - y) for the elements above and under the predicted value
        under[i] = ((double)(h_low[i] + 1)) * y_pred[i] - under[i];
        above[i] = above[i] - (((double)(N - (h_low[i] + 1))) * y_pred[i]);

        // The final error is the sum of the above and below differences multiplied by q and q-1 respectively
        double q_i = (double)cover->dm->getQuantile(i);
        errors[i] = under[i] * (1 - q_i) + above[i] * q_i;
    }

    return errors;
}

Error *QuantileLossComputer::quantile_tids_errors_slow(RCover *cover)
{
    int N = cover->getSupport();
    double *y = new double[N];
    int n_q = cover->dm->getNQuantiles();

    RCover::iterator it;

    int i = 0;
    for (it = cover->begin(true); it.wordIndex < cover->limit.top(); ++it)
    {
        y[i] = cover->dm->getY(it.value);
        i++;
    }

    sort(y, y + N);

    double *y_pred = new double[n_q];
    for (int j = 0; j < n_q; j++)
    {
        float q = cover->dm->getQuantile(j);
        float h = (N - 1) * q;

        y_pred[j] = y[(int)floor(h)] + (h - floor(h)) * (y[(int)ceil(h)] - y[(int)floor(h)]);
    }

    Error *errors = new Error[n_q];
    for (int i = 0; i < n_q; i++)
    {
        errors[i] = 0;
    }

    double *above = new double[n_q];
    double *under = new double[n_q];

    for (int j = 0; j < n_q; j++)
    {
        for (int i = 0; i < N; i++)
        {
            double q = (double)cover->dm->getQuantile(j);
            if (y[i] <= y_pred[j])
            {
                under[j] += (y[i]);
            }
            else
            {
                above[j] += (y[i]);
            }
            errors[j] += fmax(q * (y[i] - y_pred[j]), (1 - q) * (y_pred[j] - y[i]));
        }
    }

    delete[] under;
    delete[] above;
    delete[] y_pred;

    return errors;
}

Error *QuantileLossComputer::quantile_tids_errors(RCover *cover)
{
    return QuantileLossComputer::quantile_tids_errors_fast(cover);
}

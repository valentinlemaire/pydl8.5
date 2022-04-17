from sklearn.base import RegressorMixin
from ...predictors.distribution_predictor import DL85DistributionPredictor
import numpy as np
from math import floor, ceil
import json


class DL85DistributionRegressor(DL85DistributionPredictor, RegressorMixin):
    """An optimal binary decision tree regressor.
    Parameters
    ----------
    max_depth : int, default=1
        Maximum depth of the tree to be found
    min_sup : int, default=1
        Minimum number of examples per leaf
    max_error : int, default=0
        Maximum allowed error. Default value stands for no bound. If no tree can be found that is strictly better, the model remains empty.
    stop_after_better : bool, default=False
        A parameter used to indicate if the search will stop after finding a tree better than max_error
    time_limit : int, default=0
        Allocated time in second(s) for the search. Default value stands for no limit. The best tree found within the time limit is stored, if this tree is better than max_error.
    verbose : bool, default=False
        A parameter used to switch on/off the print of what happens during the search
    desc : bool, default=False
        A parameter used to indicate if the sorting of the items is done in descending order of information gain
    asc : bool, default=False
        A parameter used to indicate if the sorting of the items is done in ascending order of information gain
    repeat_sort : bool, default=False
        A parameter used to indicate whether the sorting of items is done at each level of the lattice or only before the search
    print_output : bool, default=False
        A parameter used to indicate if the search output will be printed or not
    backup_error : str, default = "mse"
        Error to optimize if no user error function is provided. Can be one of {"mse", "quantile"}
    quantile_value: float, default = 0.5 
        Quantile value. Only used when backup_error is "quantile"

    Attributes
    ----------
    tree_ : str
        Outputted tree in serialized form; remains empty as long as no model is learned.
    size_ : int
        The size of the outputted tree
    depth_ : int
        Depth of the found tree
    error_ : float
        Error of the found tree
    accuracy_ : float
        Accuracy of the found tree on training set
    lattice_size_ : int
        The number of nodes explored before found the optimal tree
    runtime_ : float
        Time of the optimal decision tree search
    timeout_ : bool
        Whether the search reached timeout or not
    classes_ : ndarray, shape (n_classes,)
        The classes seen at :meth:`fit`.
    """

    def __init__(
        self,
        max_depth=1,
        min_sup=1,
        max_errors=None,
        stop_after_better=None,
        time_limit=0,
        verbose=False,
        desc=False,
        asc=False,
        repeat_sort=False,
        leaf_value_function=None,
        print_output=False,
        quantiles=[0.5],
        quantile_estimation = "linear",
    ):

        
        DL85DistributionPredictor.__init__(
            self,
            max_depth=max_depth,
            min_sup=min_sup,
            max_errors=max_errors,
            stop_after_better=stop_after_better,
            time_limit=time_limit,
            verbose=verbose,
            desc=desc,
            asc=asc,
            repeat_sort=repeat_sort,
            leaf_value_function=leaf_value_function,
            print_output=print_output,
            quantiles=quantiles,
            quantile_estimation=quantile_estimation,
        )

        self.to_redefine = self.leaf_value_function is None
        self.backup_error = "quantile"

    @staticmethod 
    def quantile_linear_estimation(tids, y, q):
        return np.quantile(y[list(tids)], q)
    
    @staticmethod 
    def quantile_optimal_estimation(tids, y, q):
        N = len(tids)
        h = (N-1)*q
        y_sorted = sorted(y[list(tids)])
        if q < 0.5:
            return y_sorted[ceil(h)]
        elif q == 0.5: 
            return (y_sorted[floor(h)] + y_sorted[ceil(h)])/2
        elif q > 0.5:
            return y_sorted[floor(h)]

    def fit(self, X, y):
        """Implements the standard fitting function for a DL8.5 regressor.
        Parameters
        ----------
        X : array-like, shape (n_samples, n_features)
            The training input samples.

        y : array-like, shape (n_samples, n_predictions)
            The training output samples.

        Returns
        -------
        self : object
            Returns self.
        """
        idx = np.argsort(y)
        X = X[idx]
        y = y[idx]

        if self.to_redefine:
            if self.quantile_estimation == "linear":
                self.leaf_value_function = lambda tids, q: self.quantile_linear_estimation(tids, y, q)
            elif self.quantile_estimation == "optimal":
                self.leaf_value_function = lambda tids, q: self.quantile_optimal_estimation(tids, y, q)

        # call fit method of the predictor
        DL85DistributionPredictor.fit(self, X, y)

        # Return the regressor
        return self

    def predict(self, X):
        """Implements the standard predict function for a DL8.5 regressor.
        Parameters
        ----------
        X : array-like, shape (n_samples, n_features)
            The input samples.
        Returns
        -------
        y : ndarray, shape (n_samples,)
            The predicted value for each sample is the mean of the closest samples seen during fit.
        """

        return DL85DistributionPredictor.predict(self, X)


    def save(self, filename:str):
        """Saves the model in a file.
        Parameters
        ----------
        filename : str
            The name of the file where the model will be saved.
        """
    
        with open(filename, "w") as f:
            attr_dict = self.__dict__
            del attr_dict['leaf_value_function']
            json.dump(self.__dict__, f)

    @classmethod
    def load(cls, filename: str):
        """Loads a model from a file.
        Parameters
        ----------
        filename : str
            The name of the file where the model is saved.
        Returns
        -------
        model : DL85DistributionRegressor
            The loaded model.
        """
        with open(filename, "r") as f:
            attrs = json.load(f)
        model = cls()
        
        for attr, value in attrs.items():
            setattr(model, attr, value)
        return model
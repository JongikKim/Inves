# Inves

Inves is a research prototype for a graph similarity search technique with graph edit distance constraints.
The related publication is "Inves: Incremental partitioning-based verification for graph similarity search", EDBT 2019.

# How to build
> make

# How to run
> inves  max_threshold  data_file

data_file should be prepared in MOLFILE format (to use other file format, please refer to include/datafile.h and src/datafile.cpp).

Inves randomly selects 100 queries from the dataset and perform graph similarity searches for each threshold up to max_threshold (to use a custom workload, load the workload in the generateWorkload function in src/main.cpp).

For example, the following command runs Inves on the AIDS dataset for each threshold in [1, 3]

>inves 3 AID2DA99.sdz

For each threshold, Inves outputs total query time, total number of candidates, and total number of results.
It does not output or save result graphs.

The results are saved in simsearch::res_vec (refer to include/simsearch.h and simsearch::run() in src/simsearch.cpp). simsearch::res_vec does not directly contain result graphs but it contains indices of DataSet::data (refer to include/dataset.h).

We recommend to use Inves::COMPONENT_SIZE_THRESHOLD = 5 or 6 (please modify the parameter value in main.cpp).

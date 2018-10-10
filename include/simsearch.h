#ifndef __SIMSEARCH_H
#define __SIMSEARCH_H

#include <algorithm>
#include <cstdlib>
#include <list>
#include <set>

#include "dataset.h"
#include "graph.h"
#include "looptimer.h"
#include "mappinglist.h"

#define PARTIME 1 // incremental partitioning time
#define GEDTIME 2 // ged computation time

class simsearch
{
public:
	static int threshold;

private:
	loop_timer* timer;

	//statistics
	long long cands_final;
	vector<int> res_vec;

	void clearStats();

private:
	graph* qgraph;

public:
	simsearch();
	~simsearch();

public:
	void run(vector<graph*>& workload);

private:
	void invesearch();
};

#endif //__SIMSEARCH_H

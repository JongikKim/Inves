#include "simsearch.h"
#include "ged.h"
#include "inves.h"

int simsearch::threshold;

simsearch::simsearch()
{
	simsearch::threshold = 0;
	this->qgraph = NULL;

	clearStats();
}

simsearch::~simsearch(){}

void simsearch::clearStats()
{
	cands_final = 0;
	res_vec.clear();
}

void simsearch::run(vector<graph*>& workload)
{
	clearStats();

	const char* headers[] = {"partition time", "ged time"};
	timer = new loop_timer("INVES", "", workload.size(), headers, 3); 
	timer->start();

	for(unsigned i = 0; i < workload.size(); i++){
		qgraph = workload[i];
		invesearch();

		timer->next();
	}
	timer->end();

	cout << " #total candidates:" << cands_final << " #total results:" << res_vec.size() << endl;
//	for(unsigned i = 0; i < res_vec.size(); i++) cout << res_vec[i] << " "; cout << endl;

	delete timer;
}

void simsearch::invesearch()
{
	DataSet* dataset = DataSet::getInstance();
	graph* x = qgraph;

	vector<int> candidates;
	candidates.reserve(dataset->numGraphs());
	for(unsigned i = 0; i < dataset->numGraphs(); i++){
		graph* y = dataset->graphAt(i);
		if(y == NULL || x->sizeFilter(*y) > threshold
		             || x->labelFilter(*y) > threshold) continue;
		candidates.push_back(i);
	}

	timer->start(PARTIME); // record partitioning time
	                       // ged time is recorded in InvesVerifier
	for(unsigned i = 0; i < candidates.size(); i++){
		graph* y = dataset->graphAt(candidates[i]);

		InvesVerifier verifier(x, y, timer);
		bool res = verifier.verify(cands_final);
		if(res) res_vec.push_back(candidates[i]);
	}
	timer->stop(PARTIME);
}


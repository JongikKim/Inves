#include <iostream>
#include <sys/stat.h>
#include <sys/types.h>
#include <cstring>
#include <cstdlib>

#include "ged.h"
#include "datafile.h"
#include "dataset.h"
#include "simsearch.h"
#include "inves.h"

using namespace std;

void generateWorkload(vector<graph*>& workload)
{
//	srand(time(NULL));
	DataSet* dataset = DataSet::getInstance();

	while(workload.size() < 100){
		int gid = rand() % dataset->numGraphs();
		if(dataset->graphAt(gid) == NULL) continue;
		workload.push_back(dataset->graphAt(gid));
	}
}

int main(int argc, char* argv[])
{
	if(argc < 3){
		cout << "usage: run max_threshold data_file(in MOLFILE format)" << endl;
		exit(1);
	}

	int max_threshold = atoi(argv[1]);

	// Building dataset from a file in molfile format
	DataFile* file = new MolFile(argv[2]); 
	DataSet* dataset = DataSet::getInstance();
	dataset->buildDataSet(*file);
	file->close();
	delete file;

	// query workload: randomly selected from dataset
	// please see generateWorkload function above
	vector<graph*> workload;
	generateWorkload(workload);

	// set the max size of a disconnected component which should be ignored
	InvesVerifier::DISCONNECTED_SIZE = 1;

	simsearch searcher;
	for(simsearch::threshold = 1; simsearch::threshold <= max_threshold; simsearch::threshold++){
		cout << "GED threshold=" << simsearch::threshold << endl << flush;
		searcher.run(workload);
		cout << endl << flush;
	}

	DataSet::finishUp();

	return 0;
}

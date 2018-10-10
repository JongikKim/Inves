#ifndef __DATASET_H
#define __DATASET_H

#include "graph.h"
#include "datafile.h"

#include <vector>
#include <map>
#include <iostream>

using namespace std;

extern bool debug;
class DataSet
{
private:
	vector<graph*> data;
	double* vsupport;

private:
	static DataSet* instance;

private:
	DataSet() : vsupport(NULL){}
	~DataSet()
	{
		// delete data graphs
		for(unsigned i = 0; i < data.size(); i++) delete data[i];
		if(vsupport) delete [] vsupport;
	}

public:
	static DataSet* getInstance()
	{
		if(instance == NULL) instance = new DataSet();
		return instance;
	}

	static void finishUp() { delete instance; }

	void buildDataSet(DataFile& file)
	{
		graph* g = NULL;
		for(int id = 0; (g=file.getNextGraph()) != NULL; id++){
			data.push_back(g);
			cout << "\rReading dataset: " << id + 1 << " graphs" << flush;
		}

		cout << endl << "Building statistics (label multisets and frequencies) .. ";

		for(unsigned i = 0; i < data.size(); i++)
			if(data[i] != NULL) data[i]->countLabels();

		calculateVertexInnerSupport();

		cout << " Done" << endl << endl << flush;

//		cout << "avg graph size: " << (double)totalgraphsize/(data.size() - skip);
//		cout << " avg graph edge size: " << (double)totalgraphedgesize/(data.size() - skip) << endl;
	}

	unsigned numGraphs() { return data.size(); }
	graph* graphAt(int i){ return data[i]; }

	double support(int vlabel) { return vsupport[vlabel]; } 

private:
	void calculateVertexInnerSupport()
	{
		// n_vertices is in fact # of vertex labels + 1 (since 0 is not used)
		int n_vertices = graph::nl_map.size() + 1;

		int* termfreq = new int[n_vertices];
		int* docufreq = new int[n_vertices];

		for(int i = 0; i < n_vertices; i++) termfreq[i] = docufreq[i] = 0;

		for(unsigned i = 0; i < data.size(); i++){
			if(data[i] == NULL) continue;
			graph* g = data[i];

			for(int i = 0; i < g->size(); i++){
				termfreq[g->vertexLabel(i)]++;
				if(docufreq[g->vertexLabel(i)] >= 0)
					docufreq[g->vertexLabel(i)] = -1*(docufreq[g->vertexLabel(i)]+1);
			}

			for(int i = 0; i < n_vertices; i++)
				if(docufreq[i] < 0) docufreq[i] = -1*docufreq[i];
		}

		vsupport = new double[n_vertices];
		for(int i = 1; i < n_vertices; i++)
			vsupport[i] = (double)termfreq[i]/(double)docufreq[i];

		delete [] termfreq;
		delete [] docufreq;
	}
};

#endif

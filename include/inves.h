#ifndef PAGES_H
#define PAGES_H

#include "looptimer.h"
#include "graph.h"
#include "ged.h"
#include "dataset.h"

class vertexCompare
{
private:
	graph* src;
	DataSet* dataset;

public:
	vertexCompare(graph* src) : src(src)
	{
		dataset = DataSet::getInstance();
	}

	bool operator () (const int& u, const int& v)
	{
		unsigned freq1 = dataset->support(src->vertexLabel(u));
		unsigned freq2 = dataset->support(src->vertexLabel(v));

		if(freq1 == freq2)
			return src->vertexDegree(u) < src->vertexDegree(v);
		return freq1 < freq2;
	}
};

class InvesVerifier
{
public:
	static int DISCONNECTED_SIZE;

private:
	loop_timer* timer;

private:
	graph* x;
	graph* y;
	graph* x_reorder;

	int tau;

private:
	vector<int>* partition;
	vector<int> deadends; // vertices in tiny connected components

private:
	int* source;
	int* target;

private: // initialized in incrementalPartitioning
	bool* F;
	int bridge_error; // errors in bridges
	int psize; // size of the current partition

public:
	int fail_position; // maching fail position

public:
	InvesVerifier(graph* x, graph* y, loop_timer* timer);
	~InvesVerifier();

	bool verify(long long& cands_final);

private:
	bool incrementalPartitioning(int* partition, int& offset);

private:
	int removeDeadEnd(vector<int>& disconn);
	int reorderPartition();

	bool rematch();
	bool inducedSI(int d);
	bool valid(int u, int v, int d);

	graph* reorderGraph(int* vertices, int offset);
	bool verifyGED(int* vertices, int offset, bool partial);
};

#endif

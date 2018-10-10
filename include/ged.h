#ifndef __GED_H
#define __GED_H

#define EPSILON -1

#include "graph.h"
#include "mappinglist.h"
#include <deque>

#include <algorithm> // for heap functions

using namespace std;

class GED
{
public:
	bool verbose;

private:
	graph* fromGraph;
	graph* toGraph;

private:
	map<int,vector<int> > toVertexMap;
	map<int,vector<int> > fromVertexMap;

public:
	GED(graph* from, graph* to);
	int getDistance(mappinglist* initMapping = NULL);

	friend class mappinglist;
};


extern long long push_count;

class PriorityQueue
{
private:
	deque<mappinglist*> q;

	struct MLComparator
	{
		bool operator() (mappinglist* l1, mappinglist* l2)
		{
			int d1 = l1->getDistance();
			int d2 = l2->getDistance();

			if(d1 == d2) return l1->size() <= l2->size();
			return d1 > d2;
		}
	};

public:
	PriorityQueue(){}
	void push(mappinglist* ml){

/*
		if(push_count == numeric_limits<long long>::max()){
			cerr << "push_count overflows!!" << endl;
			exit(1);
		}
		push_count++;
*/

		q.push_back(ml);

		if(q.size() == 1) make_heap(q.begin(), q.end(), MLComparator());
		else push_heap(q.begin(), q.end(), MLComparator());
	}

	mappinglist* pop(){
		mappinglist* ml = NULL;
		if(!q.empty()){
			pop_heap(q.begin(), q.end(), MLComparator());
			ml = q.back(); q.pop_back();
		}
		return ml;
	}

	bool empty(){ return q.empty(); }

	void clear(){
		for(unsigned i = 0; i < q.size(); i++) delete q[i];
		deque<mappinglist*> tmp;
		swap(q, tmp);
	}

	unsigned size(){ return q.size(); }
};
#endif // __GED_H

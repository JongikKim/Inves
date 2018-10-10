#ifndef __MAPPING_LIST
#define __MAPPING_LIST

#include "graph.h"

#include <vector>
#include <iostream>

using namespace std;

class GED;

#define NO_DIST  0x0000000
#define VERTICES 0x0000001
#define FULLEDGE 0x0000002
#define BRIDGE   0x0000004
#define ESTIMATE 0x0000008

class mappinglist
{
private:
	graph* fr_graph;
	graph* to_graph;

private:
	vector<int> fr_vertices;
	vector<int> to_vertices;

	int next_from; // position of first false of  mapped_from;
	bool* mapped_fr; // size of from_graph
	bool* mapped_to; // size of to_graph
	
private: // distances
	int distmode;

	int vertexdist;
	int edgedist;
	int bridgedist;
	int estimated;

public:
	mappinglist(graph* from, graph* to, int mode = -1);
	mappinglist(const mappinglist& rh);
	~mappinglist();
	
private: // simple inline operations
	void updateVertexDist(int from, int to){
		if(fr_graph->vertexLabel(from) != to_graph->vertexLabel(to)) vertexdist++;
	}
	
	void updateEdgeDist(int from, int to){
		for(unsigned i = 0; i < fr_vertices.size(); i++){
			if(fr_graph->edgeLabel(from, fr_vertices[i]) != to_graph->edgeLabel(to, to_vertices[i]))
				edgedist++;
		}
	}

public:
	void updateBridgeDist();
	void estimateRemainingDist();
	
public:
	void addDistMode(int mode) { distmode |= mode; }
	void removeDistMode(int mode) { distmode &= (mode ^ 0xFFFFFFFF); }
	void setDistMode(int mode) { distmode = mode; }

	int getDistance(){
		return vertexdist + edgedist + bridgedist + estimated;
	}

	int size() { return fr_vertices.size(); }

public:
	int getNextFromVertex() { return next_from; }
	
	void getUnmappedToVertices(vector<int>& unmappedTo){
		unmappedTo.reserve(to_graph->size());
		for(int i = 0; i < to_graph->size(); i++)
			if(!mapped_to[i]) unmappedTo.push_back(i);
	}
	
	void getUnmappedFromVertices(vector<int>& unmappedFrom){
		unmappedFrom.reserve(fr_graph->size());
		for(int i = 0; i < fr_graph->size(); i++)
			if(!mapped_fr[i]) unmappedFrom.push_back(i);
	}

public:
	void pushBack(int from, int to);

public:
	void print(){
		cout << "Distance: " << getDistance() << " vertexDist: " << vertexdist
			 << " edgeDist: " << edgedist << " bridgeDist: " << bridgedist << " estimated: " << estimated << endl;
		for(unsigned i = 0; i < fr_vertices.size(); i++)
			cout << fr_vertices[i] << "-->" << to_vertices[i] << endl;
	}

public:
	bool complete;
};

#endif //__MAPPING_LIST

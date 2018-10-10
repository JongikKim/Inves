#ifndef __GRAPH_H
#define __GRAPH_H

inline int ABS(int x) { return x < 0 ? -x : x; }

#include <string>
#include <vector>
#include <list>
#include <map>
#include <set>
#include <iostream>

using namespace std;

typedef map<int,vector<int> >::iterator VMapIter;
class graph
{
public:
	static map<string,int> nl_map; // vertex label map --> should be shared by all instances
	static map<string,int> el_map; // edge label map --> should be shared by all instances

private:
	int* el_array; // edge label count array
	int* vl_array; // vertex label count array
	vector<int>* vl_list; // vertex label inverted lists

public:
	static unsigned numVertexLabels() { return nl_map.size(); }
	static unsigned numEdgeLabels() { return el_map.size(); }

private:
	int** matrix;
	int num_vertices;
	int num_edges;
	int* vertex_labels;

	int** vertex_esize;

public: // constructors & destructor
	graph(int n_vertices);
	graph(graph& rh, int* old);
	~graph();

public: // reordering
	void DFSOrder(int v, bool* visited, int* subgraph, int subsize, vector<int>& order);
	void determineVertexOrder(int* vertices, int offset, vector<int>& order, vector<int>* disconn = NULL);

public: // mutators
	void setVertexLabel(int vertex, string label);
	void setEdgeBond(int vertex1, int vertex2, int bond);
	void setEdgeLabel(int vertex1, int vertex2, string label);

public: // accessors
	int numVertices() { return num_vertices; }
	int numEdges() { return num_edges; }

	int size(){ return numVertices(); }

public: // accessors
	int vertexLabel(int vertex);
	int edgeLabel(int vertex1, int vertex2);

public: // operation
	void countLabels();
	void countLabels(int* vlabel, int* elabel, vector<int>& with);

public:
	vector<int>& findVertices(int label){ return vl_list[label-1]; }
    int vertexDegree(int vertex){ return vertex_esize[vertex][el_map.size()]; }

private:
	static int labelErrors(int* arr_l, int* arr_r, int size);
	
public:
	int edgeLabelErrors(int v1, graph* g, int v2){
		return labelErrors(vertex_esize[v1], g->vertex_esize[v2], numEdgeLabels());	
	}
	
	int labelFilter(graph& rh);
	int labelFilter(graph& rh, vector<int>& without_l, vector<int>& without_r);
	int sizeFilter(graph& rh);

	void print(bool showmatrix = true);

public:
	friend class mappinglist;
};

#endif // __GRAPH_H

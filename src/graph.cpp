#include "graph.h"
#include "simsearch.h"

#include <iostream>
#include <sstream>
#include <string>
#include <limits>
#include <algorithm>

map<string,int> graph::nl_map;
map<string,int> graph::el_map;

graph::graph(int n_vertices)
{
	num_vertices = n_vertices;
	num_edges = 0;

	vl_array = el_array = NULL;
	vl_list = NULL;

	vertex_labels = new int[num_vertices];
	for(int i = 0; i < num_vertices; i++) vertex_labels[i] = 0;

	matrix = new int*[num_vertices];
	for(int i = 0; i < num_vertices; i++) matrix[i] = new int[num_vertices];

	for(int i = 0; i < num_vertices; i++)
		for(int j = 0; j < num_vertices; j++) matrix[i][j] = 0;

	vertex_esize = new int*[num_vertices];
	for(int i = 0; i < num_vertices; i++) vertex_esize[i] = NULL;
}

void graph::DFSOrder(int v, bool* visited, int* subgraph, int subsize, vector<int>& order)
{
	visited[v] = true;
	order.push_back(subgraph[v]);

	for(int i = 0; i < subsize; i++)
		if(!visited[i] && matrix[subgraph[v]][subgraph[i]] != 0)
			DFSOrder(i, visited, subgraph, subsize, order);
}

void graph::determineVertexOrder(int* vertices, int subsize, vector<int>& order, vector<int>* disconn)
{
	order.reserve(num_vertices);
	order.clear();

	bool visited[subsize];
// mismatching vertices
	for(int i = 0; i < subsize; i++) visited[i] = false;
	for(int i = 0; i < subsize; i++){
		if(visited[i]) continue;
		if(disconn) disconn->push_back(order.size());
		DFSOrder(i, visited, vertices, subsize, order);
	}
}

graph::graph(graph& rh, int* old)
{
	vl_list = NULL; vertex_esize = NULL; // not used for now
	 vl_array = el_array = NULL;

	num_vertices = rh.num_vertices;
	num_edges = rh.num_edges;

	vertex_labels = new int[num_vertices];
	for(int i = 0; i < num_vertices; i++)
		vertex_labels[i] = rh.vertex_labels[old[i]];

	matrix = new int*[num_vertices];
	for(int i = 0; i < num_vertices; i++) matrix[i] = new int[num_vertices];

	for(int i = 0; i < num_vertices; i++)
		for(int j = 0; j < num_vertices; j++)
			matrix[i][j] = rh.matrix[old[i]][old[j]];

	vl_array = new int[numVertexLabels()];
	el_array = new int[numEdgeLabels()];

	for(unsigned i = 0; i < numVertexLabels(); i++) vl_array[i] = rh.vl_array[i];
	for(unsigned i = 0; i < numEdgeLabels(); i++) el_array[i] = rh.el_array[i];
}


graph::~graph()
{
	if(vl_array) delete [] vl_array;
	if(vl_list) delete [] vl_list;
	if(el_array) delete [] el_array;

	if(vertex_esize){
		for(int i = 0; i < num_vertices; i++) delete [] vertex_esize[i];
		delete [] vertex_esize;
	}

	delete [] vertex_labels;

	for(int i = 0; i < num_vertices; i++) delete [] matrix[i];
	delete [] matrix;
}

void graph::setVertexLabel(int vertex, string label)
{
	map<string,int>::iterator iter = nl_map.find(label);

	int numeric_label = (int)nl_map.size()+1;
	if(iter == nl_map.end()) nl_map.insert(pair<string,int>(label, numeric_label));
	else numeric_label = iter->second;

	vertex_labels[vertex] = numeric_label;
}

void graph::setEdgeLabel(int vertex1, int vertex2, string label)
{
	map<string,int>::iterator iter = el_map.find(label);

	int numeric_label = (int)el_map.size()+1; // numeric_label cannot be zero!!! ??
	if(iter == el_map.end()) el_map.insert(pair<string,int>(label, numeric_label));
	else numeric_label = iter->second;

	num_edges++;
	matrix[vertex1][vertex2] = matrix[vertex2][vertex1] = numeric_label;
}

void graph::setEdgeBond(int vertex1, int vertex2, int bond)
{
	// bond cannot be zero
	matrix[vertex1][vertex2] = matrix[vertex2][vertex1] = bond;
	num_edges++;

	// unnecessary (for numEdgeLabels() function)
	stringstream label;
	label << bond;
	map<string,int>::iterator iter = el_map.find(label.str());

	int numeric_label = (int)el_map.size()+1;
	if(iter == el_map.end()) el_map.insert(pair<string,int>(label.str(), numeric_label));
}

int graph::vertexLabel(int vertex)
{
	if(vertex == -1) return -1;
	return vertex_labels[vertex];
}

int graph::edgeLabel(int vertex1, int vertex2)
{
	if(vertex1 == -1 || vertex2 == -1) return 0;
	return matrix[vertex1][vertex2];
}

void graph::countLabels(int* vlabel, int* elabel, vector<int>& with)
{
	for(unsigned i = 0; i < numVertexLabels(); i++) vlabel[i] = 0;
	for(unsigned i = 0; i < numEdgeLabels(); i++) elabel[i] = 0;

	for(unsigned i = 0; i < with.size(); i++)
		vlabel[vertex_labels[with[i]]-1]++;

	for(unsigned i = 0; i < with.size(); i++){
		for(unsigned j = i; j < with.size(); j++)
			if(matrix[with[i]][with[j]]) elabel[matrix[with[i]][with[j]]-1]++;
	}
}

void graph::countLabels()
{
	if(vl_array != NULL) delete [] vl_array;
	if(vl_list != NULL) delete [] vl_list;
	if(el_array != NULL) delete [] el_array;

	vl_array = new int[numVertexLabels()];
	vl_list = new vector<int>[numVertexLabels()];
	el_array = new int[numEdgeLabels()];

	for(unsigned i = 0; i < numVertexLabels(); i++) vl_array[i] = 0;
	for(unsigned i = 0; i < numEdgeLabels(); i++) el_array[i] = 0;

	for(int i = 0; i < num_vertices; i++){
		vl_array[vertex_labels[i]-1]++;
		vl_list[vertex_labels[i]-1].push_back(i); // vertex label inverted list;
	}	

	for(int i = 0; i < num_vertices; i++){
		vertex_esize[i] = new int[numEdgeLabels() + 1];
		for(unsigned j = 0; j <= numEdgeLabels(); j++) vertex_esize[i][j] = 0;
		for(int j = 0; j < num_vertices; j++)
			if(matrix[i][j]) vertex_esize[i][matrix[i][j]-1]++;

		for(int j = i; j < num_vertices; j++) if(matrix[i][j]) el_array[matrix[i][j]-1]++;

		// calculate vertex degree and save it into vertex_size[i][numEdgeLabels()]
		for(unsigned j = 0; j < numEdgeLabels(); j++) vertex_esize[i][numEdgeLabels()] += vertex_esize[i][j];
	}
}


int graph::labelErrors(int* arr_l, int* arr_r, int size)
{
	int diff = 0;
	int sizeA = 0, sizeB = 0, ins = 0;
	for(int i = 0; i < size; i++){
		sizeA += arr_l[i];
		sizeB += arr_r[i];

		ins +=(arr_l[i] < arr_r[i] ? arr_l[i] : arr_r[i]);
	}

	diff = (sizeA > sizeB ? sizeA : sizeB);
	diff = diff - ins;

	return diff;
}

int graph::labelFilter(graph& rh, vector<int>& with_l, vector<int>& with_r)
{
	int v_arr_l[numVertexLabels()], v_arr_r[numVertexLabels()];
	int e_arr_l[numEdgeLabels()], e_arr_r[numEdgeLabels()];

	countLabels(v_arr_l, e_arr_l, with_l);
	rh.countLabels(v_arr_r, e_arr_r, with_r);

	int verr = labelErrors(v_arr_l, v_arr_r, numVertexLabels());
	int eerr = labelErrors(e_arr_l, e_arr_r, numEdgeLabels());

	return verr + eerr;
}

int graph::labelFilter(graph& rh)
{
	int verr = labelErrors(vl_array, rh.vl_array, numVertexLabels());
	int eerr = labelErrors(el_array, rh.el_array, numEdgeLabels());

	return verr + eerr;
}

int graph::sizeFilter(graph& rh)
{
	int vdiff = numVertices() - rh.numVertices();
	int ediff = numEdges() - rh.numEdges();

	if(vdiff < 0) vdiff = -1*vdiff;
	if(ediff < 0) ediff = -1*ediff;

	return vdiff + ediff;
}


void graph::print(bool showmatrix)
{
	cout << "# vertices: " << num_vertices << " # edges: " << num_edges << endl;

	if(!showmatrix) return;

	cout << "   ";
	for(int i = 0; i < num_vertices; i++)
		//cout << i % 10 << " ";
		cout << vertex_labels[i] << " ";
	cout << endl;

	cout << "   ";
	for(int i = 0; i < num_vertices; i++)
		cout << i % 10 << " ";
	cout << endl;

	for(int i = 0; i < num_vertices; i++){
		cout << i % 10 << ": ";
		for(int j = 0; j < num_vertices; j++)
			cout << matrix[i][j] << " ";
		cout << endl;
	}
}

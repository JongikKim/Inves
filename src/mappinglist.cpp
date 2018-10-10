#include "ged.h"
#include "mappinglist.h"
#include "simsearch.h"
#include "inves.h"

mappinglist::mappinglist(graph* from, graph* to, int mode)
{
	fr_graph = from;
	to_graph = to;

	mapped_fr = new bool[fr_graph->size()];
	for(int i = 0; i < fr_graph->size(); i++) mapped_fr[i] = false;
	
	mapped_to = new bool[to_graph->size()];
	for(int i = 0; i < to_graph->size(); i++) mapped_to[i] = false;

	vertexdist = edgedist = bridgedist = estimated = 0;
	if(mode == -1) distmode = VERTICES | FULLEDGE | BRIDGE | ESTIMATE;
	else distmode = mode;

	fr_vertices.reserve(fr_graph->size()); // max 2*fr_graph size
	to_vertices.reserve(to_graph->size()); // max 2*to_graph size

	next_from = 0;
	complete = false;
}

mappinglist::mappinglist(const mappinglist& rh)
{
	fr_graph = rh.fr_graph;
	to_graph = rh.to_graph;
	
	mapped_fr = new bool[fr_graph->size()];
	for(int i = 0; i < fr_graph->size(); i++) mapped_fr[i] = rh.mapped_fr[i];
	
	mapped_to = new bool[to_graph->size()];
	for(int i = 0; i < to_graph->size(); i++) mapped_to[i] = rh.mapped_to[i];

	vertexdist = rh.vertexdist;
	edgedist = rh.edgedist;
	bridgedist = rh.bridgedist;
	estimated = rh.estimated;

	distmode = rh.distmode;

	fr_vertices.reserve(fr_graph->size());
	to_vertices.reserve(to_graph->size());

	for(unsigned i = 0; i < rh.fr_vertices.size(); i++) fr_vertices.push_back(rh.fr_vertices[i]);
	for(unsigned i = 0; i < rh.to_vertices.size(); i++) to_vertices.push_back(rh.to_vertices[i]);

	next_from = rh.next_from;
	complete = rh.complete;
}

mappinglist::~mappinglist()
{
	delete [] mapped_fr;
	delete [] mapped_to;
}

void mappinglist::pushBack(int from, int to)
{
	fr_vertices.push_back(from);
	to_vertices.push_back(to);
	
	if(to != EPSILON)   mapped_to[to] = true;
	if(from != EPSILON){
		mapped_fr[from] = true;
		while(next_from < fr_graph->size() && mapped_fr[next_from]) next_from++;

		// added .. correct??
		if(next_from == fr_graph->size())
			for(int i = 0; i < fr_graph->size(); i++)
				if(!mapped_fr[i]){ next_from = i; break; }
	}

	if(distmode & VERTICES)	updateVertexDist(from, to);
	if(distmode & FULLEDGE)	updateEdgeDist(from, to);
	if(distmode & BRIDGE) updateBridgeDist(); 
	if(distmode & ESTIMATE)	estimateRemainingDist();
}

void mappinglist::updateBridgeDist()
{
	unsigned esize = graph::numEdgeLabels();
	int fr_elabel[esize];
	int to_elabel[esize];

	bridgedist = 0;
	
	int** fr_matrix = fr_graph->matrix;
	int** to_matrix = to_graph->matrix;
	for(unsigned i = 0; i < fr_vertices.size(); i++){
		int from = fr_vertices[i];
		int to = to_vertices[i];
		
		if(from == EPSILON && to == EPSILON) continue;

		for(unsigned i = 0; i < esize; i++)
			fr_elabel[i] = to_elabel[i] = 0;

		if(from != EPSILON)
			for(int i = 0; i < fr_graph->size(); i++)
				if(!mapped_fr[i] && fr_matrix[from][i])
					fr_elabel[fr_matrix[from][i]-1]++;
		
		if(to != EPSILON)
			for(int i = 0; i < to_graph->size(); i++)
				if(!mapped_to[i] && to_matrix[to][i])
					to_elabel[to_matrix[to][i]-1]++;
		
		bridgedist += graph::labelErrors(fr_elabel, to_elabel, esize);
	}
}

void mappinglist::estimateRemainingDist()
{
	estimated = 0;
	if(fr_vertices.back() == EPSILON || next_from == fr_graph->size()) return;
	if(getDistance() > simsearch::threshold) return;

	vector<int> unmappedTo;	getUnmappedToVertices(unmappedTo);
	vector<int> unmappedFrom; getUnmappedFromVertices(unmappedFrom);

	estimated = (unmappedFrom.size() > unmappedTo.size()) ?
		(unmappedFrom.size() - unmappedTo.size()): (unmappedTo.size() - unmappedFrom.size());
	if(getDistance() > simsearch::threshold) return;

	estimated = fr_graph->labelFilter(*to_graph, unmappedFrom, unmappedTo);
}

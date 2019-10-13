#include "inves.h"
#include "simsearch.h"

#include <algorithm>

InvesVerifier::InvesVerifier(graph* x, graph* y, loop_timer* timer)
{
	this->x = x;
	this->y = y;
	this->tau = simsearch::threshold;
	this->timer = timer;

	this->x_reorder = NULL;
	this->partition = NULL;

	source = NULL; // source will be set in incrementalPartitioning
	target = new int[x->size()]; // reserve max size
	F = new bool[y->size()];
}

InvesVerifier::~InvesVerifier()
{
	delete [] F;
	delete [] target;

	delete x_reorder;
}

bool sizeCompare(const vector<int>& v1, const vector<int>& v2)
{
	return v1.size() < v2.size();
}

int InvesVerifier::DISCONNECTED_SIZE = 1;

// remove tiny connected components (a.k.a deadends)
int InvesVerifier::removeDeadEnd(vector<int>& disconn)
{
	int tmp[psize];
	for(int i = 0; i < psize; i++) tmp[i] = source[i];

	int begin = 0, end = psize-1;

	// disconn must have at least 2 elements
	for(unsigned i = disconn.size() - 1; i > 0; i--){
		int start = disconn[i-1];
		int stop  = disconn[i];

		if(stop - start <= DISCONNECTED_SIZE){
		//	source[begin++] = tmp[start];
			for(int j = start; j < stop; j++)
				source[begin++] = tmp[j];
			continue;
		}
		while(start != stop) source[end--] = tmp[--stop];
	}

	return begin;
}

// reorder vertices of a partition for efficient subgraph isomorphism
int InvesVerifier::reorderPartition()
{
	sort(source, source + psize, vertexCompare(x));

	vector<int> order, disconn;
	x->determineVertexOrder(source, psize, order, &disconn);
	for(int i = 0; i < psize; i++) source[i] = order[i];

	disconn.push_back(psize);

	return removeDeadEnd(disconn);
}

bool InvesVerifier::rematch()
{
	if(tau >= 5) return false;

	int tmp = source[fail_position];
	for(int i = fail_position; i > 0; i--) source[i] = source[i-1];
	source[0] = tmp;

	// subgraph isomorphism test must fail within the following partition size
	int partition_size = fail_position + 1;

	vector<int> order;
	x->determineVertexOrder(source, partition_size, order);
	for(int i = 0; i < partition_size; i++) source[i] = order[i];

	for(int i = 0; i < y->size(); i++) F[i] = false;
	bridge_error = 0; fail_position = 0;
	return inducedSI(0);
}

bool InvesVerifier::incrementalPartitioning(int* vertices, int& offset)
{
	source = vertices + offset;
	psize = x->size() - offset;

	if(psize == 0) return true;

	// reorder source and remove tiny connected components (a.k.a deadends)
	int begin = reorderPartition(); 

	//collect deadends, or vertices in tiny connected components
	for(int i = 0; i < begin; i++) deadends.push_back(source[i]);
	offset += begin;

	source = vertices + offset;
	psize = x->size() - offset;

	for(int i = 0; i < y->size(); i++) F[i] = false;
	bridge_error = 0; fail_position = 0; 

	bool res = inducedSI(0);

	// re-matching mismatching partition while the size of mismatch decreases
	if(fail_position < psize){
		int prev_fail = fail_position;
		do{
			prev_fail = fail_position;
			res = rematch();
		}while(fail_position < prev_fail);
	}

	return res;
}

bool InvesVerifier::inducedSI(int d)
{
	if(d == psize){ fail_position = psize; return true;}

	int berr_saved = bridge_error;
	int u = source[d];
	vector<int>& vertex_list = y->findVertices(x->vertexLabel(u));

	for(unsigned i = 0; i < vertex_list.size(); i++){
		bridge_error = berr_saved;

		int v = vertex_list[i];
		if(F[v] || !valid(u, v, d)) continue;

		F[v] = true;
		target[d] = v;
		if(inducedSI(d+1)) return true;
		F[v] = false;
	}

	bridge_error = berr_saved;
	if(d > fail_position) fail_position = d;

	return false;
}

bool InvesVerifier::valid(int u, int v, int d)
{
	for(int i = d - 1; i >= 0; i--)
		if(x->edgeLabel(u, source[i]) != y->edgeLabel(v, target[i])) return false;

	bridge_error += x->edgeLabelErrors(u, y, v);
	if(bridge_error > 1) return false;

	return true;
}

// reorder vertices of a graph for efficient GED computation
graph* InvesVerifier::reorderGraph(int* vertices, int offset)
{
	int np = tau+1;
	for(int i = 0; i < tau+1; i++)
		if(partition[i].size() == 0){ np = i; break; }

	sort(partition, partition + np, sizeCompare);
	
	int pos = 0;
	for(int p = 0; p < np; p++){
		sort(partition[p].begin(), partition[p].end(), vertexCompare(x));

		vector<int> order;
		vector<int>::iterator iter = partition[p].begin();
		x->determineVertexOrder(&(*iter), partition[p].size(), order);
		for(unsigned i = 0; i < partition[p].size(); i++) partition[p][i] = order[i];

		std::copy(partition[p].begin(), partition[p].end(), vertices+pos);
		pos += partition[p].size();
	}

	// DFS Ordering that goes through all mismatching partitions
	vector<int> order;
	x->determineVertexOrder(vertices, offset, order);
	for(int i = offset; i < x->size(); i++) order.push_back(vertices[i]);

	vector<int>::iterator iter = order.begin();
	return new graph(*x, &(*iter));
}

bool InvesVerifier::verifyGED(int* vertices, int offset, bool partial)
{
	if(partial && psize < 0.7 * x->size()) return false;

	timer->stop(PARTIME); timer->start(GEDTIME);

	if(!x_reorder) x_reorder = reorderGraph(vertices, offset);

	mappinglist* mlist = NULL;
	if(partial){
		mlist = new mappinglist(x_reorder, y, VERTICES | FULLEDGE);
		for(int i = 0; i < psize; i++) mlist->pushBack(offset + i, target[i]);
		mlist->addDistMode(BRIDGE); mlist->updateBridgeDist();
	}

	GED ged(x_reorder, y);
	int distance = ged.getDistance(mlist);
	timer->stop(GEDTIME); timer->start(PARTIME);

	return distance <= tau;
}

bool InvesVerifier::verify(long long& cands_final)
{
	int size = x->size();

	int vertices[size]; // vertiecs of x
	for(int i = 0; i < size; i++) vertices[i] = i;

	vector<int> tmp[tau+1];
	this->partition = tmp;

	int offset = 0; // the start position of the last partition
	for(int p = 0; p < tau + 1; p++){
		if(incrementalPartitioning(vertices, offset)){
			cands_final++;

 			// if y is identified as an answer during partial verifications
			if(offset != size && verifyGED(vertices, offset, true)) return true;
			return verifyGED(vertices, offset, false);
		}

		for(int i = offset; i < offset + fail_position + 1; i++)
			partition[p].push_back(vertices[i]);

		int pos = (p == 0 ? 0 : p-1);
		for(unsigned i = 0; i < deadends.size(); i++)
			partition[pos].push_back(deadends[i]);
		deadends.clear();

		offset += fail_position + 1;
	}

	return false;
}

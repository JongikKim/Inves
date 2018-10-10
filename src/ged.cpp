#include "ged.h"
#include "simsearch.h"

GED::GED(graph* from, graph* to)
{
	fromGraph = from;
	toGraph = to;

	verbose = false;
//	verbose = true;	
}

int GED::getDistance(mappinglist* initMapping)
{
	PriorityQueue queue; 

	// assuming the size of fromGraph is at least 1 
	if(initMapping != NULL){
		initMapping->addDistMode(ESTIMATE);
		initMapping->estimateRemainingDist();
		if(initMapping->getDistance() <= simsearch::threshold) queue.push(initMapping);
		else delete initMapping;
	}
	else{
		int firstFromVertex = 0;
		for(int toVertex = EPSILON; toVertex < toGraph->size(); toVertex++){
			mappinglist* mapping = new mappinglist(fromGraph, toGraph);

			mapping->pushBack(firstFromVertex, toVertex);
			if(mapping->getDistance() <= simsearch::threshold) queue.push(mapping);
			else delete mapping;
		}
	}

	while(!queue.empty()){
		mappinglist* mapping = queue.pop();

		if(mapping->complete){
			if(verbose) mapping->print();
			queue.clear(); // delete all mapping lists in the queue
			int distance = mapping->getDistance(); delete mapping;

			return distance;
		}

		int fromVertex = mapping->getNextFromVertex();
		vector<int> unmappedTo; mapping->getUnmappedToVertices(unmappedTo);
		if(fromVertex < fromGraph->size()){
			// deletion
			mappinglist* newMapping = new mappinglist(*mapping);
			newMapping->pushBack(fromVertex, EPSILON);
			if(newMapping->getDistance() <= simsearch::threshold) queue.push(newMapping);
			else delete newMapping;
			
			// substitution
			for(vector<int>::iterator iter = unmappedTo.begin(); iter !=unmappedTo.end(); iter++){
				newMapping = new mappinglist(*mapping);
				newMapping->pushBack(fromVertex, *iter);
				if(newMapping->getDistance() <= simsearch::threshold) queue.push(newMapping);
				else delete newMapping;
			}

			delete mapping;
		}
		else{ // insertion
			for(vector<int>::iterator iter = unmappedTo.begin(); iter != unmappedTo.end(); iter++)
				mapping->pushBack(EPSILON, *iter);
			
			mapping->complete = true;
			queue.push(mapping);
		}
	}
	
	return simsearch::threshold + 1;
}

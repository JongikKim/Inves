#include "datafile.h"
#include <iostream>
#include <cstdlib>

#include <sstream>
#include <string>
#include <cstring>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>

MolFile::MolFile(char* filename)
{
	mol.open(filename);
	if(!mol.is_open()){
		cerr << "Failed to open file: " << filename << endl << flush;
		exit(1);
	}
}

MolFile::~MolFile()
{
	if(mol.is_open()) mol.close();
}

void MolFile::close()
{
	if(mol.is_open()) mol.close();
}

graph* MolFile::getNextGraph()
{
	if(!mol.good()) return NULL;

	graph* graph = parseMol();

	string line;
	while(getline(mol, line)){
		if(!mol.good()) break;
		if(line == "$$$$") break;
	}

	return graph;
}

string MolFile::trim(string str)
{
	size_t first = str.find_first_not_of(' ');
	size_t last = str.find_last_not_of(' ');

	return str.substr(first, last-first+1);
}

graph* MolFile::parseMol()
{
	string line;
	// skip first three lines
	getline(mol, line); 
	if(!mol.good()) return NULL;
	getline(mol, line); getline(mol, line);

	getline(mol, line);
	int vertexSize = stoi(line.substr(0, 3)); // stoi trims string
	int edgeSize = stoi(line.substr(3, 3)); // stoi trims string

	graph* g = new graph(vertexSize);
	for(int i = 0; i < vertexSize; i++){
		getline(mol, line);
		g->setVertexLabel(i, trim(line.substr(31, 3)));
	}

	for(int i = 0; i < edgeSize; i++){
		getline(mol, line);
		int vertex1 = stoi(line.substr(0, 3)); // stoi trims string
		int vertex2 = stoi(line.substr(3, 3)); // stoi trims string
		int numeric_edge = stoi(line.substr(6, 3));

		if(vertex1 > vertexSize || vertex2 > vertexSize){ delete g; return NULL; }
		g->setEdgeBond(vertex1 - 1, vertex2 - 1, numeric_edge);
	}

	return g;
}


GXLFile::GXLFile(char* dirname)
{
	DIR* directoryp;
	struct dirent* dirFormat;
	struct stat fileStatus;

	directoryp = opendir(dirname);
	if(directoryp == NULL){
		cerr << "fail to open directory " << dirname << endl;
		exit(1);
	}

	string dir(dirname);
	while((dirFormat = readdir(directoryp))){
		string filepath = dir + "/" + dirFormat->d_name;
		if(stat(filepath.c_str(), &fileStatus)) continue;
		if(S_ISDIR(fileStatus.st_mode)) continue;
		gxlfiles.push_back(filepath);
	}
	
	closedir(directoryp);

	current = 0;
}

GXLFile::~GXLFile()
{
	if(gxl.is_open()) gxl.close();
}

graph* GXLFile::getNextGraph()
{
	if(current >= gxlfiles.size()) return NULL;
	
	gxl.open(gxlfiles[current].c_str());

	graph* graph = parseGXL();

	gxl.close();
	current++;

	return graph;
}

#define MAX_LINE 1024
graph* GXLFile::parseGXL()
{
	char line[MAX_LINE];
	
	// skip first three lines
	gxl.getline(line, MAX_LINE); 
	gxl.getline(line, MAX_LINE); 
	gxl.getline(line, MAX_LINE); 

	vector<int> vertices;

	gxl.getline(line, MAX_LINE); 
	while(strncmp(&line[1], "node", 4) == 0){
		int i = 0;
		while(line[i++] != '>'); // skip node tag
		while(line[i++] != '>'); // skip attr tag
		while(line[i++] != '>'); // skip int tag

		int j = i;
		while(line[j] != '<') j++;
		line[j] = '\0';

		vertices.push_back(atoi(&line[i]));
		gxl.getline(line, MAX_LINE); 
	}
		

	vector<int> elabels;
	vector<pair<int,int> > edges;
	while(strncmp(&line[1], "edge", 4) == 0){
		int i = 0;
		while(line[i++] != '\"');

		int j = i;
		while(line[j] != '\"') j++;
		line[j] = '\0';

		int v1 = atoi(&line[i]);

		i = j;
		while(line[i++] != '\"');

		j = i;
		while(line[j] != '\"') j++;
		line[j] = '\0';

		int v2 = atoi(&line[i]);

		i = j;
		while(line[i] != 't' || line[i+1] != 'y' || line[i+2] != 'p' || line[i+3] != 'e') i++;
		while(line[i++] != '>'); // skip type0 tag
		while(line[i++] != '>'); // skip double tag
		j = i;
		while(line[j] != '<') j++;
		line[j] = '\0';

		elabels.push_back(atoi(&line[i]));
		edges.push_back(pair<int,int>(v1, v2));
		gxl.getline(line, MAX_LINE); 
	}

	int vertexSize = (int)vertices.size();
	int edgeSize = (int)edges.size();

	graph* g = new graph(vertexSize);
	for(int i = 0; i < vertexSize; i++){
		stringstream label;
		label << vertices[i];
		g->setVertexLabel(i, label.str());
	}

	for(int i = 0; i < edgeSize; i++){
		int vertex1 = edges[i].first;
		int vertex2 = edges[i].second;
		stringstream label;
		label << elabels[i];

		if(vertex1 > vertexSize || vertex2 > vertexSize){ delete g; return NULL; }
		g->setEdgeLabel(vertex1 - 1, vertex2 - 1, label.str());
	}

	return g;
}

SyntheticFile::SyntheticFile(char* filename)
{
	synthetic.open(filename);
	if(!synthetic.is_open()){
		cerr << "Failed to open file: " << filename << endl << flush;
		exit(1);
	}

	// eat the first line
	char line[MAX_LINE];
	synthetic.getline(line, MAX_LINE);
}

SyntheticFile::~SyntheticFile()
{
	if(synthetic.is_open()) synthetic.close();
}

void SyntheticFile::close()
{
	if(synthetic.is_open()) synthetic.close();
}


graph* SyntheticFile::getNextGraph()
{
	if(!synthetic.good()) return NULL;

	graph* graph = parseSynthetic();
	return graph;
}

graph* SyntheticFile::parseSynthetic()
{
	char line[MAX_LINE];

	vector<int> vertices;
	synthetic.getline(line, MAX_LINE); 
	while(line[0] == 'v'){
		int i = 2;
		while(line[i++] != ' ');

		vertices.push_back(atoi(&line[i]));
		synthetic.getline(line, MAX_LINE); 
	}

	vector<int> elabels;
	vector<pair<int,int> > edges;
	while(line[0] == 'e'){
		int i = 2;
		int j = i;
		while(line[j] != ' ') j++;
		line[j] = '\0';

		int v1 = atoi(&line[i]);

		i = j+1;
		while(line[j] != ' ') j++;
		line[j] = '\0';

		int v2 = atoi(&line[i]);

		i = j+1;
		elabels.push_back(atoi(&line[i]));
		edges.push_back(pair<int,int>(v1, v2));
		synthetic.getline(line, MAX_LINE); 
	}

	int vertexSize = (int)vertices.size();
	int edgeSize = (int)edges.size();

	graph* g = new graph(vertexSize);
	for(int i = 0; i < vertexSize; i++){
		stringstream label;
		label << vertices[i];
		g->setVertexLabel(i, label.str());
	}

	for(int i = 0; i < edgeSize; i++){
		int vertex1 = edges[i].first;
		int vertex2 = edges[i].second;
		stringstream label;
		label << elabels[i];

		if(vertex1 > vertexSize || vertex2 > vertexSize){ delete g; return NULL; }
		g->setEdgeLabel(vertex1, vertex2, label.str());
	}

	return g;
}

#ifndef __GRAPHFILE_H
#define __GRAPHFILE_H

#include "graph.h"
#include <string>
#include <fstream>

using namespace std;

class DataFile
{
public:
	virtual ~DataFile(){}
	virtual graph* getNextGraph() = 0;
	virtual void close(){}
};


class MolFile : public DataFile
{
private:
	ifstream mol;

public:
	MolFile(char* filename);
	~MolFile();
	void close();

public:
	graph* getNextGraph();

private:
	string trim(string str);
	graph* parseMol();
};

class GXLFile : public DataFile
{
private:
	ifstream gxl;

	unsigned current;
	vector<string> gxlfiles;

public:
	GXLFile(char* filename);
	~GXLFile();

public:
	graph* getNextGraph();

private:
	graph* parseGXL();
};

// for reading synthetic graphs generated from GraphGen
class SyntheticFile : public DataFile
{
private:
	ifstream synthetic;

public:
	SyntheticFile(char* filename);
	~SyntheticFile();
	
	void close();

public:
	graph* getNextGraph();

private:
	graph* parseSynthetic();
};

#endif //__GRAPHFILE_H

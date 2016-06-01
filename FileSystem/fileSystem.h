#pragma once
#include <stdlib.h>
#include <conio.h>
#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include "monitor.h"
using namespace std;

struct discInfo {
	int size;
	int fileCount;
	int nonDataSize;
	int freeBlocks;
	char name[16];
};

struct fileInfo {
	char name[24];
	int size;
	int address;
};

class FileSystem : Monitor
{
private:
	bool isOpen = false;
	Monitor _locker;

public:
	FileSystem();
	~FileSystem();

	int getSize(char * f);
	int doesFileExist(FILE * disc, char* filename);
	void gotoFat(FILE * disc);
	void printFAT();
	int createDisc(int size);
	int deleteDisc();
	int getBlocksCount(int filesize);
	int copyToDisc(char* filename);
	inline void gotoHeaders(FILE * disc);
	void gotoBeggining(FILE * disc);
	discInfo getDiscInfo(FILE * disc);
	inline bool doesFileExistOutside(const std::string& name);
	int nextFatCell(FILE * disc, int currentCell);
	void clearFatCell(FILE * disc, int currentCell);
	int copyOutside(char* filename, char* outputFilename);
	int deleteFile(char* filename);
	void printCatalogue();
	void printDiscInfo();
	void openDisc();
	void leaveDisc();
	void monitorTest();
};


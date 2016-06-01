#pragma once
#include <stdlib.h>
#include <conio.h>
#include <fstream>
#include <iostream>
#include <vector>
#include <string>
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

int getSize(char * f);
int doesFileExist(FILE * disc, char* filename);
void gotoFat(FILE * disc);
void printFAT();
int createDisc(int size);
int deleteDisc();
int getBlocksCount();
int copyToDisc(char* filename);
inline void gotoHeaders();
void gotoBeggining();
discInfo getDiscInfo();
inline bool doesFileExistOutside(const std::string& name);
int nextFatCell(FILE * disc, int currentCell);
void clearFatCell(FILE * disc, int currentCell);
int copyOutside(char* filename, char* outputFilename);
int deleteFile(char* filename);
void printCatalogue();
void printDiscInfo();

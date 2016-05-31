﻿#include <stdlib.h>
#include <conio.h>
#include <fstream>
#include <iostream>
#include <list>
using namespace std;

#define BLOCK_SIZE 2048
#define MAX_FILES 63
//pierwszy blok (nadrzedny)
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

int getSize(char * f) {
	ifstream file(f, ios::binary | ios::ate);
	return file.tellg();
}

struct blockNumber {
	int number;
};

FILE * theDISC;

//size - w KB
int createDisc(int size) {
	discInfo infoBlock;
	int blocks;
	int	fatBlocks;
	int freeSpaceBlocks;

	cout << "NOWY DYSK" << endl;

	//<size> KB / BLOCK_SIZE = ilosc bloków na dane
	freeSpaceBlocks = size * 1024 / BLOCK_SIZE;

	if (freeSpaceBlocks > 0)
	{
		blocks = freeSpaceBlocks;
	}
	else
		blocks = 0;

	cout << "Ilosc blokow na pliki: " << blocks << endl;

	fatBlocks = sizeof(int)* (blocks) / BLOCK_SIZE + 1;
	cout << "Ilosc blokow FAT: " << fatBlocks << endl;

	infoBlock.size = blocks;
	infoBlock.fileCount = 0;
	infoBlock.freeBlocks = blocks;
	infoBlock.nonDataSize = BLOCK_SIZE * (fatBlocks + 1);

	//open/make new file
	theDISC = fopen("VirtualDisc", "wb+");
	//wyzeruj cały FAT
	char emptyBlock[BLOCK_SIZE];
	fileInfo emptyFileInfo;
	emptyFileInfo.size = 0;
	//write info block
	fwrite(&infoBlock, sizeof(infoBlock), 1, theDISC);

	for (int i = 0; i < (BLOCK_SIZE - sizeof(discInfo)) / sizeof(fileInfo); i++)
		fwrite(&emptyFileInfo, sizeof(fileInfo), 1, theDISC);

	//fseek(theDISC, BLOCK_SIZE, SEEK_SET);

	int* fatTable = new int[blocks];
	for (int i = 0; i < blocks; i++)
		fatTable[i] = 0;
	//write fat table block
	fwrite(&fatTable, sizeof(fatTable), 1, theDISC);
	fseek(theDISC, BLOCK_SIZE * (1 + fatBlocks), SEEK_SET);
	//size for blocks
	for (int i = 0; i < blocks; i++) {
		fwrite(&emptyBlock, BLOCK_SIZE, 1, theDISC);
	}

	fclose(theDISC);
	return 0;
}

int deleteDisc() {
	return remove("VirtualDisc");
}

int copyToDisc(char* filename) {

	theDISC = fopen("VirtualDisc", "rb+");

	int fileSize = getSize(filename);
	discInfo info;

	fread(&info, sizeof(discInfo), 1, theDISC);

	if (info.fileCount >= MAX_FILES)
		return 0;

	int blockToWrite = fileSize / BLOCK_SIZE;
	if (fileSize % BLOCK_SIZE != 0)
		fileSize += 1;

	if (blockToWrite > info.freeBlocks)
		return 0;

	//no space left
	//przesunięcie do tablicy z plikami
	//header 

	//znajdowanie miejsca w facie
	fseek(theDISC, BLOCK_SIZE, SEEK_SET);

	int firstCellAddress;
	int previousCellAddress;
	int cellAddress = 1;
	bool firstCell = true;
	list<int> fileAddresses;

	for (int i = 0; i < blockToWrite; i++)
	{
		while (cellAddress != 0)
		{
			fread(&cellAddress, sizeof(int), 1, theDISC);
		}
		
		fileAddresses.push_back(i);
		if (firstCell)
		{
			firstCellAddress = i;
			firstCell = false;
		}
		else
		{
			if (i == blockToWrite - 1)
			{
				int lastCell = -1;

				fseek(theDISC, BLOCK_SIZE + previousCellAddress * sizeof(int), SEEK_SET);
				fwrite(&lastCell, sizeof(int), 1, theDISC);
				fseek(theDISC, BLOCK_SIZE + i * sizeof(int), SEEK_SET);
				fileAddresses.pop_back();
				fileAddresses.push_back(lastCell);
			}
			else
			{
				fseek(theDISC, BLOCK_SIZE + previousCellAddress * sizeof(int), SEEK_SET);
				fwrite(&previousCellAddress, sizeof(int), 1, theDISC);
				fseek(theDISC, BLOCK_SIZE + i * sizeof(int), SEEK_SET);
			}
		}
		//place file
		previousCellAddress = i;
	}

	FILE * newFile = fopen(filename, "rb+");
	//make file header
	fseek(theDISC, sizeof(fileInfo), SEEK_SET);
	fileInfo fileHeader;

	for (int i = 0; i < MAX_FILES; i++)
	{
		fread(&fileHeader, sizeof(fileInfo), 1, theDISC);
		if (fileHeader.size == 0)
		{
			//mozna zapisac plik
			int sizeOfHeader = sizeof(fileHeader);
			sizeOfHeader = -sizeOfHeader;

			fseek(theDISC, sizeOfHeader, SEEK_CUR);
			//dodanie info o pliku
			fileInfo fileData;
			fileData.address = firstCellAddress;
			fileData.size = getSize(filename);
			strcpy(fileData.name, filename);
			fwrite(&fileData, sizeof(fileInfo), 1, theDISC);
			break;
		}
	}

	char buffer[BLOCK_SIZE];
	//write file
	for (int i = 0; i < blockToWrite; i++)
	{
		int blockAddress = fileAddresses.front();
		fileAddresses.pop_front();
		
		if (blockAddress != -1)
		{
			fread(&buffer, BLOCK_SIZE, 1, theDISC);
			fseek(theDISC, BLOCK_SIZE * (blockAddress + info.nonDataSize), SEEK_SET);
			fwrite(&buffer, BLOCK_SIZE, 1, theDISC);
		}
		else
		{
			int finalSizeToWrite = getSize(filename) % BLOCK_SIZE;

			fread(&buffer, finalSizeToWrite, 1, theDISC);
			fseek(theDISC, BLOCK_SIZE * (blockAddress + info.nonDataSize), SEEK_SET);
			fwrite(&buffer, finalSizeToWrite, 1, theDISC);
		}
	}

	fclose(newFile);
	fclose(theDISC);
	return 1;
}


void main() {
	int a;
	cin >> a;
	createDisc(a);
	cout << getSize("VirtualDisc");
	//copyToDisc("test1");

	_getch();
	deleteDisc();
}
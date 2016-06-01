#include <stdlib.h>
#include <conio.h>
#include <fstream>
#include <iostream>
#include <vector>
#include <string>
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

int doesFileExist(FILE * disc, char* filename)
{
	fseek(disc, sizeof(discInfo), SEEK_SET);
	fileInfo info;

	for (int i = 0; i < MAX_FILES; i++)
	{
		fread(&info, sizeof(fileInfo), 1, disc);
		if (strcmp(info.name, filename) == 0)
		{
			cout << "File exist" << endl;
			return i;
		}
	}
	return -1;
}

struct blockNumber {
	int number;
};

FILE * theDISC;

void printFAT() {
	cout << endl << "=Disc Info=" << endl;
	FILE * theDISC = fopen("VirtualDisc", "rb+");
	int m;
	fileInfo f;
	discInfo inf;
	fread(&inf, sizeof(discInfo), 1, theDISC);
	cout << inf.size << " " << inf.fileCount << " " << inf.freeBlocks << " " << inf.nonDataSize << endl;

	for (int i = 0; i < 63; i++) {
		fread(&f, sizeof(fileInfo), 1, theDISC);
		cout << f.size << " " << f.address << " " << f.name << endl;
	}

	for (int i = 0; i < BLOCK_SIZE / sizeof(int); i++) {
		fread(&m, sizeof(int), 1, theDISC);
		if (m <= 9 && m >= 0)
			cout << " ";
		cout << m;
		if (i % 32 == 31)
			cout << endl;
	}
	fclose(theDISC);
}

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
	infoBlock.nonDataSize = (fatBlocks + 1);

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

	/*int* fatTable = new int[blocks];
	for (int i = 0; i < blocks; i++)
		fatTable[i] = 0;*/
	//write fat table block
	int zero = 0;

	fseek(theDISC, BLOCK_SIZE, SEEK_SET);
	for (int i = 0; i < blocks; i++)
		fwrite(&zero, sizeof(int), 1, theDISC);

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
	{
		cout << "Brak miejsca na pliki.\n";
		return 0;
	}


	int blockToWrite = fileSize / BLOCK_SIZE;
	if (fileSize % BLOCK_SIZE != 0)
		blockToWrite += 1;

	if (blockToWrite > info.freeBlocks)
	{
		cout << "Brak miejsca na pliki.\n";
		return 0;
	}

	if (doesFileExist(theDISC, filename) >= 0)
	{
		cout << "Plik o podanej nazwie już istnieje.\n";
		return 0;
	}

	cout << "liczba blokow do zapisu:" << blockToWrite << endl;
	//no space left
	//przesunięcie do tablicy z plikami
	//header 

	//znajdowanie miejsca w facie
	int firstCellAddress;
	int previousCellAddress = 0;
	int cellAddress = 1;
	bool firstCell = true;
	vector<int> fileAddresses;

	int blocksLeft = blockToWrite;

	fseek(theDISC, BLOCK_SIZE, SEEK_SET);
	for (int i = 0; i < MAX_FILES; i++)
	{
		fread(&cellAddress, sizeof(int), 1, theDISC);
		cout << previousCellAddress;
		cout << endl;
		if (cellAddress == 0)
		{
			fileAddresses.push_back(i);
			if (firstCell)
			{
				firstCellAddress = i;
				firstCell = false;
			}
			else
			{
				if (blocksLeft == 0)
				{
					int lastCell = -1;

					fseek(theDISC, BLOCK_SIZE + previousCellAddress * sizeof(int), SEEK_SET);
					fwrite(&lastCell, sizeof(int), 1, theDISC);
					fseek(theDISC, BLOCK_SIZE + i * sizeof(int), SEEK_SET);

					break;
				}
				else
				{
					fseek(theDISC, BLOCK_SIZE + previousCellAddress * sizeof(int), SEEK_SET);
					fwrite(&i, sizeof(int), 1, theDISC);
					fseek(theDISC, BLOCK_SIZE + i * sizeof(int), SEEK_SET);
				}
			}
			//place file
			previousCellAddress = i;
			blocksLeft--;
		}
	}
	printFAT();

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
			fileData.size = fileSize;
			strcpy(fileData.name, filename);
			fwrite(&fileData, sizeof(fileInfo), 1, theDISC);
			cout << "Zapisano header "<< fileData.name <<" na miejscu: " << i << endl;
			break;
		}
	}

	for (unsigned i = 0; i<fileAddresses.size(); ++i)
		std::cout << ' ' << fileAddresses[i];
	std::cout << '\n';

	char buffer[BLOCK_SIZE];
	//write file
	for (int i = 0; i < blockToWrite; i++)
	{
		int blockAddress = fileAddresses.front();
		fileAddresses.erase(fileAddresses.begin());
		cout << blockAddress;

		if (fileAddresses.size() != 0)
		{
			fread(&buffer, BLOCK_SIZE, 1, newFile);
			fseek(theDISC, BLOCK_SIZE * (blockAddress + info.nonDataSize), SEEK_SET);
			fwrite(&buffer, BLOCK_SIZE, 1, theDISC);
		}
		else
		{
			int finalSizeToWrite = fileSize % BLOCK_SIZE;

			fread(&buffer, finalSizeToWrite, 1, newFile);
			fseek(theDISC, BLOCK_SIZE * (blockAddress + info.nonDataSize), SEEK_SET);
			fwrite(&buffer, finalSizeToWrite, 1, theDISC);
			break;
		}
	}
	
	info.fileCount++;
	info.freeBlocks = info.freeBlocks - blockToWrite;
	fseek(theDISC, 0, SEEK_SET);
	fwrite(&info, sizeof(discInfo), 1, theDISC);

	fclose(newFile);
	fclose(theDISC);
	return 1;
}

void gotoHeaders(FILE * disc)
{
	fseek(theDISC, sizeof(discInfo), SEEK_SET);
}

void gotoBeggining(FILE * disc)
{
	fseek(theDISC, 0, SEEK_SET);
}

void gotoFat(FILE * disc)
{
	fseek(theDISC, BLOCK_SIZE, SEEK_SET);
}

discInfo getDiscInfo(FILE * disc)
{
	discInfo info;
	fseek(disc, 0, SEEK_SET);
	fread(&info, sizeof(discInfo), 1, disc);
	return info;
}

inline bool doesFileExistOutside(const std::string& name) {
	struct stat buffer;
	return (stat(name.c_str(), &buffer) == 0);
}

int nextFatCell(FILE * disc, int currentCell)
{
	int cellAddress;
	fseek(disc, BLOCK_SIZE + currentCell*sizeof(int), SEEK_SET);
	fread(&cellAddress, sizeof(int), 1, disc);
	
	return cellAddress;
}

int copyOutside(char* filename, char* outputFilename)
{
	if (doesFileExistOutside(outputFilename))
	{
		cout << "Plik o podanej nazwie juz istnieje na komputerze!";
		return 0;
	}

	theDISC = fopen("VirtualDisc", "rb+");

	discInfo info = getDiscInfo(theDISC);
	int headerAddress = doesFileExist(theDISC, filename);

	if (headerAddress == -1)
	{
		cout << "Nie ma pliku o podanej nazwie na wirtualnym dysku.";
		return 0;
	}

	FILE * newFile = fopen(outputFilename, "wb+");

	gotoHeaders(theDISC);
	//goto right header
	fileInfo fileHeader;
	fseek(theDISC, sizeof(discInfo) + headerAddress * sizeof(fileInfo), SEEK_SET);
	fread(&fileHeader, sizeof(fileInfo), 1, theDISC);

	int fatAddress = fileHeader.address;

	int count = 1;
	while (1)
	{
		char buffer[BLOCK_SIZE];
		int nextCell = nextFatCell(theDISC, fatAddress);

		if (nextCell == -1)
		{
			int sizeToCopy = fileHeader.size % BLOCK_SIZE;

			fseek(theDISC, BLOCK_SIZE * (fatAddress + info.nonDataSize), SEEK_SET);
			fread(&buffer, sizeToCopy, 1, theDISC);
			fwrite(&buffer, sizeToCopy, 1, newFile);
			break;
		}
		else
		{
			fseek(theDISC, BLOCK_SIZE * (fatAddress + info.nonDataSize), SEEK_SET);
			fread(&buffer, BLOCK_SIZE, 1, theDISC);
			fwrite(&buffer, BLOCK_SIZE, 1, newFile);
		}
		fatAddress = nextCell;
	}

	fclose(newFile);
	fclose(theDISC);
}

void main() {
	//createDisc(a);
	while (1)
	{
		cout << "\nSystem plikow.\nWybierz polecenie:\n1. Stworz dysk.\n2. Skopiuj plik na dysk.\n" <<
			"3. Usun dysk.\n4. Skopiuj plik na komputer.\n5. Wyswietl tablice FAT."<< endl;

		int a;
		cin >> a;

		switch (a)
		{
			case (1):
			{
				cout << "Podaj pojemnosc w kB: ";
				int size;
				cin >> size;
				createDisc(size);
				break;
			}
			case (2):
			{
				cout << "Podaj nazwe pliku: ";
				std::cin.clear();
				string input;
				char filename[24];
				cin >> input;
				strcpy(filename, input.c_str());
				copyToDisc(filename);
				break;
			}
			case (3):
			{
				deleteDisc();
			}
			case (4):
			{
				cout << "Podaj nazwe pliku: ";
				std::cin.clear();
				string input;
				char filename[24];
				cin >> input;
				strcpy(filename, input.c_str());

				cout << "Podaj nazwe pliku docelowego: ";
				std::cin.clear();
				char outputName[24];
				cin >> input;
				strcpy(outputName, input.c_str());

				copyOutside(filename, outputName);
				break;
			}
			case (5):
			{
				printFAT();
			}
			default:
				break;
		}
	}
}

#include "fileSystem.h"

#define BLOCK_SIZE 2048
#define MAX_FILES 63

FileSystem::FileSystem() {}
FileSystem::~FileSystem() {}

void FileSystem::openDisc()
{
	_locker.enter();
}

void FileSystem::leaveDisc()
{
	_locker.leave();
}

void FileSystem::monitorTest()
{
	openDisc();
	Sleep(4000);
	leaveDisc();
}

int FileSystem::getSize(char * f) 
{
	ifstream file(f, ios::binary | ios::ate);
	return file.tellg();
}

int FileSystem::doesFileExist(FILE * disc, char* filename)
{
	fseek(disc, sizeof(discInfo), SEEK_SET);
	fileInfo info;

	for (int i = 0; i < MAX_FILES; i++)
	{
		fread(&info, sizeof(fileInfo), 1, disc);
		if (strcmp(info.name, filename) == 0)
		{
			cout << "File exist " << i << endl;
			return i;
		}
	}
	return -1;
}

void FileSystem::gotoFat(FILE * disc)
{
	fseek(disc, BLOCK_SIZE, SEEK_SET);
}

bool FileSystem::doesDiscExist(FILE * disc)
{
	if (disc != NULL)
		return true;
	else
		return false;
}

void FileSystem::printFAT() {
	openDisc();
	FILE * theDISC = fopen("VirtualDisc", "rb+");

	if (!doesDiscExist(theDISC))
	{
		cout << "Dysk nie istnieje" << endl;
		return;
	}

	discInfo info = getDiscInfo(theDISC);
	int m;
	
	gotoFat(theDISC);
	for (int i = 0; i < info.size; i++) {
		fread(&m, sizeof(int), 1, theDISC);
		if (m <= 9 && m >= 0)
			cout << " ";
		cout << m;
		if (i % 32 == 31)
			cout << endl;
	}

	cout << endl;

	fclose(theDISC);
	leaveDisc();
}

int FileSystem::createDisc(int size) {

	FILE * theDISC;
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
	openDisc();
	theDISC = fopen("VirtualDisc", "wb+");
	//wyzeruj ca³y FAT
	char emptyBlock[BLOCK_SIZE];
	fileInfo emptyFileInfo;
	emptyFileInfo.size = 0;
	//write info block
	fwrite(&infoBlock, sizeof(infoBlock), 1, theDISC);

	for (int i = 0; i < (BLOCK_SIZE - sizeof(discInfo)) / sizeof(fileInfo); i++)
		fwrite(&emptyFileInfo, sizeof(fileInfo), 1, theDISC);

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
	leaveDisc();

	return 1;
}

int FileSystem::deleteDisc() {
	return remove("VirtualDisc");
}

int FileSystem::getBlocksCount(int filesize)
{
	int blocksCount = filesize / BLOCK_SIZE;
	if (filesize % BLOCK_SIZE != 0)
		blocksCount += 1;

	return blocksCount;
}

int FileSystem::copyToDisc(char* filename) {

	FILE * theDISC;

	openDisc();
	theDISC = fopen("VirtualDisc", "rb+");

	if (!doesDiscExist(theDISC))
	{
		cout << "Dysk nie istnieje" << endl;
		return 0;
	}

	int fileSize = getSize(filename);
	discInfo info;

	fread(&info, sizeof(discInfo), 1, theDISC);

	if (info.fileCount >= MAX_FILES)
	{
		cout << "Brak miejsca na pliki.\n";
		leaveDisc();
		return 0;
	}

	int blockToWrite = getBlocksCount(fileSize);

	if (blockToWrite > info.freeBlocks)
	{
		cout << "Brak miejsca na pliki.\n";
		leaveDisc();
		return 0;
	}

	if (doesFileExist(theDISC, filename) >= 0)
	{
		cout << "Plik o podanej nazwie ju¿ istnieje.\n";
		leaveDisc();
		return 0;
	}

	cout << "liczba blokow do zapisu:" << blockToWrite << endl;
	//no space left
	//przesuniêcie do tablicy z plikami
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
				fseek(theDISC, BLOCK_SIZE + (i + 1) * sizeof(int), SEEK_SET);
			}
			else
			{
				if (blocksLeft == 0)
				{
					int lastCell = -1;

					fseek(theDISC, BLOCK_SIZE + previousCellAddress * sizeof(int), SEEK_SET);
					fwrite(&lastCell, sizeof(int), 1, theDISC);
					fseek(theDISC, BLOCK_SIZE + (i + 1) * sizeof(int), SEEK_SET);

					break;
				}
				else
				{
					fseek(theDISC, BLOCK_SIZE + previousCellAddress * sizeof(int), SEEK_SET);
					fwrite(&i, sizeof(int), 1, theDISC);
					fseek(theDISC, BLOCK_SIZE + (i + 1) * sizeof(int), SEEK_SET);
				}
			}
			previousCellAddress = i;
			blocksLeft--;
		}
	}

	FILE * newFile = fopen(filename, "rb+");
	//make file header
	fseek(theDISC, sizeof(discInfo), SEEK_SET);
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
			//cout << "Zapisano header " << fileData.name << " na miejscu: " << i << endl;
			break;
		}
	}

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
	leaveDisc();

	return 1;
}

inline void FileSystem::gotoHeaders(FILE * disc)
{
	fseek(disc, sizeof(discInfo), SEEK_SET);
}

void FileSystem::gotoBeggining(FILE * disc)
{
	fseek(disc, 0, SEEK_SET);
}

discInfo FileSystem::getDiscInfo(FILE * disc)
{
	discInfo info;
	fseek(disc, 0, SEEK_SET);
	fread(&info, sizeof(discInfo), 1, disc);
	return info;
}

inline bool FileSystem::doesFileExistOutside(const std::string& name) {
	struct stat buffer;
	return (stat(name.c_str(), &buffer) == 0);
}

int FileSystem::nextFatCell(FILE * disc, int currentCell)
{
	int cellAddress;
	fseek(disc, BLOCK_SIZE + currentCell * sizeof(int), SEEK_SET);
	fread(&cellAddress, sizeof(int), 1, disc);

	return cellAddress;
}

void FileSystem::clearFatCell(FILE * disc, int currentCell)
{
	int emptyCell = 0;
	fseek(disc, BLOCK_SIZE + currentCell * sizeof(int), SEEK_SET);
	fwrite(&emptyCell, sizeof(int), 1, disc);
}

int FileSystem::copyOutside(char* filename, char* outputFilename)
{
	FILE * theDISC;

	if (doesFileExistOutside(outputFilename))
	{
		cout << "Plik o podanej nazwie juz istnieje na komputerze!";
		return 0;
	}

	openDisc();
	theDISC = fopen("VirtualDisc", "rb+");

	if (!doesDiscExist(theDISC))
	{
		cout << "Dysk nie istnieje" << endl;
		return 0;
	}

	discInfo info = getDiscInfo(theDISC);
	int headerAddress = doesFileExist(theDISC, filename);

	if (headerAddress == -1)
	{
		cout << "Nie ma pliku o podanej nazwie na wirtualnym dysku.";
		leaveDisc();
		return 0;
	}

	FILE * newFile = fopen(outputFilename, "wb+");

	gotoHeaders(theDISC);
	//goto right header
	fileInfo fileHeader;
	fseek(theDISC, sizeof(discInfo) + headerAddress * sizeof(fileInfo), SEEK_SET);
	fread(&fileHeader, sizeof(fileInfo), 1, theDISC);

	int fatAddress = fileHeader.address;

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
	leaveDisc();

	return 1;
}

int FileSystem::deleteFile(char* filename)
{
	openDisc();
	FILE * theDISC = fopen("VirtualDisc", "rb+");

	if (!doesDiscExist(theDISC))
	{
		cout << "Dysk nie istnieje" << endl;
		return 0;
	}

	int fatAddress;
	int fileAddress = doesFileExist(theDISC, filename);
	fileInfo fileHeader;
	fileInfo emptyHeader;
	discInfo info = getDiscInfo(theDISC);
	emptyHeader.size = 0;
	if (fileAddress == -1)
	{
		cout << "Plik o podanej nazwie nie istnieje\n";
		leaveDisc();
		return 0;
	}

	//read address and delete header
	//gotoHeaders(theDISC);
	fseek(theDISC, sizeof(discInfo) + sizeof(fileInfo) * fileAddress, SEEK_SET);
	fread(&fileHeader, sizeof(fileInfo), 1, theDISC);
	fseek(theDISC, sizeof(discInfo) + sizeof(fileInfo) * fileAddress, SEEK_SET);
	fatAddress = fileHeader.address;
	fwrite(&emptyHeader, sizeof(fileInfo), 1, theDISC);

	//delete fat and data
	while (1)
	{
		char buffer[BLOCK_SIZE];
		int nextCell = nextFatCell(theDISC, fatAddress);

		if (nextCell == -1)
		{
			int sizeLeft = fileHeader.size % BLOCK_SIZE;

			fseek(theDISC, BLOCK_SIZE * (fatAddress + info.nonDataSize), SEEK_SET);
			fwrite(&buffer, sizeLeft, 1, theDISC);
			clearFatCell(theDISC, fatAddress);
			break;
		}
		else
		{
			fseek(theDISC, BLOCK_SIZE * (fatAddress + info.nonDataSize), SEEK_SET);
			fwrite(&buffer, BLOCK_SIZE, 1, theDISC);
			clearFatCell(theDISC, fatAddress);
		}
		fatAddress = nextCell;
	}
	//update disc info header
	info.fileCount--;
	info.freeBlocks += getBlocksCount(fileHeader.size);
	fseek(theDISC, 0, SEEK_SET);
	fwrite(&info, sizeof(discInfo), 1, theDISC);

	fclose(theDISC);
	leaveDisc();

	return 1;
}

void FileSystem::printCatalogue()
{
	openDisc();
	FILE * theDISC = fopen("VirtualDisc", "rb+");

	if (!doesDiscExist(theDISC))
	{
		cout << "Dysk nie istnieje" << endl;
		return;
	}

	fseek(theDISC, sizeof(discInfo), SEEK_SET);

	for (int i = 0; i < MAX_FILES; i++)
	{
		fileInfo fileHeader;
		fread(&fileHeader, sizeof(fileInfo), 1, theDISC);

		if (fileHeader.size != 0)
		{
			cout << fileHeader.name << " ";
		}
	}
	cout << "\n";

	fclose(theDISC);
	leaveDisc();
}

void FileSystem::printDiscInfo()
{
	openDisc();
	FILE * theDISC = fopen("VirtualDisc", "rb+");

	if (!doesDiscExist(theDISC))
	{
		cout << "Dysk nie istnieje" << endl;
		return;
	}

	discInfo info;

	//fseek(theDISC, 0, SEEK_SET);
	fread(&info, sizeof(discInfo), 1, theDISC);

	cout << "Liczba plikow: " << info.fileCount << "\n";
	cout << "Wolne bloki pamieci: " << info.freeBlocks << "\n";
	cout << "Wolna pamiec: " << info.freeBlocks*BLOCK_SIZE << "\n";
	cout << "Pojemnosc: " << info.size*BLOCK_SIZE << "\n";
	cout << "\n";

	fclose(theDISC);
	leaveDisc();
}

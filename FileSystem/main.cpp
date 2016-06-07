#include "fileSystem.h"
#define THREAD_COUNT 4
#define COPIER_COUNT 2
#define DELETER_COUNT 2

using namespace std;
FileSystem fileSystem;

void main() {
	
	while (1)
	{
		cout << "\nSystem plikow.\nWybierz polecenie:\n1. Stworz dysk.\n2. Skopiuj plik na dysk.\n" <<
			"3. Usun dysk.\n4. Skopiuj plik na komputer.\n5. Wyswietl tablice FAT.\n6. Usun plik.\n7. Pokaz katalog."<< 
			"\n8. Wyswietl informacje o dysku." << endl;

		int a;
		cin >> a;

		switch (a)
		{
			case (1):
			{
				cout << "Podaj pojemnosc w kB: ";
				int size;
				cin >> size;
				fileSystem.createDisc(size);
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
				fileSystem.copyToDisc(filename);
				break;
			}
			case (3):
			{
				fileSystem.deleteDisc();
				break;
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

				fileSystem.copyOutside(filename, outputName);
				break;
			}
			case (5):
			{
				fileSystem.printFAT();
				break;
			}
			case (6):
			{
				cout << "Podaj nazwe pliku: ";
				std::cin.clear();
				string input;
				char filename[24];
				cin >> input;
				strcpy(filename, input.c_str());

				fileSystem.deleteFile(filename);
				break;
			}
			case (7):
			{
				fileSystem.printCatalogue();
				break;
			}
			case (8):
			{
				fileSystem.printDiscInfo();
				break;
			}
			case (9):
			{
				fileSystem.monitorTest();
				break;
			}
			case (10):
			{
				testMultiThread();
				break;
			}
			default:
				break;
		}
	}
}

void fileCopier()
{
	for (int i = 0; i < 3; i++)
	{
		cout << "proba skopiowania test1" << endl;
		fileSystem.copyToDisc("test1");
		fileSystem.printCatalogue();

		cout << "proba skopiowania test2" << endl;
		fileSystem.copyToDisc("test2");
		fileSystem.printCatalogue();
	}
}

void fileDeleter()
{
	for (int i = 0; i < 3; i++)
	{
		cout << "proba usuniecia test1" << endl;
		fileSystem.deleteFile("test1");
		fileSystem.printCatalogue();

		cout << "proba usuniecia test2" << endl;
		fileSystem.deleteFile("test2");
		fileSystem.printCatalogue();
	}
}

int testMultiThread()
{
	int i;
#ifdef _WIN32
	HANDLE tid[THREAD_COUNT];
	DWORD id;

	//utworz watek producenta
	for (i = 0; i < COPIER_COUNT; i++)
		tid[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)fileCopier, (void*)i, 0, &id);
	//utworz watki konsumentow
	for (i = COPIER_COUNT; i < COPIER_COUNT + DELETER_COUNT; i++)
		tid[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)fileDeleter, (void*)i, 0, &id);

	for (i = 0; i <= THREAD_COUNT; i++)
		WaitForSingleObject(tid[i], INFINITE);

#else
	pthread_t tid[THREAD_COUNT];

	//utworz watek producenta
	for (i = 0; i < COPIER_COUNT; i++)
		pthread_create(&(tid[i]), NULL, fileCopier, (void*)i);

	//utworz watki konsumentow
	for (i = COPIER_COUNT; i < THREAD_COUNT; i++)
		pthread_create(&(tid[i]), NULL, konsument, (void*)i);


	//czekaj na zakonczenie watkow
	for (i = 0; i < THREAD_COUNT; i++)
		pthread_join(tid[i], (void **)NULL);
#endif
	return 0;
}

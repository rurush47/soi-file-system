#include "fileSystem.h"
using namespace std;
FileSystem fileSystem;

void main() {
	//fileSystem.monitorTest();
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
			default:
				break;
		}
	}
}

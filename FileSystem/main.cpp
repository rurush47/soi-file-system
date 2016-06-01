#include "fileSystem.h"
using namespace std;

void main() {
	//createDisc(a);
	while (1)
	{
		cout << "\nSystem plikow.\nWybierz polecenie:\n1. Stworz dysk.\n2. Skopiuj plik na dysk.\n" <<
			"3. Usun dysk.\n4. Skopiuj plik na komputer.\n5. Wyswietl tablice FAT.\n6. Usun plik.\n7. Pokaż katalog."<< 
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

				copyOutside(filename, outputName);
				break;
			}
			case (5):
			{
				printFAT();
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

				deleteFile(filename);
				break;
			}
			case (7):
			{
				printCatalogue();
				break;
			}
			case (8):
			{
				printDiscInfo();
				break;
			}
			default:
				break;
		}
	}
}

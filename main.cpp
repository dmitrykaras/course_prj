#include "SearchEngine.h"
#include "Utils.h"

using namespace std;

int main() {
    SetConsoleOutputCP(1251);
    SetConsoleCP(1251);

    SearchEngine engine(2);
    int choice;

    while (true) {
        cout << "\n--- МЕНЮ ---" << endl;
        cout << "1. Добавить слово вручную" << endl;
        cout << "2. Загрузить базу (russian_nouns.txt)" << endl;
        cout << "3. Поиск (автозамена)" << endl;
        cout << "0. Выход" << endl;
        cout << "Выбор: ";

        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore(1000, '\n');
            continue;
        }

        if (choice == 1) {
            string word;
            cout << "Введите слово: ";
            cin >> word;
            engine.addWord(word);
            cout << "Добавлено!" << endl;
        }
        else if (choice == 2) {
            engine.loadFromFile("russian_nouns.txt");
        }
        else if (choice == 3) {
            string query;
            cout << "Введите запрос: ";
            cin >> query;
            engine.search(query);
        }
        else if (choice == 0) break;
    }
    return 0;
}
#include <iostream>
#include <iomanip>
#include <string>

using namespace std;

// Константи розмірів (співвідношення 16:256 > 1:10)
const int CACHE_SIZE = 16;
const int MEM_SIZE = 256;

// Структура рядка кеш-пам'яті
struct CacheLine {
    bool valid; // Ознака присутності (чи є дані дійсними)
    bool dirty; // Ознака модифікації
    int tag;    // Тег (старші розряди адреси)
    int data;   // Самі дані

    // Конструктор за замовчуванням (кеш порожній)
    CacheLine() : valid(false), dirty(false), tag(-1), data(0) {}
};

int mainMemory[MEM_SIZE];
CacheLine cache[CACHE_SIZE];

// Початкова ініціалізація основної пам'яті
void initMemory() {
    for (int i = 0; i < MEM_SIZE; i++) {
        mainMemory[i] = i * 10; // Просто базові дані для наглядності
    }
}

// Функція читання з пам'яті
void readData(int address) {
    if (address < 0 || address >= MEM_SIZE) {
        cout << "Помилка: адреса виходить за межі основної пам'яті (0 - " << MEM_SIZE - 1 << ")!" << endl;
        return;
    }

    // Детерміноване відображення: знаходимо індекс та тег
    int index = address % CACHE_SIZE;
    int tag = address / CACHE_SIZE;

    cout << "\n--- Запит на ЧИТАННЯ за адресою " << address << " ---" << endl;
    cout << "Індекс кешу: " << index << ", Тег: " << tag << endl;

    // Перевірка на кеш-влучання
    if (cache[index].valid && cache[index].tag == tag) {
        cout << "-> Кеш-влучання (Cache Hit)! Дані успішно прочитано з кешу." << endl;
        cout << "Дані: " << cache[index].data << endl;
    } else {
        cout << "-> Кеш-промах (Cache Miss)!" << endl;
        
        // Зворотний запис (Write-back): якщо рядок зайнятий і був модифікований
        if (cache[index].valid && cache[index].dirty) {
            int oldAddress = cache[index].tag * CACHE_SIZE + index;
            cout << "   [!] Рядок кешу зайнятий модифікованими даними." << endl;
            cout << "   Витиснення... Запис старих даних (" << cache[index].data 
                 << ") в основну пам'ять за адресою " << oldAddress << endl;
            mainMemory[oldAddress] = cache[index].data;
        }
        
        // Завантаження нових даних з ОП в кеш
        cout << "   Завантаження потрібних даних з основної пам'яті в кеш..." << endl;
        cache[index].valid = true;
        cache[index].dirty = false; // Дані щойно з пам'яті, отже не модифіковані
        cache[index].tag = tag;
        cache[index].data = mainMemory[address];
        
        cout << "Дані прочитано: " << cache[index].data << endl;
    }
}

// Функція запису в пам'ять
void writeData(int address, int value) {
    if (address < 0 || address >= MEM_SIZE) {
        cout << "Помилка: адреса виходить за межі основної пам'яті (0 - " << MEM_SIZE - 1 << ")!" << endl;
        return;
    }

    int index = address % CACHE_SIZE;
    int tag = address / CACHE_SIZE;

    cout << "\n--- Запит на ЗАПИС значення " << value << " за адресою " << address << " ---" << endl;
    cout << "Індекс кешу: " << index << ", Тег: " << tag << endl;

    if (cache[index].valid && cache[index].tag == tag) {
        cout << "-> Кеш-влучання (Cache Hit)!" << endl;
        cout << "   Дані в кеші оновлено. Ознака модифікації (dirty) встановлена в 1." << endl;
        cache[index].data = value;
        cache[index].dirty = true;
    } else {
        cout << "-> Кеш-промах (Cache Miss)!" << endl;
        
        if (cache[index].valid && cache[index].dirty) {
            int oldAddress = cache[index].tag * CACHE_SIZE + index;
            cout << "   [!] Рядок кешу зайнятий модифікованими даними." << endl;
            cout << "   Витиснення... Запис старих даних (" << cache[index].data 
                 << ") в основну пам'ять за адресою " << oldAddress << endl;
            mainMemory[oldAddress] = cache[index].data;
        }
        
        cout << "   Блок завантажено в кеш та одразу модифіковано новим значенням." << endl;
        cache[index].valid = true;
        cache[index].dirty = true; // Записали нові дані, отже вони відрізняються від ОП
        cache[index].tag = tag;
        cache[index].data = value;
    }
}

// Вивід стану кеш-пам'яті
void printCache() {
    cout << "\n================ ПОТОЧНА КАРТА КЕШ-ПАМ'ЯТІ ================" << endl;
    cout << setw(8) << "Індекс" << setw(10) << "Valid" << setw(10) << "Dirty" 
         << setw(10) << "Tag" << setw(15) << "Дані" << endl;
    cout << "-----------------------------------------------------------" << endl;
    for (int i = 0; i < CACHE_SIZE; i++) {
        cout << setw(8) << i 
             << setw(10) << (cache[i].valid ? "1" : "0") 
             << setw(10) << (cache[i].dirty ? "1" : "0") 
             << setw(10) << (cache[i].valid ? to_string(cache[i].tag) : "-") 
             << setw(15) << (cache[i].valid ? to_string(cache[i].data) : "-") << endl;
    }
    cout << "===========================================================" << endl;
}

// Вивід стану фрагмента основної пам'яті навколо певної адреси
void printMainMemoryStatus(int startAddr) {
    int start = max(0, startAddr - 2);
    int end = min(MEM_SIZE - 1, startAddr + 5);
    
    cout << "\n--- Фрагмент основної пам'яті (адреси " << start << " - " << end << ") ---" << endl;
    for (int i = start; i <= end; i++) {
        cout << "Адреса " << i << ": " << mainMemory[i] << endl;
    }
}

int main() {
    initMemory();
    int choice = -1;
    int addr, val;

    cout << "Моделювання Кеш-пам'яті (Детерміноване відображення, Зворотний запис)" << endl;
    cout << "Розмір ОП: " << MEM_SIZE << ", Розмір кешу: " << CACHE_SIZE << endl;

    while (choice != 0) {
        cout << "\n===== ІНТЕРАКТИВНЕ МЕНЮ =====" << endl;
        cout << "1. Прочитати дані за адресою" << endl;
        cout << "2. Записати дані за адресою" << endl;
        cout << "3. Показати карту кеш-пам'яті" << endl;
        cout << "4. Перевірити фрагмент основної пам'яті" << endl;
        cout << "0. Вийти" << endl;
        cout << "Ваш вибір: ";
        cin >> choice;

        switch (choice) {
            case 1:
                cout << "Введіть адресу для читання (0-" << MEM_SIZE - 1 << "): ";
                cin >> addr;
                readData(addr);
                break;
            case 2:
                cout << "Введіть адресу (0-" << MEM_SIZE - 1 << "): ";
                cin >> addr;
                cout << "Введіть значення: ";
                cin >> val;
                writeData(addr, val);
                break;
            case 3:
                printCache();
                break;
            case 4:
                cout << "Введіть початкову адресу фрагмента: ";
                cin >> addr;
                printMainMemoryStatus(addr);
                break;
            case 0:
                cout << "Завершення роботи." << endl;
                break;
            default:
                cout << "Невірний вибір. Спробуйте ще раз." << endl;
        }
    }

    return 0;
}
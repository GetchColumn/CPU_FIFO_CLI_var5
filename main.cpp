#include <iostream>
#include <vector>
#include <Windows.h>

#include "commanderClass.h"
#include "modelClass.h"

#define COMM_IN_CACHE 75
#define COMM_COUNT 10

using namespace std;

vector<Command> commandListFull;
SystemBus SB;
CacheController CC1(&SB);
Microprocessor MP1(commandListFull, &SB, &CC1);

void step();

int main()
{
    // настройки для вывода кириллицы
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);

    setlocale(LC_ALL, "Russian");
    genComm(commandListFull, COMM_COUNT, COMM_IN_CACHE); // генерация списка команд
  
    //for (const auto& item : commandListFull)
    //{
    //    Command currCom = item;

    //    // вывод команд
    //    cout << item.getId() << ") \t" << item.getDuration() << "(";
    //    if (item.getInCacheState() == 1) cout << "кэш "; else cout << "н.к ";
    //    if (item.getType() == 1) cout << "УО"; else cout << "__";
    //    cout << ")" << endl;
    //}

    MP1.printVars();

}

void step()
{
    MP1.step();
}
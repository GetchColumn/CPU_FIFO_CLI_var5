#include <iostream>
#include <vector>
#include <Windows.h>
#include <chrono>

#include "commanderClass.h"
#include "modelClass.h"

#define COMM_IN_CACHE 75
#define COMM_COUNT 15

int clockSpeed = 300;

using namespace std;

vector<Command> commandListFull; // список команд, к которому имеют прямой доступ МП и КК
SystemBus SB;
CacheController CC1(&SB);
Microprocessor MP1(&SB, &CC1);

void step();
bool checkCommsIsDone(vector<Command> comms);

int main()
{
    // настройки для вывода кириллицы
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);

    setlocale(LC_ALL, "Russian");
    //genComm(commandListFull, COMM_COUNT, COMM_IN_CACHE); // генерация списка команд
    getUserComm(commandListFull);
 
    for (const auto& item : commandListFull)
    {
        Command currCom = item;

        // вывод команд
        cout << item.getId() << ") \t" << item.getDuration() << "(";
        if (item.getInCacheState() == 1) cout << "кэш "; else cout << "н.к ";
        if (item.getUO() == 1) cout << "УО"; else cout << "__";
        cout << ")" << endl;
    }

    MP1.loadCommands(&commandListFull);
    CC1.init(&commandListFull);
    //MP1.printVars();
    auto start = std::chrono::high_resolution_clock::now();

    int plotY = 0;
    while (!checkCommsIsDone(commandListFull))
    {
        plotY++;
        cout << endl << "Такт " << plotY << endl;
        step();
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;
    cout << "Время выполнения программы: " << duration.count() << " сек." << endl;
    cout << "Производительность системы: " << (commandListFull.capacity() * clockSpeed) / plotY << " миллионов инструкций в сек." << endl;

}

void step()
{
    // какая то хрень
    MP1.stepConv();
    MP1.stepWait();
    CC1.step();
    MP1.stepWait();
    MP1.stepWaitCC();
    if (!MP1.waitCheck() && !CC1.isWorking())
    {
        CC1.step();
    }
    //if (MP1.wait == true) MP1.stepConv();
}

bool checkCommsIsDone(vector<Command> comms)
{
    bool allDone = true;
    for (const auto& item : comms)
    {
        Command cur = item;
        if (!cur.isDone())
        {
            allDone = false;
            return allDone;
        }
    }
    return allDone;
}
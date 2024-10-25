#include <iostream>
#include <list>
#include <Windows.h>

#include "commGenerator.h"

using namespace std;

int main()
{
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);

    setlocale(LC_ALL, "Russian");
    std::cout << "Hello World!\n";
    list<Command> comms;
    genComm(comms, 15, 65);
  
    for (const auto& item : comms)
    {
        cout << item.id << ") \t Длит-сть: " << item.duration;
        cout << " Кэш: " << item.inCache;
        cout << " Тип: " << item.type << endl;

    }
}

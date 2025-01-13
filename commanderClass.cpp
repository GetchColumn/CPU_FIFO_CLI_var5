#include <vector>
#include <iostream>
#include <random>
#include "commanderClass.h"

#define DMADuration 4

using namespace std;

void genComm(vector<Command>& commList, const int commandCount, const int inCacheChance, const int dmaChance)
{
	random_device rd;
	mt19937 gen(rd()); // Мерсеннский твистер
	uniform_int_distribution<> dis(1, 100);

	for (int c = 0; c < commandCount; c++)
	{
		int randTaskNum = dis(gen);	// число для определения типа задачи
		int randInCache = dis(gen);	// число для определения нахождения в кэше
		int randDuratNum = dis(gen);	// число для определения длительности команды
		int randDMANum = dis(gen);
		bool commInCache = 0;	// признак нахождения команды в КЭШе
		int duration;
		bool type = false;

		if (randDMANum <= dmaChance)
		{
			duration = DMADuration;
			Command comm(c, duration, 1, 0, true);
			commList.push_back(comm);
			continue;
		}

		// определяем, находится команда в кэше или нет
		if (randInCache <= inCacheChance)
		{
			commInCache = true;
		}
		else
		{
			commInCache = 0;
		}

		// ДВП
		if (randTaskNum <= 50)
		{
			if (randDuratNum <= 60)
			{
				duration = 2;
			}
			else
			{
				duration = 1;
			}
			Command comm(c,duration,commInCache,type, false);
			commList.push_back(comm);
		}

		// МДО
		else if (randTaskNum > 50 && randTaskNum <= 70)
		{
			if (randDuratNum <= 70)
			{
				duration = 5;
			}
			else if (randDuratNum > 70 && randDuratNum <= 90)
			{
				duration = 2;
			}
			else if (randDuratNum > 90)
			{
				duration = 1;
			}
			Command comm(c, duration, commInCache, type, false);
			commList.push_back(comm);
		}
		// МСО
		else if (randTaskNum > 70 && randTaskNum <= 85)
		{
			if (randDuratNum <= 70)
			{
				duration = 2;
			}
			else if (randDuratNum > 70 && randDuratNum <= 90)
			{
				duration = 5;
			}
			else if (randDuratNum > 90)
			{
				duration = 1;
			}
			Command comm(c, duration, commInCache, type, false);
			commList.push_back(comm);
		}
		// УО
		else if (randTaskNum > 85)
		{
			type = true;

			if (randDuratNum <= 80)
			{
				duration = 2;
			}
			else
			{
				duration = 1;
			}
			Command comm(c, duration, commInCache, type, false);
			commList.push_back(comm);
		}

	}
}

void getUserComm(vector<Command>& commList)
{
	int count;
	cout << endl << "Введите количество команд -> ";
	cin >> count;

	for (int c = 0; c < count; c++)
	{
		int duration = 0;
		int commInCache = 0;
		bool type = 0;
		bool DMA = 0;

		cout << endl << "Команда № " << c;
		cout << endl << "Введите длительность команды -> ";
		cin >> duration;

		cout << "Введите состояние НК / К / DMA (0/1/2) команды -> ";
		cin >> commInCache;

		if (commInCache != 2)
		{
			cout << "Введите тип УО/_ (1/0) команды -> ";
			cin >> type;
		}
		else
		{
			commInCache = 1;
			type = 0;
			DMA = 1;
			duration = DMADuration;
		}


		Command comm(c, duration, commInCache, type, DMA);
		commList.push_back(comm);
	}

}

void printCommands(vector<Command> cmdVect)
{
	for (const auto& item : cmdVect)
	{
		Command currCom = item;
		
		// вывод команд
		if (item.isDMA())
		{
			cout << item.getId() << ") \t" << item.getDuration() << "(DMA)" << endl;
		}
		else
		{
			cout << item.getId() << ") \t" << item.getDuration() << "(";
			if (item.getInCacheState() == 1) cout << "кэш "; else cout << "н.к ";
			if (item.getUO() == 1) cout << "УО"; else cout << "__";
			cout << ")" << endl;
		}
	}
}

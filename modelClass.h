#pragma once
#include <vector>
#include <deque>

#include "commanderClass.h"

using namespace std;

// System bus
class SystemBus
{
	bool busy;
public:
	SystemBus() : busy(false)
	{}

	void takeBus() { busy = true; }

	void releaseBus() { busy = false; }

	bool isBusy() const { return busy; }
};

// CacheController
class CacheController
{
public:
	SystemBus *SB_CC;
	//bool needFreeSB = false;	//Нужно ли КК освободить СШ?
	bool work = false;	//Работает ли КК
	const int durationCommand;	//Длительность работы КК
	int remainingTime;
	int currentIndex;
	vector<Command> CCTask;	// список команд
	deque<int> indexes;	// индексы нк команд

	CacheController(SystemBus* _SB_CC) : SB_CC(_SB_CC), work(false), durationCommand(6), remainingTime(0), currentIndex(0), CCTask({}), indexes({})
	{}

	// загрузка актуального списка задач
	void init(vector<Command> _CCTask)
	{
		CCTask = _CCTask;
	}

	//bool check()
	//{
	//	if (remainingTime == 0) return true; else return false;
	//}
	bool checkId(int _id)
	{
		return (CCTask.at(_id).getInCacheState());
	}
	void load(int ind)
	{
		indexes.push_back(ind);
	}

	void step()
	{
		if (indexes.empty())
		{
			//drawNullCC();
			return;
		}
		if (SB_CC->isBusy())
		{
			work = false;
			//drawNullCC();
			return;
		}
		else
		{

		}
	}
};

// Microprocessor
class Microprocessor
{
private:
	vector<Command> commandVectMP;
	Command *currentCommand;
	SystemBus *SB_MP;
	CacheController *CC_MP;
	bool requestSB; // запрос на использование СШ
	bool wait;     //находится ли процессор в режиме ожидания?
	bool work;     //работает ли процессор?
	int remainingTime;	// оставшееся время работы мп
	bool workOnSB; // признак работы на СШ
	int convCount;	// количество конвейеров
	int comIndex;	// индекс текущей команды
	vector<int> requestVect;
public:

	Microprocessor(vector<Command> _commandVectMP, SystemBus* _SB_MP, CacheController* _CC_MP) : commandVectMP(_commandVectMP),
		currentCommand(nullptr), SB_MP(_SB_MP), CC_MP(_CC_MP), requestSB(false), wait(false), work(false), remainingTime(0), workOnSB(false),
		convCount(1), comIndex(0), requestVect()
	{}

	void printVars()
	{
		cout << "Class MP: \n" << "reqestSB> " << requestSB << "\nwait> " << wait << "\nwork> " << work << "\nremainingTime> " << remainingTime
			<< "\nworkOnSB> " << workOnSB << "\nconvCount> " << convCount << "\ncomIndex> " << comIndex << endl;
		for (const auto& item : commandVectMP)
		{
			Command currCom = item;

			// вывод команд
			cout << item.getId() << ") \t" << item.getDuration() << "(";
			if (item.getInCacheState() == 1) cout << "кэш "; else cout << "н.к ";
			if (item.getUO() == 1) cout << "УО"; else cout << "__";
			cout << ")" << endl;
		}
	}

	void step()
	{
		if (comIndex == (commandVectMP.size() + 1)) comIndex = 0;
		if (work == true)
		{
			if (remainingTime != 0)
			{
				remainingTime--;
				//draw();
				return;
			}
			else
			{
				work = false;
				workOnSB = false;
				commandVectMP.at(comIndex).markDone();
				comIndex++;
			}
		}
		currentCommand = &(commandVectMP.at(comIndex)); // получение текущей команды по индексу
		
		// поиск и получение невыполненной команды
		if (currentCommand->isDone() == true)
		{
			while (currentCommand->isDone())
			{
				comIndex++;
				currentCommand = &(commandVectMP.at(comIndex)); // получение текущей команды по индексу
			}
		}

		// проверка команды на нахождение в кэше
		if (currentCommand->getInCacheState())
		{
			//drawDecode()
			if (currentCommand->getUO())
			{
				if (SB_MP->isBusy())
				{
					wait = true;
					//drawNull()
				}
				else
				{
					work = true;
					workOnSB = true;
					remainingTime = (currentCommand->getDuration()) * 2;
					return;
				}
			}
			else
			{
				work = true;
				remainingTime = currentCommand->getDuration();
				return;
			}
		}
		else
		{
			//cc.load(currentCommand)
			requestVect.push_back(comIndex);
			comIndex++;
			return;
		}

	}

};

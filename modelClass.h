#pragma once
#include <vector>
#include <deque>
#include <algorithm>

#include "commanderClass.h"

using namespace std;

// System bus
class SystemBus
{
	bool busy;
public:
	SystemBus() : busy(false)
	{}

	void takeBus() { busy = true; cout << "СШ занята" << endl; }

	void releaseBus() { busy = false; cout << "СШ освобождена" << endl;}

	bool isBusy() const { return busy; }
};

// CacheController
class CacheController
{
private:
	SystemBus *SB_CC;
	bool requestSB = false;	//Нужно ли КК освободить СШ?
	bool work = false;	//Работает ли КК
	const int durationCommand;	//Длительность работы КК
	int remainingTime;	// оставшиеся такты до окончания работы КК
	int currentIndex;	// индекс текущей команды
	vector<Command> *CCTask;	// список всех команд
	deque<int> indexes;	// индексы нк команд

public:
	CacheController(SystemBus* _SB_CC) : SB_CC(_SB_CC), requestSB(false), work(false), durationCommand(6), remainingTime(0), currentIndex(0), CCTask(nullptr), indexes({})
	{}

	// загрузка актуального списка задач
	void init(vector<Command> *_CCTask)
	{
		CCTask = _CCTask;
	}

	//bool check()
	//{
	//	if (remainingTime == 0) return true; else return false;
	//}
	bool checkId(int _id)
	{
		return (CCTask->at(_id).getInCacheState());
	}
	void load(int ind)
	{
		cout << "КК получил запрос на " << ind << endl;
		if (!CCTask->at(ind).getInCacheState()) indexes.push_back(ind);
	}

	// вызывается каждый такт
	void step()
	{
		if (work == true)
		{
			// если КК уже работает
			if (remainingTime == 1)
			{
				cout << "Загружено в кэш (" << currentIndex << ")" << endl;
				// если время работы КК вышло
				CCTask->at(currentIndex).setInCache();
				indexes.pop_front();
				work = false;
				SB_CC->releaseBus();
				return;
			}
			else
			{
				// работа КК - уменьшение кол-ва оставшихся тактов и рисование
				remainingTime--;
				//drawCC()
				return;
			}
		}
		else
		{
			// если КК не работает
			if (indexes.empty())
			{
				// если список индексов команд для загрузки пуст
				//drawCCNull
				return;
			}
			else
			{
				// если есть команды для загрузки
				currentIndex = indexes.at(0);
				if (SB_CC->isBusy())
				{
					// если СШ занята
					//drawSBRequest
					requestSB = true;
					return;
				}
				else
				{
					cout << "КК начал работу над " << currentIndex << endl;
					// если СШ свободна
					work = true;
					SB_CC->takeBus();
					requestSB = false;
					remainingTime = 6;
					//drawCC
				}
			}
		}
	}
};

// Microprocessor
class Microprocessor
{
private:
	vector<Command> *commandVectMP;
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
	deque<int> requestVect;
public:

	Microprocessor(SystemBus* _SB_MP, CacheController* _CC_MP) :currentCommand(nullptr), commandVectMP(nullptr), SB_MP(_SB_MP), CC_MP(_CC_MP),
		requestSB(false), wait(false), work(false), remainingTime(0), workOnSB(false), convCount(1), comIndex(0), requestVect()
	{}

	void loadCommands(vector<Command> *_commandVectMP)
	{
		commandVectMP = _commandVectMP;
	}

	void printVars()
	{
		cout << "Class MP: \n" << "reqestSB> " << requestSB << "\nwait> " << wait << "\nwork> " << work << "\nremainingTime> " << remainingTime
			<< "\nworkOnSB> " << workOnSB << "\nconvCount> " << convCount << "\ncomIndex> " << comIndex << endl;
		for (const auto& item : *commandVectMP)
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
		if (work == true)
		{
			if (remainingTime != 1)
			{
				cout << "МП работает~" << endl;
				remainingTime--;
				//draw();
				return;
			}
			else
			{
				cout << "Команда (" << comIndex << ") выполнена" << endl;
				work = false;
				if (workOnSB) SB_MP->releaseBus(); // отпустить СШ если работал на ней
				workOnSB = false;
				commandVectMP->at(comIndex).markDone();
				comIndex++;
			}
		}

		if (comIndex >= (commandVectMP->size())) comIndex = 0; // защита от убегания индекса
		//////////
		currentCommand = &(commandVectMP->at(comIndex)); // получение текущей команды по индексу
		
		// поиск и получение невыполненной команды
		if (currentCommand->isDone() == true)
		{
			int whileStopper = 0;
			while (currentCommand->isDone())
			{
				comIndex++;
				////////
				if (comIndex >= (commandVectMP->size())) comIndex = 0; // защита от убегания индекса
				currentCommand = &(commandVectMP->at(comIndex)); // получение текущей команды по индексу
				whileStopper++;
				if (whileStopper > commandVectMP->size())
				{
					cout << "conv.done" << endl;
					return;
					break; // conv done
				}
			}
		}

		// проверка команды на нахождение в кэше
		if (!currentCommand->getInCacheState())
		{

			int whileStopper = 0;
			while (!(currentCommand->getInCacheState() && !currentCommand->isDone()))
			{
				// цикл для заявок и поиска КЭШ команд
				if ((requestVect.end() == find(requestVect.begin(), requestVect.end(), comIndex)) && !currentCommand->getInCacheState())
				{
					cout << "Заявка на команду " << comIndex << endl;
					requestVect.push_back(comIndex);
					CC_MP->load(currentCommand->getId());
				}
				comIndex++;
				if (comIndex >= (commandVectMP->size())) comIndex = 0; // защита от убегания индекса
				//////////
				currentCommand = &(commandVectMP->at(comIndex)); // получение текущей команды по индексу
				whileStopper++;
				if (whileStopper > commandVectMP->size())
				{
					cout << "CCfinder done" << endl;
					return;
					//break;
				}
			}

		}
		
		// декодирование, если не сделано
		if (!currentCommand->isDecoded())
		{
			cout << "Декодирование " << currentCommand->getId() << endl;
			//drawDecode()
			currentCommand->markDecoded();
			return;
		}
		// проверка команды на УО
		if (currentCommand->getUO())
		{
			if (SB_MP->isBusy())
			{
				wait = true;
				//drawNull()
				return;
			}
			else
			{
				cout << "МП на СШ с командой " << currentCommand->getId() << endl;
				work = true;
				workOnSB = true;
				wait = false;
				SB_MP->takeBus();
				remainingTime = (currentCommand->getDuration()) * 2;
				return;
			}
		}
		else
		{
			cout << "МП с командой " << currentCommand->getId() << endl;
			// draw
			work = true;
			remainingTime = currentCommand->getDuration();
			return;
		}

	}

};

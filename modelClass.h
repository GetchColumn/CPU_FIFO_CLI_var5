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
	bool work;     //работает ли процессор?
//	int remainingTime;	// оставшееся время работы мп
//	bool workOnSB; // признак работы на СШ
	int countConv;	// количество конвейеров
	int comIndex;	// индекс текущей команды
	//deque<int> requestVect;

	class Conveyor
	{
		vector<Command*> commandVectCV;
		Command* currentCommandCV;
		SystemBus* SB_CV;
		CacheController* CC_CV;
		vector<Command> commVectCV;
		short convNum;
		bool requestSBCV; // запрос на использование СШ
		bool workCV;     //работает ли процессор?
		int remainingTimeCV;	// оставшееся время работы мп
		bool workOnSBCV; // признак работы на СШ
		bool busy;
		int comIndexCV;	// индекс текущей команды
		deque<int> requestVectCV;
	public:
		bool waitCV;	//находится ли поток в режиме ожидания?

		Conveyor() :currentCommandCV(nullptr), commandVectCV(), SB_CV(nullptr), CC_CV(nullptr),
			convNum(0), requestSBCV(false), waitCV(false), workCV(false), busy(false), remainingTimeCV(0), workOnSBCV(false), comIndexCV(0), requestVectCV()
		{}

		void init(SystemBus* _SB_CV, CacheController* _CC_CV, short _convNum)
		{
			SB_CV = _SB_CV;
			CC_CV = _CC_CV;
			convNum = _convNum;
		}

		bool needNewComms()
		{
			int i = 0;
			for (const auto& item : commandVectCV)
			{
				if (!item->isDone())
				{
					// если команда не выполнена
					i++;
				}
			}
			if (i <= 1)
			{
				return true;
			}
			return false;
		}

		bool loadCommand(Command* _commandCV)
		{
			commandVectCV.push_back(_commandCV);
			if (_commandCV->getInCacheState())
			{
				busy = true;
				return true;
			}
			return false;
		}

		bool isBusy()
		{
			return busy;
		}

		void step()
		{
			if (commandVectCV.empty()) return; // если команд нет, выход

			if (workCV == true)
			{
				if (remainingTimeCV != 1)
				{
					cout << "K" << convNum << " работает~" << endl;
					remainingTimeCV--;
					//draw();
					return;
				}
				else
				{
					cout << "Команда (" << commandVectCV.at(comIndexCV)->getId() << ") выполнена" << endl;
					workCV = false;
					if (workOnSBCV) SB_CV->releaseBus(); // отпустить СШ если работал на ней
					workOnSBCV = false;
					commandVectCV.at(comIndexCV)->markDone();
					comIndexCV++;

					if (!requestVectCV.empty())
					{
						// вект запросов не пуст
						if (commandVectCV.at(requestVectCV.at(0))->getInCacheState())
						{
							comIndexCV = requestVectCV.at(0);
							requestVectCV.pop_front();
						}
					}
				}
			}

			if (comIndexCV >= (commandVectCV.size())) comIndexCV = 0; // защита от убегания индекса
			//////////
			currentCommandCV = (commandVectCV.at(comIndexCV)); // получение текущей команды по индексу

			// поиск и получение невыполненной команды
			if (currentCommandCV->isDone() == true)
			{
				int whileStopper = 0;
				while (currentCommandCV->isDone())
				{
					comIndexCV++;
					////////
					if (comIndexCV >= (commandVectCV.size())) comIndexCV = 0; // защита от убегания индекса
					currentCommandCV = (commandVectCV.at(comIndexCV)); // получение текущей команды по индексу
					whileStopper++;
					if (whileStopper > commandVectCV.size())
					{
						busy = false;
						cout << "conv.done" << endl;
						return;
					}
				}
			}

			// проверка команды на нахождение в кэше
			if (!currentCommandCV->getInCacheState())
			{

				int whileStopper = 0;
				while (!(currentCommandCV->getInCacheState() && !currentCommandCV->isDone()))
				{
					// цикл для заявок и поиска КЭШ команд
					if ((requestVectCV.end() == find(requestVectCV.begin(), requestVectCV.end(), comIndexCV)) && !currentCommandCV->getInCacheState())
					{
						cout << "Заявка К"<< convNum <<" на команду " << comIndexCV << endl;
						requestVectCV.push_back(comIndexCV);
						CC_CV->load(currentCommandCV->getId());
					}
					comIndexCV++;
					if (comIndexCV >= (commandVectCV.size())) comIndexCV = 0; // защита от убегания индекса
					//////////
					currentCommandCV = (commandVectCV.at(comIndexCV)); // получение текущей команды по индексу
					whileStopper++;
					if (whileStopper > commandVectCV.size())
					{
						//cout << "CCfinder done" << endl;
						return;
						//break;
					}
				}

			}

			// декодирование, если не сделано
			if (!currentCommandCV->isDecoded())
			{
				cout << "К" << convNum <<" Декодирование " << currentCommandCV->getId() << endl;
				//drawDecode()
				busy = true;
				currentCommandCV->markDecoded();
				return;
			}
			// проверка команды на УО
			if (currentCommandCV->getUO())
			{
				if (SB_CV->isBusy())
				{
					waitCV = true;
					//drawNull()
					return;
				}
				else
				{
					cout << "К"<< convNum << " на СШ с командой " << currentCommandCV->getId() << endl;
					workCV = true;
					workOnSBCV = true;
					waitCV = false;
					SB_CV->takeBus();
					remainingTimeCV = (currentCommandCV->getDuration()) * 2;
					return;
				}
			}
			else
			{
				cout << "К" << convNum << " с командой " << currentCommandCV->getId() << endl;
				// draw
				workCV = true;
				remainingTimeCV = currentCommandCV->getDuration();
				return;
			}

		}

	};

	Conveyor CV1, CV2;

public:

	bool wait;     //находится ли процессор в режиме ожидания?

	Microprocessor(SystemBus* _SB_MP, CacheController* _CC_MP) :currentCommand(nullptr), commandVectMP(nullptr), SB_MP(_SB_MP), CC_MP(_CC_MP),
		requestSB(false), wait(false), work(false), countConv(1), comIndex(0)
	{
		CV1.init(SB_MP, CC_MP, 1);
		CV2.init(SB_MP, CC_MP, 2);
	}

	void loadCommands(vector<Command> *_commandVectMP)
	{
		commandVectMP = _commandVectMP;
	}
	void commandFeeder()
	{
		if (!CV1.isBusy() || !CV2.isBusy())
		{
			// если кто-то не работает

			if (!CV1.isBusy())
			{
				bool flag1 = true;
				while (flag1)
				{
					if (comIndex >= (commandVectMP->size())) break; // защита от убегания индекса
					flag1 = !CV1.loadCommand(&commandVectMP->at(comIndex));
					comIndex++;
				}
			}
			if (!CV2.isBusy())
			{
				bool flag2 = true;
				while (flag2)
				{
					if (comIndex >= (commandVectMP->size())) break; // защита от убегания индекса
					flag2 = !CV2.loadCommand(&commandVectMP->at(comIndex));
					comIndex++;
				}
			}
		}
	}

	void commandAdder()
	{
		// если кто-то не работает

		if (CV1.needNewComms())
		{
			bool flag1 = true;
			while (flag1)
			{
				if (comIndex >= (commandVectMP->size())) break; // защита от убегания индекса
				flag1 = !CV1.loadCommand(&commandVectMP->at(comIndex));
				comIndex++;
			}
		}
		if (CV2.needNewComms())
		{
			bool flag2 = true;
			while (flag2)
			{
				if (comIndex >= (commandVectMP->size())) break; // защита от убегания индекса
				flag2 = !CV2.loadCommand(&commandVectMP->at(comIndex));
				comIndex++;
			}
		}
	}

	//void printVars()
	//{
	//	cout << "Class MP: \n" << "reqestSB> " << requestSB << "\nwait> " << wait << "\nwork> " << work << "\nremainingTime> " << remainingTime
	//		<< "\nworkOnSB> " << workOnSB << "\nconvCount> " << countConv << "\ncomIndex> " << comIndex << endl;
	//	for (const auto& item : *commandVectMP)
	//	{
	//		Command currCom = item;
	//		// вывод команд
	//		cout << item.getId() << ") \t" << item.getDuration() << "(";
	//		if (item.getInCacheState() == 1) cout << "кэш "; else cout << "н.к ";
	//		if (item.getUO() == 1) cout << "УО"; else cout << "__";
	//		cout << ")" << endl;
	//	}
	//}

	void stepConv()
	{
		commandFeeder();
		CV1.step();
		CV2.step();

		//if (CV1.needNewComms() || CV2.needNewComms())
		//{
		//	commandAdder();
		//}

	}

	void stepWait()
	{
		if (CV1.waitCV == true) CV1.step();
		if (CV2.waitCV == true) CV2.step();
	}

	//void step()
	//{
	//	if (work == true)
	//	{
	//		if (remainingTime != 1)
	//		{
	//			cout << "МП работает~" << endl;
	//			remainingTime--;
	//			//draw();
	//			return;
	//		}
	//		else
	//		{
	//			cout << "Команда (" << comIndex << ") выполнена" << endl;
	//			work = false;
	//			if (workOnSB) SB_MP->releaseBus(); // отпустить СШ если работал на ней
	//			workOnSB = false;
	//			commandVectMP->at(comIndex).markDone();
	//			comIndex++;

	//			if (!requestVect.empty())
	//			{
	//				// вект запросов не пуст
	//				if (commandVectMP->at(requestVect.at(0)).getInCacheState())
	//				{
	//					comIndex = requestVect.at(0);
	//					requestVect.pop_front();
	//				}
	//			}
	//		}
	//	}

	//	if (comIndex >= (commandVectMP->size())) comIndex = 0; // защита от убегания индекса
	//	//////////
	//	currentCommand = &(commandVectMP->at(comIndex)); // получение текущей команды по индексу
	//	
	//	// поиск и получение невыполненной команды
	//	if (currentCommand->isDone() == true)
	//	{
	//		int whileStopper = 0;
	//		while (currentCommand->isDone())
	//		{
	//			comIndex++;
	//			////////
	//			if (comIndex >= (commandVectMP->size())) comIndex = 0; // защита от убегания индекса
	//			currentCommand = &(commandVectMP->at(comIndex)); // получение текущей команды по индексу
	//			whileStopper++;
	//			if (whileStopper > commandVectMP->size())
	//			{
	//				cout << "conv.done" << endl;
	//				return;
	//				break; // conv done
	//			}
	//		}
	//	}

	//	// проверка команды на нахождение в кэше
	//	if (!currentCommand->getInCacheState())
	//	{

	//		int whileStopper = 0;
	//		while (!(currentCommand->getInCacheState() && !currentCommand->isDone()))
	//		{
	//			// цикл для заявок и поиска КЭШ команд
	//			if ((requestVect.end() == find(requestVect.begin(), requestVect.end(), comIndex)) && !currentCommand->getInCacheState())
	//			{
	//				cout << "Заявка на команду " << comIndex << endl;
	//				requestVect.push_back(comIndex);
	//				CC_MP->load(currentCommand->getId());
	//			}
	//			comIndex++;
	//			if (comIndex >= (commandVectMP->size())) comIndex = 0; // защита от убегания индекса
	//			//////////
	//			currentCommand = &(commandVectMP->at(comIndex)); // получение текущей команды по индексу
	//			whileStopper++;
	//			if (whileStopper > commandVectMP->size())
	//			{
	//				//cout << "CCfinder done" << endl;
	//				return;
	//				//break;
	//			}
	//		}

	//	}
	//	
	//	// декодирование, если не сделано
	//	if (!currentCommand->isDecoded())
	//	{
	//		cout << "Декодирование " << currentCommand->getId() << endl;
	//		//drawDecode()
	//		currentCommand->markDecoded();
	//		return;
	//	}
	//	// проверка команды на УО
	//	if (currentCommand->getUO())
	//	{
	//		if (SB_MP->isBusy())
	//		{
	//			wait = true;
	//			//drawNull()
	//			return;
	//		}
	//		else
	//		{
	//			cout << "МП на СШ с командой " << currentCommand->getId() << endl;
	//			work = true;
	//			workOnSB = true;
	//			wait = false;
	//			SB_MP->takeBus();
	//			remainingTime = (currentCommand->getDuration()) * 2;
	//			return;
	//		}
	//	}
	//	else
	//	{
	//		cout << "МП с командой " << currentCommand->getId() << endl;
	//		// draw
	//		work = true;
	//		remainingTime = currentCommand->getDuration();
	//		return;
	//	}
	//}

};

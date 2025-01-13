#pragma once
#include <vector>
#include <deque>
#include <algorithm>

#include "commanderClass.h"

using namespace std;

// System bus
class SystemBus
{
	bool busy; // признак того что системная шина занята
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
	SystemBus *SB_CC; // указатель на объект системной шины
	bool requestSB = false;	// Нужно ли КК освободить СШ?
	bool work = false;	// Работает ли КК
	const int durationCommand;	// Длительность работы КК
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

	bool isWorking()
	{
		return work;
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
	vector<Command> *commandVectMP; // указатель на вектор с командами
	Command *currentCommand;	// указатель на текущую команду
	SystemBus *SB_MP;	// указатель на объект класса "системная шина"
	CacheController *CC_MP;	// указатель на объект класса "кэш-контроллер"
	bool requestSB; // флаг запроса на использование системной шины
	bool work;	// признак работы микропроцессора
//	int remainingTime;	// оставшееся время работы мп
//	bool workOnSB; // признак работы на СШ
	int countConv;	// количество конвейеров
	int comIndex;	// индекс текущей команды
	deque<int> requestDeque;

	class Conveyor
	{
		vector<Command*> commandVectCV; // вектор указателей на указатели в векторе команд микропроцессора
		Command* currentCommandCV;	// указатель на текущую команду
		SystemBus* SB_CV;	// указатель на объект класса "системная шина"
		CacheController* CC_CV;	// указатель на объект класса "кэш-контроллер"
		deque<int>* RQ;
		//vector<Command> commVectCV;
		short convNum;	// номер конвейера
		bool requestSBCV; // запрос на использование системной шины
		bool workCV;     // признак работы конвейера
		int remainingTimeCV;	// оставшееся время работы конвейера
		bool workOnSBCV; // признак работы на СШ
		bool busy;	// признак того что у конвейера есть работа
		bool waitCache;	// признак того что конвейер ожидает загрузки команды в кэш
		int comIndexCV;	// индекс текущей команды
		deque<int> requestVectCV;	// структура данных с индексами н.к. команд

		void busyToggler()
		{
			if (remainingTimeCV < 2)
			{
				if (this->needNewComms())
				{
					cout << "conv. " << convNum << " unbusy" << endl;
					busy = false;
				}
			}
		}

	public:
		bool waitCV;	//находится ли поток в режиме ожидания?

		Conveyor() :currentCommandCV(nullptr), RQ(nullptr), commandVectCV(), SB_CV(nullptr), CC_CV(nullptr),
			convNum(0), requestSBCV(false), waitCV(false), workCV(false), busy(false), waitCache(false), remainingTimeCV(0), workOnSBCV(false), comIndexCV(0), requestVectCV()
		{}

		void init(SystemBus* _SB_CV, CacheController* _CC_CV, short _convNum, deque<int>* _RQ)
		{
			SB_CV = _SB_CV;
			CC_CV = _CC_CV;
			convNum = _convNum;
			RQ = _RQ;
		}

		bool needNewComms()
		{
			int i = 0;
			//for (const auto& item : commandVectCV)
			//{
			//	if (!item->isDone())
			//	{
			//		// если команда не выполнена
			//		i++;
			//	}
			//}
			//if (i <= 1 && remainingTimeCV <= 2 )
			//{
			//	return true;
			//}
			//return false;
			for (const auto& item : commandVectCV)
			{
				//if (!(item->isDone() && item->getInCacheState()))
				if (!item->isDone() && item->getInCacheState())
				{
					// команда не сделана и в кэше
					i++;
				}
			}
			if (i < 2)
			{
				return true;
			}
			return false;
		}

		bool loadCommand(Command* _commandCV)
		{
			if (!_commandCV->isDone())
			{
				commandVectCV.push_back(_commandCV);
				if (_commandCV->getInCacheState())
				{
					busy = true;
					return true;
				}
			}
			return false;
		}

		bool isBusy()
		{
			return busy;
		}
		bool isWorking() { return this->workCV; }

		bool isWaitCache()
		{
			return waitCache;
		}

		void step(bool zapret = false)
		{
			if (commandVectCV.empty()) return; // если команд нет, выход

			if (workCV == true)
			{
				waitCache = false;
				if (remainingTimeCV != 1)
				{
					cout << "K" << convNum << " работает~" << endl;
					remainingTimeCV--;
					//draw();
					this->busyToggler();
					return;
				}
				else
				{
					cout << "Команда (" << commandVectCV.at(comIndexCV)->getId() << ") выполнена" << endl;
					if (workOnSBCV && !RQ->empty())
					{
						if (RQ->front() == convNum) RQ->pop_front();
					}
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
						cout << "conv. " << convNum << " done" << endl;
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
						cout << "Заявка К"<< convNum <<" на команду " << commandVectCV.at(comIndexCV)->getId() << endl;
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
						cout << "CCfinder done "<< convNum << endl;
						this->busyToggler();
						waitCache = true;
						return;
						//break;
					}
					waitCache = false;

				}

			}

			// декодирование, если не сделано
			if (!currentCommandCV->isDecoded())
			{
				cout << "К" << convNum <<" Декодирование " << currentCommandCV->getId() << endl;
				//drawDecode()
				busy = true;
				waitCache = false;
				currentCommandCV->markDecoded();
				return;
			}
			// проверка команды на УО
			if (currentCommandCV->getUO())
			{
				if (SB_CV->isBusy() )//|| zapret == true)
				{
					waitCV = true;
					if (find(RQ->begin(), RQ->end(), convNum) == RQ->end())
					{
						RQ->push_back(convNum);
					}
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
				this->busyToggler();
				return;
			}

		}

	};

	class DMAconv
	{
		vector<Command*> commandVectDMA; // вектор указателей на указатели в векторе команд микропроцессора
		Command* currentCommandDMA;	// указатель на текущую команду
		Microprocessor* mp;
		SystemBus* SB_DMA;	// указатель на объект класса "системная шина"
		//deque<int>* RQ;
		short convNum;	// номер конвейера
		bool requestSBDMA; // запрос на использование системной шины
		bool workDMA;     // признак работы конвейера
		int remainingTimeDMA;	// оставшееся время работы конвейера
		bool workOnSBDMA; // признак работы на СШ
		bool busy;	// признак того что у конвейера есть работа
		bool waitCache;	// признак того что конвейера ожидает загрузки команды в кэш
		int comIndexDMA;	// индекс текущей команды

		void busyToggler()
		{
			if (remainingTimeDMA <= 2)
			{
				if (this->needNewComms())
				{
					cout << "conv. " << convNum << " unbusy" << endl;
					busy = false;
				}
			}
		}
	public:
		bool waitDMA;	//находится ли поток в режиме ожидания?

		void init(SystemBus* _SB_CV, Microprocessor* _mp, short _convNum)//, deque<int>* _RQ)
		{
			SB_DMA = _SB_CV;
			convNum = _convNum;
			mp = _mp;
			//RQ = _RQ;
		}

		bool needNewComms()
		{
			int i = 0;

			for (const auto& item : commandVectDMA)
			{
				//if (!(item->isDone() && item->getInCacheState()))
				if (!item->isDone() && item->getInCacheState())
				{
					// команда не сделана и в кэше
					i++;
				}
			}
			if (i < 2)
			{
				return true;
			}
			return false;
		}

		bool loadCommand(Command* _commandCV)
		{
			if (!_commandCV->isDone())
			{
				commandVectDMA.push_back(_commandCV);

				if (_commandCV->getInCacheState())
				{
					busy = true;
					return true;
				}
			}
			return false;
		}

		bool isBusy()
		{
			return busy;
		}
		bool isWorking() { return this->workDMA; }

		bool isWaitCache()
		{
			return waitCache;
		}

		void step(bool zapret = false)
		{
			if (commandVectDMA.empty()) return; // если команд нет, выход

			if (workDMA == true)
			{
				waitCache = false;
				if (remainingTimeDMA != 1)
				{
					cout << "DMA " << convNum << " работает~" << endl;
					remainingTimeDMA--;
					//draw();
					//this->busyToggler();
					return;
				}
				else
				{
					cout << "Команда на DMA (" << commandVectDMA.at(comIndexDMA)->getId() << ") выполнена" << endl;
					//if (workOnSBDMA && !RQ->empty())
					//{
					//	if (RQ->front() == convNum) RQ->pop_front();
					//}
					workDMA = false;
					if (workOnSBDMA) SB_DMA->releaseBus(); // отпустить СШ если работал на ней
					workOnSBDMA = false;
					commandVectDMA.at(comIndexDMA)->markDone();
					comIndexDMA++;
				}
			}

			if (comIndexDMA >= (commandVectDMA.size())) comIndexDMA = 0; // защита от убегания индекса
			currentCommandDMA = (commandVectDMA.at(comIndexDMA)); // получение текущей команды по индексу

			// поиск и получение невыполненной команды
			if (currentCommandDMA->isDone() == true)
			{
				int whileStopper = 0;
				while (currentCommandDMA->isDone())
				{
					comIndexDMA++;
					if (comIndexDMA >= (commandVectDMA.size())) comIndexDMA = 0; // защита от убегания индекса
					currentCommandDMA = (commandVectDMA.at(comIndexDMA)); // получение текущей команды по индексу
					whileStopper++;
					if (whileStopper > commandVectDMA.size())
					{
						busy = false;
						cout << "dma " << convNum << " done" << endl;
						return;
					}
				}
			}

			// проверка занятости СШ
			if (SB_DMA->isBusy())
			{
				waitDMA = true;
				//if (find(RQ->begin(), RQ->end(), convNum) == RQ->end())
				//{
				//	RQ->push_back(convNum);
				//}
				//drawNull()
				return;
			}
			else
			{
				if (!mp->waitCheck())
				{
					cout << "DMA" << convNum << " на СШ с командой " << currentCommandDMA->getId() << endl;
					workDMA = true;
					workOnSBDMA = true;
					waitDMA = false;
					SB_DMA->takeBus();
					remainingTimeDMA = (currentCommandDMA->getDuration()) * 2;
					return;
				}
				else
				{
					waitDMA = true;
					return;
				}
			}
		}
	};

	Conveyor CV1, CV2;
	DMAconv DMA1;

public:

	bool wait;	//находится ли процессор в режиме ожидания?

	Microprocessor(SystemBus* _SB_MP, CacheController* _CC_MP) :currentCommand(nullptr), commandVectMP(nullptr), SB_MP(_SB_MP), CC_MP(_CC_MP),
		requestSB(false), wait(false), work(false), countConv(1), comIndex(0)
	{
		CV1.init(SB_MP, CC_MP, 1, &requestDeque);
		CV2.init(SB_MP, CC_MP, 2, &requestDeque);
		DMA1.init(SB_MP, this, 3);//, &requestDeque);
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
					if (commandVectMP->at(comIndex).isDMA())
					{
						// если команда dma, поиск не dma
						while (commandVectMP->at(comIndex).isDMA())
						{
							DMA1.loadCommand(&commandVectMP->at(comIndex));
							cout << "command " << comIndex << "to DMA" << endl;
							comIndex++;
							if (comIndex >= (commandVectMP->size())) break; // защита от убегания индекса
						}
					}
					flag1 = !CV1.loadCommand(&commandVectMP->at(comIndex));
					cout << "command " << comIndex << "to CV1" << endl;
					comIndex++;
				}
			}
			if (!CV2.isBusy())
			{
				bool flag2 = true;
				while (flag2)
				{
					if (comIndex >= (commandVectMP->size())) break; // защита от убегания индекса
					if (commandVectMP->at(comIndex).isDMA())
					{
						// если команда dma, поиск не dma
						while (commandVectMP->at(comIndex).isDMA())
						{
							DMA1.loadCommand(&commandVectMP->at(comIndex));
							cout << "command " << comIndex << "to DMA" << endl;
							comIndex++;
							if (comIndex >= (commandVectMP->size())) break; // защита от убегания индекса
						}
					}
					flag2 = !CV2.loadCommand(&commandVectMP->at(comIndex));
					cout << "command " << comIndex << "to CV2" << endl;
					comIndex++;
				}
			}
		}
	}

	void commandAdder()
	{
		// если кто-то не работает
		// то получает работу!
		if (CV1.needNewComms())
		{
			cout << endl << "cv1 needcommands" << endl;
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
			cout << endl << "cv2 needcommands" << endl;
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
		DMA1.step();
		commandFeeder();

	}

	bool waitCheck()
	{
		bool flagWait = false;
		if (CV1.waitCV == true) flagWait = true;
		if (CV2.waitCV == true) flagWait = true;
		return flagWait;
	}

	void stepWait()
	{
		if (!requestDeque.empty())
		{
			// список запросов не пуст
			int candidate = requestDeque.front();
			if (candidate == 1 && !CV1.isWorking()) CV1.step();
			if (candidate == 2 && !CV2.isWorking()) CV2.step();
		}

		//bool zapretConv = false;
		//if (CV2.waitCV) zapretConv = true;
		//if (CV1.waitCV == true) CV1.step(zapretConv);
		//if (CV2.waitCV == true) CV2.step();

	}

	void stepWaitCC()
	{
		bool zapretConv = false;
		if (CV2.waitCV) zapretConv = true;
		if (!CV1.isBusy() && CV1.isWaitCache()) CV1.step(zapretConv);
		if (!CV2.isBusy() && CV2.isWaitCache()) CV2.step();
	}
};

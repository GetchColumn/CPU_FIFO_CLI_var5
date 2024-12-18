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
	//deque<int> requestVect;

	class Conveyor
	{
		vector<Command*> commandVectCV; // вектор указателей на указатели в векторе команд микропроцессора
		Command* currentCommandCV;	// указатель на текущую команду
		SystemBus* SB_CV;	// указатель на объект класса "системная шина"
		CacheController* CC_CV;	// указатель на объект класса "кэш-контроллер"
		//vector<Command> commVectCV;
		short convNum;	// номер конвеера
		bool requestSBCV; // запрос на использование системной шины
		bool workCV;     // признак работы конвеера
		int remainingTimeCV;	// оставшееся время работы конвеера
		bool workOnSBCV; // признак работы на СШ
		bool busy;	// признак того что у конвеера есть работа
		bool waitCache;	// признак того что конвеер ожидает загрузки команды в кэш
		int comIndexCV;	// индекс текущей команды
		deque<int> requestVectCV;	// структура данных с индексами н.к. команд

		void busyToggler()
		{
			if (remainingTimeCV == 1)
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

		Conveyor() :currentCommandCV(nullptr), commandVectCV(), SB_CV(nullptr), CC_CV(nullptr),
			convNum(0), requestSBCV(false), waitCV(false), workCV(false), busy(false), waitCache(false), remainingTimeCV(0), workOnSBCV(false), comIndexCV(0), requestVectCV()
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
				if (!(item->isDone() && item->getInCacheState()))
				{
					// команда не сделана
					i++;
				}
			}
			if (i <= 2)
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

		bool isWaitCache()
		{
			return waitCache;
		}

		void step()
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
						busyToggler();
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
				this->busyToggler();
				return;
			}

		}

	};

	Conveyor CV1, CV2;

public:

	bool wait;	//находится ли процессор в режиме ожидания?

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

		//commandAdder();

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
		if (CV1.waitCV == true) CV1.step();
		if (CV2.waitCV == true) CV2.step();

	}

	void stepWaitCC()
	{
		if (!CV1.isBusy() && CV1.isWaitCache()) CV1.step();
		if (!CV2.isBusy() && CV2.isWaitCache()) CV2.step();
	}
};

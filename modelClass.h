#pragma once
#include <vector>
#include <deque>
#include <algorithm>

#include "commanderClass.h"

using namespace std;

// System bus
class SystemBus
{
	bool busy; // ������� ���� ��� ��������� ���� ������
public:
	SystemBus() : busy(false)
	{}

	void takeBus() { busy = true; cout << "�� ������" << endl; }

	void releaseBus() { busy = false; cout << "�� �����������" << endl;}

	bool isBusy() const { return busy; }
};

// CacheController
class CacheController
{
private:
	SystemBus *SB_CC; // ��������� �� ������ ��������� ����
	bool requestSB = false;	// ����� �� �� ���������� ��?
	bool work = false;	// �������� �� ��
	const int durationCommand;	// ������������ ������ ��
	int remainingTime;	// ���������� ����� �� ��������� ������ ��
	int currentIndex;	// ������ ������� �������
	vector<Command> *CCTask;	// ������ ���� ������
	deque<int> indexes;	// ������� �� ������

public:
	CacheController(SystemBus* _SB_CC) : SB_CC(_SB_CC), requestSB(false), work(false), durationCommand(6), remainingTime(0), currentIndex(0), CCTask(nullptr), indexes({})
	{}

	// �������� ����������� ������ �����
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
		cout << "�� ������� ������ �� " << ind << endl;
		if (!CCTask->at(ind).getInCacheState()) indexes.push_back(ind);
	}

	bool isWorking()
	{
		return work;
	}

	// ���������� ������ ����
	void step()
	{
		if (work == true)
		{
			// ���� �� ��� ��������
			if (remainingTime == 1)
			{
				cout << "��������� � ��� (" << currentIndex << ")" << endl;
				// ���� ����� ������ �� �����
				CCTask->at(currentIndex).setInCache();
				indexes.pop_front();
				work = false;
				SB_CC->releaseBus();
				return;
			}
			else
			{
				// ������ �� - ���������� ���-�� ���������� ������ � ���������
				remainingTime--;
				//drawCC()
				return;
			}
		}
		else
		{
			// ���� �� �� ��������
			if (indexes.empty())
			{
				// ���� ������ �������� ������ ��� �������� ����
				//drawCCNull
				return;
			}
			else
			{
				// ���� ���� ������� ��� ��������
				currentIndex = indexes.at(0);
				if (SB_CC->isBusy())
				{
					// ���� �� ������
					//drawSBRequest
					requestSB = true;
					return;
				}
				else
				{
					cout << "�� ����� ������ ��� " << currentIndex << endl;
					// ���� �� ��������
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
	vector<Command> *commandVectMP; // ��������� �� ������ � ���������
	Command *currentCommand;	// ��������� �� ������� �������
	SystemBus *SB_MP;	// ��������� �� ������ ������ "��������� ����"
	CacheController *CC_MP;	// ��������� �� ������ ������ "���-����������"
	bool requestSB; // ���� ������� �� ������������� ��������� ����
	bool work;	// ������� ������ ���������������
//	int remainingTime;	// ���������� ����� ������ ��
//	bool workOnSB; // ������� ������ �� ��
	int countConv;	// ���������� ����������
	int comIndex;	// ������ ������� �������
	//deque<int> requestVect;

	class Conveyor
	{
		vector<Command*> commandVectCV; // ������ ���������� �� ��������� � ������� ������ ���������������
		Command* currentCommandCV;	// ��������� �� ������� �������
		SystemBus* SB_CV;	// ��������� �� ������ ������ "��������� ����"
		CacheController* CC_CV;	// ��������� �� ������ ������ "���-����������"
		//vector<Command> commVectCV;
		short convNum;	// ����� ��������
		bool requestSBCV; // ������ �� ������������� ��������� ����
		bool workCV;     // ������� ������ ��������
		int remainingTimeCV;	// ���������� ����� ������ ��������
		bool workOnSBCV; // ������� ������ �� ��
		bool busy;	// ������� ���� ��� � �������� ���� ������
		bool waitCache;	// ������� ���� ��� ������� ������� �������� ������� � ���
		int comIndexCV;	// ������ ������� �������
		deque<int> requestVectCV;	// ��������� ������ � ��������� �.�. ������

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
		bool waitCV;	//��������� �� ����� � ������ ��������?

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
			//		// ���� ������� �� ���������
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
					// ������� �� �������
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
			if (commandVectCV.empty()) return; // ���� ������ ���, �����

			if (workCV == true)
			{
				waitCache = false;
				if (remainingTimeCV != 1)
				{
					cout << "K" << convNum << " ��������~" << endl;
					remainingTimeCV--;
					//draw();
					this->busyToggler();
					return;
				}
				else
				{
					cout << "������� (" << commandVectCV.at(comIndexCV)->getId() << ") ���������" << endl;
					workCV = false;
					if (workOnSBCV) SB_CV->releaseBus(); // ��������� �� ���� ������� �� ���
					workOnSBCV = false;
					commandVectCV.at(comIndexCV)->markDone();
					comIndexCV++;

					if (!requestVectCV.empty())
					{
						// ���� �������� �� ����
						if (commandVectCV.at(requestVectCV.at(0))->getInCacheState())
						{
							comIndexCV = requestVectCV.at(0);
							requestVectCV.pop_front();
						}
					}
				}
			}

			if (comIndexCV >= (commandVectCV.size())) comIndexCV = 0; // ������ �� �������� �������
			//////////
			currentCommandCV = (commandVectCV.at(comIndexCV)); // ��������� ������� ������� �� �������

			// ����� � ��������� ������������� �������
			if (currentCommandCV->isDone() == true)
			{
				int whileStopper = 0;
				while (currentCommandCV->isDone())
				{
					comIndexCV++;
					////////
					if (comIndexCV >= (commandVectCV.size())) comIndexCV = 0; // ������ �� �������� �������
					currentCommandCV = (commandVectCV.at(comIndexCV)); // ��������� ������� ������� �� �������
					whileStopper++;
					if (whileStopper > commandVectCV.size())
					{
						busy = false;
						cout << "conv. " << convNum << " done" << endl;
						return;
					}
				}
			}

			// �������� ������� �� ���������� � ����
			if (!currentCommandCV->getInCacheState())
			{

				int whileStopper = 0;
				while (!(currentCommandCV->getInCacheState() && !currentCommandCV->isDone()))
				{
					// ���� ��� ������ � ������ ��� ������
					if ((requestVectCV.end() == find(requestVectCV.begin(), requestVectCV.end(), comIndexCV)) && !currentCommandCV->getInCacheState())
					{
						cout << "������ �"<< convNum <<" �� ������� " << commandVectCV.at(comIndexCV)->getId() << endl;
						requestVectCV.push_back(comIndexCV);
						CC_CV->load(currentCommandCV->getId());
					}
					comIndexCV++;
					if (comIndexCV >= (commandVectCV.size())) comIndexCV = 0; // ������ �� �������� �������
					//////////
					currentCommandCV = (commandVectCV.at(comIndexCV)); // ��������� ������� ������� �� �������
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

			// �������������, ���� �� �������
			if (!currentCommandCV->isDecoded())
			{
				cout << "�" << convNum <<" ������������� " << currentCommandCV->getId() << endl;
				//drawDecode()
				busy = true;
				waitCache = false;
				currentCommandCV->markDecoded();
				return;
			}
			// �������� ������� �� ��
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
					cout << "�"<< convNum << " �� �� � �������� " << currentCommandCV->getId() << endl;
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
				cout << "�" << convNum << " � �������� " << currentCommandCV->getId() << endl;
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

	bool wait;	//��������� �� ��������� � ������ ��������?

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
			// ���� ���-�� �� ��������

			if (!CV1.isBusy())
			{
				bool flag1 = true;
				while (flag1)
				{
					if (comIndex >= (commandVectMP->size())) break; // ������ �� �������� �������
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
					if (comIndex >= (commandVectMP->size())) break; // ������ �� �������� �������
					flag2 = !CV2.loadCommand(&commandVectMP->at(comIndex));
					cout << "command " << comIndex << "to CV2" << endl;
					comIndex++;
				}
			}
		}
	}

	void commandAdder()
	{
		// ���� ���-�� �� ��������
		// �� �������� ������!
		if (CV1.needNewComms())
		{
			cout << endl << "cv1 needcommands" << endl;
			bool flag1 = true;
			while (flag1)
			{
				if (comIndex >= (commandVectMP->size())) break; // ������ �� �������� �������
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
				if (comIndex >= (commandVectMP->size())) break; // ������ �� �������� �������
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
	//		// ����� ������
	//		cout << item.getId() << ") \t" << item.getDuration() << "(";
	//		if (item.getInCacheState() == 1) cout << "��� "; else cout << "�.� ";
	//		if (item.getUO() == 1) cout << "��"; else cout << "__";
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

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

	void takeBus() { busy = true; cout << "�� ������" << endl; }

	void releaseBus() { busy = false; cout << "�� �����������" << endl;}

	bool isBusy() const { return busy; }
};

// CacheController
class CacheController
{
private:
	SystemBus *SB_CC;
	bool requestSB = false;	//����� �� �� ���������� ��?
	bool work = false;	//�������� �� ��
	const int durationCommand;	//������������ ������ ��
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
	vector<Command> *commandVectMP;
	Command *currentCommand;
	SystemBus *SB_MP;
	CacheController *CC_MP;
	bool requestSB; // ������ �� ������������� ��
	bool wait;     //��������� �� ��������� � ������ ��������?
	bool work;     //�������� �� ���������?
	int remainingTime;	// ���������� ����� ������ ��
	bool workOnSB; // ������� ������ �� ��
	int convCount;	// ���������� ����������
	int comIndex;	// ������ ������� �������
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

			// ����� ������
			cout << item.getId() << ") \t" << item.getDuration() << "(";
			if (item.getInCacheState() == 1) cout << "��� "; else cout << "�.� ";
			if (item.getUO() == 1) cout << "��"; else cout << "__";
			cout << ")" << endl;
		}
	}

	void step()
	{
		if (work == true)
		{
			if (remainingTime != 1)
			{
				cout << "�� ��������~" << endl;
				remainingTime--;
				//draw();
				return;
			}
			else
			{
				cout << "������� (" << comIndex << ") ���������" << endl;
				work = false;
				if (workOnSB) SB_MP->releaseBus(); // ��������� �� ���� ������� �� ���
				workOnSB = false;
				commandVectMP->at(comIndex).markDone();
				comIndex++;
			}
		}

		if (comIndex >= (commandVectMP->size())) comIndex = 0; // ������ �� �������� �������
		//////////
		currentCommand = &(commandVectMP->at(comIndex)); // ��������� ������� ������� �� �������
		
		// ����� � ��������� ������������� �������
		if (currentCommand->isDone() == true)
		{
			int whileStopper = 0;
			while (currentCommand->isDone())
			{
				comIndex++;
				////////
				if (comIndex >= (commandVectMP->size())) comIndex = 0; // ������ �� �������� �������
				currentCommand = &(commandVectMP->at(comIndex)); // ��������� ������� ������� �� �������
				whileStopper++;
				if (whileStopper > commandVectMP->size())
				{
					cout << "conv.done" << endl;
					return;
					break; // conv done
				}
			}
		}

		// �������� ������� �� ���������� � ����
		if (!currentCommand->getInCacheState())
		{

			int whileStopper = 0;
			while (!(currentCommand->getInCacheState() && !currentCommand->isDone()))
			{
				// ���� ��� ������ � ������ ��� ������
				if ((requestVect.end() == find(requestVect.begin(), requestVect.end(), comIndex)) && !currentCommand->getInCacheState())
				{
					cout << "������ �� ������� " << comIndex << endl;
					requestVect.push_back(comIndex);
					CC_MP->load(currentCommand->getId());
				}
				comIndex++;
				if (comIndex >= (commandVectMP->size())) comIndex = 0; // ������ �� �������� �������
				//////////
				currentCommand = &(commandVectMP->at(comIndex)); // ��������� ������� ������� �� �������
				whileStopper++;
				if (whileStopper > commandVectMP->size())
				{
					cout << "CCfinder done" << endl;
					return;
					//break;
				}
			}

		}
		
		// �������������, ���� �� �������
		if (!currentCommand->isDecoded())
		{
			cout << "������������� " << currentCommand->getId() << endl;
			//drawDecode()
			currentCommand->markDecoded();
			return;
		}
		// �������� ������� �� ��
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
				cout << "�� �� �� � �������� " << currentCommand->getId() << endl;
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
			cout << "�� � �������� " << currentCommand->getId() << endl;
			// draw
			work = true;
			remainingTime = currentCommand->getDuration();
			return;
		}

	}

};

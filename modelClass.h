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
	//bool needFreeSB = false;	//����� �� �� ���������� ��?
	bool work = false;	//�������� �� ��
	const int durationCommand;	//������������ ������ ��
	int remainingTime;
	int currentIndex;
	vector<Command> CCTask;	// ������ ������
	deque<int> indexes;	// ������� �� ������

	CacheController(SystemBus* _SB_CC) : SB_CC(_SB_CC), work(false), durationCommand(6), remainingTime(0), currentIndex(0), CCTask({}), indexes({})
	{}

	// �������� ����������� ������ �����
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
	bool requestSB; // ������ �� ������������� ��
	bool wait;     //��������� �� ��������� � ������ ��������?
	bool work;     //�������� �� ���������?
	int remainingTime;	// ���������� ����� ������ ��
	bool workOnSB; // ������� ������ �� ��
	int convCount;	// ���������� ����������
	int comIndex;	// ������ ������� �������
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

			// ����� ������
			cout << item.getId() << ") \t" << item.getDuration() << "(";
			if (item.getInCacheState() == 1) cout << "��� "; else cout << "�.� ";
			if (item.getUO() == 1) cout << "��"; else cout << "__";
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
		currentCommand = &(commandVectMP.at(comIndex)); // ��������� ������� ������� �� �������
		
		// ����� � ��������� ������������� �������
		if (currentCommand->isDone() == true)
		{
			while (currentCommand->isDone())
			{
				comIndex++;
				currentCommand = &(commandVectMP.at(comIndex)); // ��������� ������� ������� �� �������
			}
		}

		// �������� ������� �� ���������� � ����
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

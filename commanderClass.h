#pragma once
#include <vector>

using namespace std;

// �����, �������������� ����� ������� ��� �������
class Command
{
	int id;	// ���������� ������������� �������
	int duration;	// ������������ � ������
	bool inCache;	// ������� ���������� � ���� 1- ���, 0- �.�.
	bool UO;	// ��� �������. 1-������� ��, 0- �� ������� ��
	bool floatType; // ������� ���� � ��������� ������
	bool done;	// ������� ������������� �������

public:
	Command()
	{
		id = 0;
		duration = 0;
		inCache = false;
		UO = false;
		done = false;
	}

	Command(int _id, int _duration, bool _inCache, bool _type)
	{
		id = _id;
		duration = _duration;
		inCache = _inCache;
		UO = _type;
		done = false;
	}

	int getId() const
	{
		return id;
	}
	int getDuration() const
	{
		return duration;
	}
	bool getInCacheState() const
	{
		return inCache;
	}
	bool getUO() const
	{
		return UO;
	}
	bool getStatus() const
	{
		return done;
	}
	void markDone()
	{
		done = true;
	}
	bool isDone()
	{
		return done;
	}
	bool isFloat()
	{
		return floatType;
	}
};

void genComm(vector<Command>& commList, const int commandCount, const int inCacheChance);
void printCommands(vector<Command> cmdVect);
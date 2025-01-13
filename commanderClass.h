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
	bool DMA;	// ������� ������� ������� �������
	bool decoded;	// ������� ���� ��� ������� ������������
	bool done;	// ������� ����������� �������

public:
	Command()
	{
		id = 0;
		duration = 0;
		inCache = false;
		UO = false;
		DMA = false;
		done = false;
		decoded = false;
	}

	Command(int _id, int _duration, bool _inCache, bool _type, bool _DMA)
	{
		id = _id;
		duration = _duration;
		inCache = _inCache;
		UO = _type;
		DMA = _DMA;
		done = false;
		decoded = false;
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
	bool isDecoded() const
	{
		return decoded;
	}
	bool getStatus() const
	{
		return done;
	}
	bool isDone() const
	{
		return done;
	}
	bool isDMA() const
	{
		return DMA;
	}
	void setInCache()
	{
		inCache = true;
	}
	void markDone()
	{
		done = true;
	}
	void markDecoded()
	{
		decoded = true;
	}
};

void genComm(vector<Command>& commList, const int commandCount, const int inCacheChance, const int dmaChance);
void getUserComm(vector<Command>& commList);
void printCommands(vector<Command> cmdVect);
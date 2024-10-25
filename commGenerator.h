#pragma once
#include <list>

using namespace std;

// �����, �������������� ����� ������� ��� �������
class Command
{
public:
	int id;	// ���������� ������������� �������
	int duration;	// ������������ � ������
	bool inCache;	// ������� ���������� � ���� 1- ���, 0- �.�.
	bool type;	// ��� �������. 1-������� ��, 0- �� ������� ��

	Command(int _id, int _duration, bool _inCache, bool _type)
	{
		id = _id;
		duration = _duration;
		inCache = _inCache;
		type = _type;
	}
};

void genComm(list<Command>& commList, const int commandCount, const int inCacheChance);
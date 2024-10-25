#include <list>
#include <random>
#include "commGenerator.h"

using namespace std;

void genComm(list<Command>& commList, const int commandCount, const int inCacheChance)
{
	random_device rd;
	mt19937 gen(rd()); // ����������� �������
	uniform_int_distribution<> dis(1, 100);

	for (int c = 0; c < commandCount; c++)
	{
		int randTaskNum = dis(gen);	// ����� ��� ����������� ���� ������
		int randInCache = dis(gen);	// ����� ��� ����������� ���������� � ����
		int randDuratNum = dis(gen);	// ����� ��� ����������� ������������ �������
		bool commInCache = 0;	// ������� ���������� ������� � ����
		int duration;
		bool type = false;

		// ����������, ��������� ������� � ���� ��� ���
		if (randInCache <= inCacheChance)
		{
			commInCache = true;
		}
		else
		{
			commInCache = 0;
		}

		// ���
		if (randTaskNum <= 50)
		{
			if (randDuratNum <= 60)
			{
				duration = 2;
			}
			else
			{
				duration = 1;
			}
			Command comm(c,duration,commInCache,type);
			commList.push_back(comm);
		}

		// ���
		else if (randTaskNum > 50 && randTaskNum <= 70)
		{
			if (randDuratNum <= 70)
			{
				duration = 5;
			}
			else if (randDuratNum > 70 && randDuratNum <= 90)
			{
				duration = 2;
			}
			else if (randDuratNum > 90)
			{
				duration = 1;
			}
			Command comm(c, duration, commInCache, type);
			commList.push_back(comm);
		}
		// ���
		else if (randTaskNum > 70 && randTaskNum <= 85)
		{
			if (randDuratNum <= 70)
			{
				duration = 2;
			}
			else if (randDuratNum > 70 && randDuratNum <= 90)
			{
				duration = 5;
			}
			else if (randDuratNum > 90)
			{
				duration = 1;
			}
			Command comm(c, duration, commInCache, type);
			commList.push_back(comm);
		}
		// ��
		else if (randTaskNum > 85)
		{
			type = true;

			if (randDuratNum <= 80)
			{
				duration = 2;
			}
			else
			{
				duration = 1;
			}
			Command comm(c, duration, commInCache, type);
			commList.push_back(comm);
		}

	}
}
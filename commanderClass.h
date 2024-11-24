#pragma once
#include <vector>

using namespace std;

// класс, представляющий собой команды для системы
class Command
{
	int id;	// уникальный идентификатор команды
	int duration;	// длительность в тактах
	bool inCache;	// признак нахождения в кэше 1- КЭШ, 0- Н.К.
	bool UO;	// тип команды. 1-требует СШ, 0- не требует СШ
	bool floatType; // признак типа с плавающей точкой
	bool done;	// признак выполненности команды

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
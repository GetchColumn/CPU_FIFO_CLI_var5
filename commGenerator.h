#pragma once
#include <list>

using namespace std;

// класс, представляющий собой команды для системы
class Command
{
public:
	int id;	// уникальный идентификатор команды
	int duration;	// длительность в тактах
	bool inCache;	// признак нахождения в кэше 1- КЭШ, 0- Н.К.
	bool type;	// тип команды. 1-требует СШ, 0- не требует СШ

	Command(int _id, int _duration, bool _inCache, bool _type)
	{
		id = _id;
		duration = _duration;
		inCache = _inCache;
		type = _type;
	}
};

void genComm(list<Command>& commList, const int commandCount, const int inCacheChance);
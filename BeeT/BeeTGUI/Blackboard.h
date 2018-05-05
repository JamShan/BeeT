#ifndef __BLACKBOARD_H__
#define __BLACKBOARD_H__

#include <string>
#include <vector>
#include <boost/any.hpp>
#include "Data.h"

class BTDecorator;

enum BBVarType
{
	BV_BOOL,
	BV_INT,
	BV_FLOAT,
	BV_STRING,
};

struct BBVar
{
	BBVarType type;
	std::string name;
	boost::any value;
	std::vector<BTDecorator*> decorators; // Decorators this BBVar is linked to
};

class Blackboard
{
public:
	Blackboard();
	Blackboard(Data& data);
	~Blackboard();

	void Serialize(Data& data)const;

	void CreateDummyVar();
	void RemoveVar(int id);

	void ChangeVarType(int varId, BBVarType type);
	BBVar* FindVar(const std::string& name);

private:

	// Helpers
	void SetDefaultTypeValue(BBVar* var);

public:
	std::vector<BBVar*> variables;

private:
	BBVarType lastTypeUsed = BV_BOOL;
	int dummyId = 0;
};
#endif // !__BLACKBOARD_H__


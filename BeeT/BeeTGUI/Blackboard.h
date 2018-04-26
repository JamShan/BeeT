#ifndef __BLACKBOARD_H__
#define __BLACKBOARD_H__

#include <string>
#include <vector>
#include <boost/any.hpp>

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
};

class Blackboard
{
public:
	Blackboard();
	~Blackboard();

	void CreateDummyVar();

	void SetLastTypeUsed(BBVarType type);

public:
	std::vector<BBVar*> variables;

private:
	BBVarType lastTypeUsed = BV_BOOL;
	int dummyId = 0;
};
#endif // !__BLACKBOARD_H__


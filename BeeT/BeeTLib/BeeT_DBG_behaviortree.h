#ifndef __BEET_DBG_BEHAVIORTREE_H__
#define __BEET_DBG_BEHAVIORTREE_H__

#include "beet_std.h"
#include <time.h>

typedef struct BeeT_dBT BeeT_dBT;
struct BeeT_dBT
{
	unsigned int uid;
	BEET_bool initialized;

	void* btBuffer; // Only used at the beginning to send the BT structure
	unsigned int dataToSendSize;
	dequeue* samples; // Changes of the BT that will be sent this Tick
	clock_t startTime;
};

BeeT_dBT*		BeeT_dBT_Init			(unsigned int uid, const char* buffer, unsigned int size);		// Constructor
BEET_bool		BeeT_dBT_HasDataToSend	(const BeeT_dBT* bt);											// Returns BEET_TRUE if there is new data to send, BEET_FALSE otherwise
int				BeeT_dBT_GetSampleData	(BeeT_dBT* bt, char** buf);													// Returns a buffer with the data ready to be sent. After calling this, dataToSendSize contains the buffer size.

double GetTimestamp(clock_t startTime); // Helper


// Blackboard variables change their value
void BeeT_dBT_bbBool(BeeT_dBT* bt, struct BBVar* var, BEET_bool newValue);
void BeeT_dBT_bbInt(BeeT_dBT* bt, struct BBVar* var, int newValue);
void BeeT_dBT_bbFloat(BeeT_dBT* bt, struct BBVar* var, float newValue);
void BeeT_dBT_bbString(BeeT_dBT* bt, struct BBVar* var, const char* newValue);

// Information to send to the Editor
// --------------------------------------------------------------------------------
// Samples
// --------------------------------------------------------------------------------

typedef enum SampleType SampleType;
enum SampleType
{
	BBVAR_CHANGED,
	NODE_RETURNS,
	NEW_CURRENT_NODE,
	DECORATOR_CONDITION
};

typedef struct BeeT_dSample BeeT_dSample;
struct BeeT_dSample
{
	SampleType type;
	double time;
};

struct BeeT_Serializer* BeeT_dSample_Serialize(BeeT_dSample* sample);

// Sample Types
// --------------------------------------------------------------------------------

typedef struct BeeT_sBBVar
{
	BeeT_dSample sample;
	enum BBVarType varType;
	char* name;
	void* oldValue;
	void* newValue;
}BeeT_sBBVar;

BeeT_sBBVar* BeeT_dBT_InitsBBVar();
void BeeT_dBT_BBVarSerialize(struct BeeT_Serializer* data, BeeT_sBBVar* sample);

#endif // !__BEET_DBG_BEHAVIORTREE_H__


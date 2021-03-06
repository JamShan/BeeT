#include "BeeT_blackboard.h"
#include <stdio.h>

// Forward declarations
BBVar* Blackboard_FindVar(BeeT_Blackboard* bb, const char* name);


BeeT_Blackboard* BeeT_Blackboard_Init(const BeeT_Serializer * data)
{
	BeeT_Blackboard* bb = (BeeT_Blackboard*)BEET_malloc(sizeof(BeeT_Blackboard));
	
	bb->FindVar = &Blackboard_FindVar;
	bb->vars = InitDequeue();

	int numVars = (int)BeeT_Serializer_GetArraySize(data, "blackboard");

	for (int i = 0; i < numVars; ++i)
	{
		BeeT_Serializer* varData = BeeT_Serializer_GetArray(data, "blackboard", i);
		BBVar* var = (BBVar*)BEET_malloc(sizeof(BBVar));
	
		const char* name = BeeT_Serializer_GetString(varData, "name");
		unsigned int length = strlen(name) + 1;
		var->name = (char*)BEET_malloc(length);
		strcpy(var->name, name);

		var->type = (BBVarType)BeeT_Serializer_GetInt(varData, "type");

		switch (var->type)
		{
		case BV_BOOL:
			var->data = BEET_malloc(sizeof(BEET_bool));
			*((BEET_bool*)var->data) = BeeT_Serializer_GetBool(varData, "value");	
			break;
		case BV_INT:
			var->data = BEET_malloc(sizeof(int));
			*((int*)var->data) = BeeT_Serializer_GetInt(varData, "value");
			break;
		case BV_FLOAT:
			var->data = BEET_malloc(sizeof(float));
			*((float*)(var->data)) = BeeT_Serializer_GetFloat(varData, "value");
			break;
		case BV_STRING:
			{
				const char* bvName = BeeT_Serializer_GetString(varData, "value");
				unsigned int bvLength = strlen(bvName) + 1;
				var->data = BEET_malloc(bvLength);
				strcpy(var->data, bvName);
			}	
			break;
		case BV_VECTOR2:
			var->data = BEET_malloc(sizeof(float2));
			*(float2*)(var->data) = BeeT_Serializer_GetFloat2(varData, "value");
			break;
		case BV_VECTOR3:
			var->data = BEET_malloc(sizeof(float3));
			*(float3*)var->data = BeeT_Serializer_GetFloat3(varData, "value");
			break;
		}
		BEET_free(varData);
		dequeue_push_back(bb->vars, var);
	}

	return bb;
}

void BeeT_Blackboard_Destroy(BeeT_Blackboard * self)
{
	if (self->vars)
	{
		node_deq* item = dequeue_head(self->vars);
		while (item)
		{
			BBVar* v = (BBVar*)item->data;
			BEET_free(v->name);
			switch (v->type)
			{
			case BV_BOOL:
				BEET_free((BEET_bool*)v->data);
				break;
			case BV_INT:
				BEET_free((int*)v->data);
				break;
			case BV_FLOAT:
				BEET_free((float*)v->data);
				break;
			case BV_STRING:
				BEET_free((char*)v->data);
				break;
			case BV_VECTOR2:
				BEET_free((float2*)v->data);
				break;
			case BV_VECTOR3:
				BEET_free((float3*)v->data);
				break;
			}
			item = item->next;
		}
		DestroyDequeue(self->vars);
	}
	BEET_free(self);
}

BBVar* Blackboard_FindVar(BeeT_Blackboard* bb, const char* name)
{
	node_deq* it = bb->vars->head;
	while (it)
	{
		BBVar* var = (BBVar*)it->data;
		if (strcmp(var->name, name) == 0)
			return var;
		else
			it = it->next;
	}
	return NULL;
}

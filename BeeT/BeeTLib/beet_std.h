#ifndef __beet_std_h__
#define __beet_std_h__

#include <stdio.h>
#include <string.h>

typedef enum {
	BEET_FALSE = 0,
	BEET_TRUE = 1
}BEET_bool;

#ifndef BEET_ASSERT
#include <assert.h>
#define BEET_ASSERT(_EXPR)	assert(_EXPR)
#endif

#include "beet_begin.h"

#ifdef __cplusplus
extern "C" {
#endif

// Memory ---------------------------------------------------------
#define BEET_memcpy memcpy

extern BEET_API void *BEET_malloc(size_t size);
extern BEET_API void  BEET_free(void* mem);
extern BEET_API void *BEET_realloc(void* mem, size_t size);
extern BEET_API void *BEET_calloc(size_t num, size_t size);

typedef void * (*BeeT_Malloc_Function)(size_t);
typedef void(*BeeT_Free_Function)(void*);
typedef void * (*BeeT_Realloc_Function)(void* mem, size_t size);
typedef void * (*BeeT_Calloc_Function)(size_t, size_t);
//------------------------------------------------------------------


// Dequeue ---------------------------------------------------------
typedef struct BeeT_dequeue dequeue;
struct BeeT_dequeue
{
	struct node_dequeue* head;
	struct node_dequeue* tail;
};

typedef struct node_dequeue node_deq;
struct node_dequeue
{
	struct node_dequeue* next;
	struct node_dequeue* prev;
	void* data;
};

BEET_API dequeue* InitDequeue();
BEET_API void DestroyDequeue(dequeue* deq);

BEET_API BEET_bool dequeue_is_empty(dequeue* d);
BEET_API void dequeue_clear(dequeue* d); // This causes memory leaks. Free the content of each item before calling clear
BEET_API size_t dequeue_size(dequeue* d);

BEET_API void dequeue_push_front(dequeue* d, void* value);
BEET_API void dequeue_push_back(dequeue* d, void* value);

BEET_API void dequeue_pop_front(dequeue* d);
BEET_API void dequeue_pop_back(dequeue* d);

BEET_API void* dequeue_front(dequeue* d);
BEET_API void* dequeue_back(dequeue* d);

BEET_API node_deq* dequeue_head(dequeue* d);
BEET_API node_deq* dequeue_tail(dequeue* d);

// ------------------------------------------------------------------
typedef struct float2 float2;
struct float2
{
	float x;
	float y;
};

BEET_API BEET_bool Float2Equal(const float2* a, const float2* b);

typedef struct float3 float3;
struct float3
{
	float x;
	float y;
	float z;
};

BEET_API BEET_bool Float3Equal(const float3* a, const float3* b);
// Map --------------------------------------------------------------



//-------------------------------------------------------------------
#ifdef __cplusplus
}
#endif
#include "beet_end.h"



#endif // !__beet_std_h__

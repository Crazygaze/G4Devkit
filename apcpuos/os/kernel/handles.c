#include "handles.h"

typedef void (*handles_destroyFunc)(void* data);
typedef struct HandleTypeFuncs {
	const char* name;
	handles_destroyFunc dtr;
} HandleTypeFuncs;

/******************************************************************************/
//                      Type 0 (unused)
/******************************************************************************/
static void handles_type0_dtr(void* data)
{
}

/******************************************************************************/
//                      Mutex handling functions
/******************************************************************************/
static void handles_mutex_dtr(void* data)
{
}

/******************************************************************************/
//                      Thread handling functions
/******************************************************************************/
void handles_thread_dtr(void* data);

/******************************************************************************/
//                      Handle functions
/******************************************************************************/
static HandleTypeFuncs handleFuncs[kHandleType_MAX] =
{
	{ "N/A"    , handles_type0_dtr }, // Index 0 is not used
	{ "MUTEX"  , handles_mutex_dtr },
	{ "THREAD" , handles_thread_dtr }
};

// Index 0 is not used, so we can use the value 0 as "No handle"
static Handle handles[OS_MAXHANDLES];

// This one doesn't exist in the enum, so the caller code doesn't use it by
// mistake when creating handles.
// The caller can still use 0 explicitly for some things tough.
#define kHandleType_NONE 0

/*!
 * Sets a slot as free for use
 */
static void handles_resetSlot(Handle* ptr)
{
	ptr->owner = NULL;
	ptr->data = NULL;
	ptr->type = kHandleType_NONE;
}

void handles_init(void)
{
	for (int i = 0; i < OS_MAXHANDLES; i++) {
		handles[i].id.os.index = i;
		handles_resetSlot(&handles[i]);
	}
}

HANDLE handles_create(struct PCB* owner , HandleType type, void* data)
{
	// Note: Index 0 is not used, since we use that index as "No Handle"
	for (int i = 1; i < OS_MAXHANDLES; i++) {
		Handle* ptr = &handles[i];
		if (ptr->type == kHandleType_NONE) {
			ptr->id.os.counter++;
			ptr->owner = owner;
			ptr->type = type;
			ptr->data = data;

			OS_LOG(
				"Handles: Created handle %u for pcb %p, type %d(%s), data %p",
				ptr->id.user, owner , type, handleFuncs[type].name, ptr->data);

			return ptr->id.user;
		}
	}

	OS_ERR(
		"Handles: Failed to create handle for pcb %p, type %d(%s)",
		owner, type, handleFuncs[type].name);
	return INVALID_HANDLE;
}

//
// Gets the handle struct, given the HANDLE value
static Handle* getHandle(HANDLE h)
{
	HandleHelperUnion u;
	u.user = h;
	u16 index = u.os.index;

	if (!(index > 0 && index < OS_MAXHANDLES))
		return NULL;
	Handle* ptr = &handles[index];
	if (ptr->type == kHandleType_NONE)
		return NULL;

	return ptr;
}

#pragma dontwarn 323
// For internal use
static void handles_destroyImpl(Handle* ptr)
{
	OS_LOG("Handles: Destroying handle for pcb %p, type %d(%s), data %p",
		ptr->owner, ptr->type, handleFuncs[ptr->type].name, ptr->data);
		
	Handle tmp = *ptr;
	
	// Reseting this before calling the destructor function, so that if the
	// destructor function calls handles functions, it doesn't cause problems
	handles_resetSlot(ptr);
	handleFuncs[tmp.type].dtr(tmp.data);
}
#pragma popwarn

bool handles_destroy(HANDLE h, struct PCB* owner)
{
	Handle* ptr = getHandle(h);
	if (!ptr || !owner|| (owner && ptr->owner != owner)) {
		OS_ERR("Handles: Tried to destroy invalid handle %p", h);
		return false;
	}

	handles_destroyImpl(ptr);
	return true;
}

void handles_destroyPrcHandles(struct PCB* owner)
{
	// Note: Index 0 is not used, since we use that index as "No Handle"
	for (int i = 1; i < OS_MAXHANDLES; i++) {
		Handle* ptr = &handles[i];
		if (owner == NULL || ptr->owner == owner)
			handles_destroyImpl(ptr);
	}
}

void* handles_getData(HANDLE h, struct PCB* owner, HandleType type)
{
	Handle* ptr = getHandle(h);
	if (ptr &&
			(owner == NULL || (ptr->owner == owner) &&
			(type == 0 || ptr->type == type)))
		return ptr->data;
	else
		return NULL;
}

void handles_setData(HANDLE h, void* data)
{
	Handle* ptr = getHandle(h);
	if (!ptr)
		return;
	ptr->data = data;
}


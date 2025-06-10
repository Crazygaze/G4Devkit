#include "handles.h"

#define HANDLE_ACCESS_ALL 0
#define HANDLE_ACCESS_NONE 0xFFFFFFFF

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

void handles_init(void)
{
	for (int i = 0; i < OS_MAXHANDLES; i++) {
		handles[i].id.os.index = i;
		handles[i].pid = HANDLE_ACCESS_NONE;
	}
}

HANDLE handles_create(u32 pid, int type, void* data)
{
	// Note: Index 0 is not used, since we use that index as "No Handle"
	for(int i=1; i<OS_MAXHANDLES; i++) {
		Handle* ptr = &handles[i];
		if (ptr->pid == HANDLE_ACCESS_NONE) {
			ptr->id.os.counter++;
			ptr->pid = pid;
			ptr->type = type;
			ptr->data = data;
			
			OS_LOG(
				"Handles: Created handle %u for pid %d, type %d(%s), data %Xh",
				ptr->id.user, ptr->pid, ptr->type, handleFuncs[ptr->type].name,
				ptr->data);
				
			return ptr->id.user;
		}
	}

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
	if (ptr->pid == HANDLE_ACCESS_NONE)
		return NULL;

	return ptr;
}

#pragma dontwarn 323
// For internal use
static void handles_destroyImpl(Handle* ptr)
{
	
	OS_LOG("Handles: Destroying handle for pid %d, type %d(%s), data %Xh",
		ptr->pid, ptr->type, handleFuncs[ptr->type].name, ptr->data);
		
	void* data = ptr->data;
	
	// Reseting this before calling the destructor function, so that if the
	// destructor function calls handles functions, it doesn't cause problems
	ptr->pid = HANDLE_ACCESS_NONE;
	ptr->data = NULL;
	handleFuncs[ptr->type].dtr(data);
}
#pragma popwarn

bool handles_destroy(HANDLE h, u32 pid)
{
	Handle* ptr = getHandle(h);
	if (!ptr || (pid && ptr->pid!=pid)) {
		OS_WRN("Handles: Tried to destroy invalid handle %u", h);
		return FALSE;		
	}
	
	handles_destroyImpl(ptr);
	return TRUE;
}

void handles_destroyPrcHandles(u32 pid)
{
	// Note: Index 0 is not used, since we use that index as "No Handle"
	for (int i = 1; i < OS_MAXHANDLES; i++) {
		Handle* ptr = &handles[i];
		if (ptr->pid == pid)
			handles_destroyImpl(ptr);
	}
}

void* handles_getData(HANDLE h, u32 pid, int type)
{
	Handle* ptr = getHandle(h);
	if (!ptr || (pid && (ptr->pid != pid)) || (type && type != ptr->type))
		return NULL;

	return ptr->data;
}

void handles_setData(HANDLE h, void* data)
{
	Handle* ptr = getHandle(h);
	if (!ptr)
		return;
	ptr->data = data;
}


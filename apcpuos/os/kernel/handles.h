#ifndef _os_handles_h
#define _os_handles_h

#include "../os_config.h"
#include "appsdk/os_shared/process_shared.h"

/*
HANDLE values are built from the index into the handles array, and a `counter`
that is incremented every time a handle is created reusing a previously used
index
*/
typedef union HandleHelperUnion {

	// The components of an handle
	struct {
		u16 counter;
		u16 index;
	} os;
	
	// The handle representation itself
	HANDLE user;
	
} HandleHelperUnion;

typedef struct Handle {
	union HandleHelperUnion id;
	
	// Process that owns this handle
	// 0 means that any process can use this handle
	struct PCB* owner;
	
	void* data;
	
	// Type of handle (e.g THREAD, MUTEX, etc)
	// This also tells if the slot is in use. If this is "NONE", then it's not
	// in use
	u8 type;
} Handle;

/*!
 * Initializes the handles system
 */
void handles_init(void);

/*!
 * Create a new handle of the specified type
 * 
 * \param owner
 *	Process that will own this handle, or NULL for no specific owner
 *
 * \param type
 *	Handle type to create
 *
 * \param data
 *	Data what will be connected with the handle.
 *	Optionally, this can be set later with handles_setData
 *
 * \return Returns INVALID_HANDLE if it couldn't create the new handle
 */
HANDLE handles_create(struct PCB* owner, HandleType type, void* data);

/*!
 * Destroys the specified handle
 *
 * \param h
 *		Handle to destroy
 *
 * \param owner
 *	If NULL, the handle will be destroyed regardless of the owner.
 *	If not NULL, then the handle will only be destroyed if the owner matches.
 *
 * \return true if the handle was destroyed, false otherwise
 */
bool handles_destroy(HANDLE h, struct PCB* owner);

/*!
 * Destroys all handles that belong to the specified process
 *
 * \param owner
 *	If NULL, it will destroy all handles. If specified, it will destroy only
 *	the handles owned this this process.
 */
void handles_destroyPrcHandles(struct PCB* owner);

/*!
 * Gets the handle data, validating the handle with the parameters passed
 *
 * \param HANDLE
 *	Handle to use
 *
 * \param owner
 *	If NULL, then no check is done for the owner. 
 *	If not NULL, then the handle must belong to this process, otherwise it fails
 *	the tests.
 *
 * \param type
 *	The type the handle should be. If it doesn't match, it fails the test.
 *	If 0, no type check is made
 *
 * \return
 *	The data associated with the handle, or NULL on error.
 *	Note that if the associated data itself is NULL, then there is no way to 
 *	distinguish between success or failure.
 */
void* handles_getData(HANDLE h, struct PCB* owner, HandleType type);

/*!
 * Sets the data associated with the handle.
 * This is the same as specifying it in the handles_create.
 * It is also provided as seperate function since depending on the calling code,
 * sometimes is more convenient to associate the data after everything else is
 * setup.
 */
void handles_setData(HANDLE h, void* data);

#endif


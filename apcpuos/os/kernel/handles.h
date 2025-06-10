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
	// 0 - Any process can use this handle
	// 0xFFFFFFFF - Handle invalid/doesn't exit/not used
	u32 pid;
	
	// Type of handle (e.g THREAD, MUTEX, etc)
	u8 type;
	void* data;
} Handle;

/*!
 * Initializes the handles system
 */
void handles_init(void);

/*!
 * Create a new handle of the specified type
 * 
 * \param pid
 *	Process that will own this handle
 *
 * \param type
 *	Handle type
 *
 * \param data
 *	Data what will be connected with the handle.
 *	Optionally, this can be set later with handles_setData
 *
 * \return Returns INVALID_HANDLE if it couldn't create the new handle
 */
HANDLE handles_create(u32 pid, int type, void* data);

/*!
 * Destroys the specified handle
 *
 * \param h
 *		Handle to destroy
 *
 * \param pid
 *		If not 0, the handle will only be destroyed if the owner pid matches,
 * 		otherwise nothing happens.
 *
 * \return true if the handle was destroyed, false otherwise
 */
bool handles_destroy(HANDLE h, u32 pid);

/*!
 * Destroys all handles that belong to the specified process
 */
void handles_destroyPrcHandles(u32 pid);

/*!
 * Gets the handle data, validating the handle with the parameters passed
 *
 * \param HANDLE
 *		Handle to use
 *
 * \param pid
 *		If not 0, the handle must belong to this process, otherwise it fails
 *		the tests.
 *
 * \param type
 *		The type the handle should be. If it doesn't match, it fails the test.
 *		If 0, no type check is made
 *
 * \return
 * 	The data associated with the handle, or NULL on error.
 *	Note that if the associated data itself is NULL, then there is no way to 
 * distinguish between success or failure.
 */
void* handles_getData(HANDLE h, u32 pid, int type);

/*!
 * Sets the data associated with the handle.
 * This is the same as specifying it in the handles_create.
 * It is also provided as seperate function, since depending on the calling
 * code, sometimes is more convenient to associate the data after everything
 * else is setup.
 */
void handles_setData(HANDLE h, void* data);

#endif


/*******************************************************************************
 * OS Handles code
 ******************************************************************************/

#ifndef _APCPU_HANDLES_H_
#define _APCPU_HANDLES_H_

#include "kerneldefs.h"
#include "stddef_shared.h"
#include "stdint_shared.h"
#include "appsdk/kernel_shared/syscalls_shared.h"

// How many handles the OS supports
#define MAXHANDLES 128

/*
HANDLE values are built from the index into the handles array, and a "counter"
that is increment every time a handle is created reusing a previously used
index
*/

typedef union HandleHelperUnion
{
	struct
	{
		u16 counter;
		u16 index;
	} os;
	HANDLE user;
} HandleHelperUnion;

typedef struct Handle
{
	union HandleHelperUnion id;
	/*
	union
	{
		HandleHelper os;
		HANDLE user;
	} id;
	*/
	
	// Process that owns this handle
	// 0 - Any process can use this handle
	// 1..254 - Only the process with this id can use this handle
	// 255 - Handle invalid/doesn't exit/not used
	u8 pid;
	u8 type;
	void* data;
} Handle;

/*!
 * Initializes the handles system
 */
void handles_init(void);

/*!
 * Create a new handle, of the specified type
 * \param pid
 *		Process that will own this handle
 * \param type
 *		Handle type
 * \param handleData
 *		Data what will be connected with the handle.
 *		Optionally, this can be set later with handles_setData
 * \return Returns INVALID_HANDLE if it couldn't create the new handle
 */
HANDLE handles_create(u8 pid, int type, void* data);

/*!
 * Destroys the specified handle
 * \param h
 *		Handle to destroy
 * \param pid
 *		If not 0, the handle will only be destroyed if the owner pid matches,
 * 		otherwise nothing happens.
 */
bool handles_destroy(HANDLE h, uint8_t pid);

/*! Destroys all handles that belong to the specified process
*/
void handles_destroyPrcHandles(uint8_t pid);

/*!
 * Gets the handle data, validating the handle with the parameters passed
 * \param HANDLE
 *		Handle to use
 * \param pid
 *		If not 0, the handle must belong to this process, otherwise it fails
 *		the tests.
 * \param type
 *		The type the handle should be. If it doesn't match, it fails the test.
 *		If 0, no type check is made
 */
void* handles_getData(HANDLE h, uint8_t pid, int type);

/*! Sets the data associated with the handle
 * This is the same as specifying it in the handles_create.
 * Provided as seperate function, since depending on the calling code, sometimes
 * is more convenient to associate the data after everything else is setup.
 */
void handles_setData(HANDLE h, void* data);

#endif

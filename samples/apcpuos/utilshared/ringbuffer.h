#ifndef _czRingBuffer_h_
#define _czRingBuffer_h_


#include "utilsharedconfig.h"


//////////////////////////////////////////////////////////////////////////
// No need to change stuff bellow
//////////////////////////////////////////////////////////////////////////


#define RINGBUFFER_FLAG_OWNSBUFFER 1
#define RINGBUFFER_FLAG_GROWABLE 2

typedef struct RingBuffer
{
	char* buf;
	int capacity;
	int fillcount;
	int readpos;
	int writepos;
	unsigned char flags;
} RingBuffer;


/*
 * Initializes the ring buffer.
 * \param ring RingBuffer to initialize
 * \param userbuffer
 *		If specified, this buffer will be used instead of allocating dynamic memory. Also,
 *		the ring buffer capacity will not be allowed to grow.
 *	\param usersize
 *		If "userbuffer" is specified, then this is the size of that buffer
 */
void ringbuffer_create(RingBuffer* ring);


/*
 * Initializes the ring buffer with a supplied buffer.
 * When using this function to initialize a ring buffer, it means the capacity is fixed, since it's using a
 * user supplied buffer.
 *
 * \param ring RingBuffer to initialize
 * \param userbuffer
 *		The buffer to use
 *	\param usersize
 *		The size of the supplied buffer
 */
void ringbuffer_createWithCustom(RingBuffer* ring, void* userbuffer, int usersize);

/*
 * Frees any resources used by the ring buffer
 */
void ringbuffer_destroy(RingBuffer* ring);

/*
 * Grows the capacity, if the buffer is growable, and if the current capacity is lower.
 * \return true on success, false on error (buffer can't grow, or out of memory)
 */
bool ringbuffer_reserve(RingBuffer* ring, int size);

/*
 * Tells if the ring buffer can grow its capacity
 */
#define ringbuffer_canGrow(ring) ((ring)->flags&RINGBUFFER_FLAG_GROWABLE)

/*
 * Tells if the ring buffer owns its buffer
 */
#define ringbuffer_ownsBuffer(ring) (ring->flags&RINGBUFFER_FLAG_OWNSBUFFER)

/*
 * Tells how much data is available to read
 */
#define ringbuffer_getUsedSize(ring) ((ring)->fillcount)

/*
 * Tells the total how much data the ring buffer can hold without having to resizing its internal buffer
 */
#define ringbuffer_getCapacity(ring) ((ring)->capacity)

/*
 * Tells how much data we can write
 */
#define ringbuffer_getFreeSize(ring) ((ring)->capacity - (ring)->fillcount)

void ringbuffer_clear(RingBuffer* ring, bool releaseMemory);

//! Writes data to the buffer
/*!
 * \param ptr Data to write
 * \param size how many bytes to write
 * \return number of bytes written. 0 means an error (see bellow)
 *
 * \note If necessary, the buffer will expand to accept the new data
 *
 * \note If there is not enough space in the buffer and it can't grow, either because it's
 * using a user supplied buffer, or an out memory trying to allocate more space, it will return 0
 *
 */
int ringbuffer_write(RingBuffer* ring, const void *ptr, int size);


//! Simulates a write, and give you the pointers you can use to write the data yourself
/*!
 * \param writeSize How many bytes to write
 * \param ptr1 (out) Where you get the pointer you can write to
 * \param ptr1size (out) How many bytes you can write to pr1
 * \param ptr2 (out) If not NULL, it contains the wrap around part you can write to
 * \param ptr2size
 * Because writing the specified size can cause wrap around, ptr2 (and ptr2size) gives you the
 * second part after wrapping.
 */
int ringbuffer_customWrite(RingBuffer* ring, int writeSize, void** ptr1, int* ptr1size, void **ptr2, int* ptr2size);

//! Reads data from the buffer
/*
 *
 * \param ptr where you get the data
 * \param size how many bytes to read
 * \return the number of bytes actually read
 */
int ringbuffer_read(RingBuffer* ring, void *ptr, int size);

//!
/*!
 * \brief Returns the internal buffers pointers for the specified read operation
 *
 * This function is useful if you want to avoid some memory copying when reading from the buffer.
 *
 * \param readsize How many bytes you want to read
 * \param ptr1 Where you'll get the pointer to the first part
 * \param ptr1size Here you'll get how many bytes you can read from ptr1
 * \param ptr2 Where you'll get the pointer to the first part, or NULL if everything is available with ptr1
 * \param ptr2size Here you'll get how many bytes you can read from ptr2
 * \param size size you want to read
 * \return
 *   Number of bytes you can actually read from the returned pointers. If it's smaller than the readsize,
 *   it means there wasn't enough bytes available in the buffer.
 *
 * \note
 *  This returns pointers to the internal memory, for read only, which is handy in some cases to avoid some unnecessary memory copying
 *  Therefore you shouldn't keep those pointers. Those pointers should be considered valid only until another method is called on the buffer
 */
int ringbuffer_customRead(RingBuffer* ring, int readsize, void **ptr1, int *ptr1size, void **ptr2, int *ptr2size);

//! \brief Returns the internal buffer for read pointer
/*!
 * You can use this function to get a pointer to the internal buffer, without worrying about
 * the wrap around, as the buffer returned is what you can use without wrapping around.
 * \param ptr Where you get the pointer
 * \return The number of bytes you can read from the pointer, or 0 if nothing to read.
 */
 int ringbuffer_getReadPointer(RingBuffer* ring, void **ptr);

 //!
 /*!
  * \brief Returns the internal buffers for the specified read operation, without removing data from the buffer
  *
  * \param readsize How many bytes you want to read
  * \param ptr1 Where you'll get the pointer to the first part
  * \param ptr1size Here you'll get how many bytes you can read from ptr1
  * \param ptr2 Where you'll get the pointer to the first part, or NULL if everything is available with ptr1
  * \param ptr2size Here you'll get how many bytes you can read from ptr2
  * \param size size you want to read
  * \return
  *   Number of bytes you can actually read from the returned pointers. If it's smaller than the readsize,
  *   it means there wasn't enough bytes available in the buffer.
  *
  * \note
  *  This returns pointers to the internal memory, for read only, which is handy in some cases to avoid some unnecessary memory copying
  *  Therefore you shouldn't keep those pointers. Those pointers should be considered valid only until another method is called on the buffer
  */
 int ringbuffer_getReadPointers(RingBuffer* ring, int readsize, void **ptr1, int *ptr1size, void **ptr2, int *ptr2size);

/*
 * Peek at data, without removing it from the buffer.
 * Returns true if successful, false if there is enough data in the buffer
 */
bool ringbuffer_peek(RingBuffer* ring, void* buf, int size);

//! Skips the specified amount of bytes
/*!
 * \param size How many bytes to dump
 * \return how many bytes were actually dumped
 */
int ringbuffer_skip(RingBuffer* ring, int size);

#endif

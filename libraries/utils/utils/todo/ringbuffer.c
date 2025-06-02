#include "ringbuffer.h"


//////////////////////////////////////////////////////////////////////////
// No need to change stuff bellow
//////////////////////////////////////////////////////////////////////////

void ringbuffer_create(RingBuffer* ring)
{
	utilshared_memset(ring, 0, sizeof(*ring));
	ring->flags = RINGBUFFER_FLAG_OWNSBUFFER|RINGBUFFER_FLAG_GROWABLE;
}

void ringbuffer_createWithCustom(RingBuffer* ring, void* userbuffer, int usersize)
{
	utilshared_memset(ring, 0, sizeof(*ring));
	if (userbuffer) {
		ring->buf = userbuffer;
		ring->capacity = usersize;
	}
}

void ringbuffer_destroy(RingBuffer* ring)
{
	if (ring->buf && ringbuffer_ownsBuffer(ring)) {
		utilshared_free(ring->buf);
	}
	utilshared_memset(ring, 0, sizeof(*ring));
}

void ringbuffer_clear(RingBuffer* ring, bool releaseMemory)
{
	ring->fillcount = 0;
	ring->readpos = 0;
	ring->writepos = 0;
	if (releaseMemory && ringbuffer_ownsBuffer(ring)) {
		utilshared_free(ring->buf);
		ring->buf = 0;
		ring->capacity = 0;
	}
}

int ringbuffer_write(RingBuffer* ring, const void *ptr, int size)
{
	void *p1, *p2;
	int s1, s2;
	int writeSize = ringbuffer_customWrite(ring, size, &p1, &s1, &p2, &s2);
	if (!writeSize)
		return 0;

	utilshared_memcpy(p1, ptr, s1);
	if (p2)
		utilshared_memcpy(p2, ((char*)ptr) + s1, s2);

	return size;
}

int ringbuffer_customWrite(RingBuffer* ring, int writeSize, void** ptr1, int* ptr1size, void **ptr2, int* ptr2size)
{
	if (ringbuffer_getFreeSize(ring) < writeSize) {
		if (!ringbuffer_reserve(ring, ring->capacity + writeSize - ringbuffer_getFreeSize(ring)))
			return 0;
	}

	if ( (ring->capacity - ring->writepos) >= writeSize) // In case we don't need to wrap around
	{
		*ptr1 = &ring->buf[ring->writepos];
		*ptr1size = writeSize;
		*ptr2 = NULL;
		*ptr2size = 0;
	}
	else
	{
		*ptr1 = &ring->buf[ring->writepos];
		*ptr1size = ring->capacity - ring->writepos;
		*ptr2 = &ring->buf[0];
		*ptr2size = writeSize - *ptr1size;
	}

	ring->fillcount += writeSize;
	ring->writepos += writeSize;
	if (ring->writepos >= ring->capacity)
		ring->writepos -= ring->capacity;

	return writeSize;
}


bool ringbuffer_reserve(RingBuffer* ring, int size)
{
	if (ring->capacity>= size)
		return true;

	if (!ringbuffer_ownsBuffer(ring) || !ringbuffer_canGrow(ring))
		return false;

	char *newbuf = (char*)utilshared_malloc(size);
	if (!newbuf)
		return false;

	if (ring->buf)
	{
		//! Copy existing data to the new buffer
		if (ring->fillcount)
		{
			int len = min(ring->fillcount, ring->capacity - ring->readpos);
			utilshared_memcpy(&newbuf[0], &ring->buf[ring->readpos], len);
			if (len != ring->fillcount)
			{
				utilshared_memcpy(&newbuf[len], &ring->buf[0], ring->fillcount - len);
			}
		}

		utilshared_free(ring->buf);
	}

	ring->buf = newbuf;
	ring->readpos = 0;
	ring->writepos = ring->fillcount;
	ring->capacity = size;
	return true;
}

int ringbuffer_getReadPointer(RingBuffer* ring, void **ptr)
{
	int todo = min(ring->fillcount, ring->capacity - ring->readpos);
	*ptr = &ring->buf[ring->readpos];
	return todo;
}

int ringbuffer_getReadPointers(RingBuffer* ring, int readsize, void **ptr1, int *ptr1size, void **ptr2, int *ptr2size)
{
	const int todo = min(ring->fillcount, readsize);

	if (todo == 0)
	{
		*ptr1 = 0;
		*ptr1size = 0;
		*ptr2 = 0;
		*ptr2size = 0;
		return 0;
	}

	if (ring->readpos + todo <= ring->capacity)
	{
		*ptr1 = &ring->buf[ring->readpos];
		*ptr1size = todo;
		if (ptr2)
			*ptr2 = 0;
		if (ptr2size)
			*ptr2size = 0;
	}
	else
	{
		*ptr1 = &ring->buf[ring->readpos];
		*ptr1size = ring->capacity - ring->readpos;
		if (ptr2)
			*ptr2 = &ring->buf[0];
		if (ptr2size)
			*ptr2size = todo - *ptr1size;
	}

	return todo;
}

int ringbuffer_read(RingBuffer* ring, void *ptr, int size)
{
	void* p1, *p2;
	int s1, s2;
	int done = ringbuffer_customRead(ring, size, &p1, &s1, &p2, &s2);

	if (s1)
	{
		utilshared_memcpy(ptr, p1, s1);
		if (s2)
		{
			utilshared_memcpy((char*)ptr + s1, p2, s2);
		}
	}

	return done;
}

int ringbuffer_customRead(RingBuffer* ring, int readsize, void **ptr1, int *ptr1size, void **ptr2, int *ptr2size)
{
	int done = ringbuffer_getReadPointers(ring, readsize, ptr1, ptr1size, ptr2, ptr2size);
	ring->fillcount -= done;
	ring->readpos += done;
	if (ring->readpos >= ring->capacity)
		ring->readpos -= ring->capacity;
	return done;
}

bool ringbuffer_peek(RingBuffer* ring, void* buf, int size)
{
	void* ptr1;
	void* ptr2;
	int size1, size2;
	int done = ringbuffer_getReadPointers(ring, size, &ptr1, &size1, &ptr2, &size2);
	if (done != size)
		return false;

	utilshared_memcpy(buf, ptr1, size1);
	if (ptr2)
		utilshared_memcpy((char*)(buf)+size1, ptr2, size2);
	return true;
}
 
int ringbuffer_skip(RingBuffer* ring, int size)
{
	if (size >= ring->fillcount)
	{
		int done = ring->fillcount;
		ringbuffer_clear(ring,false);
		return done;
	}
	else if (size <= ring->capacity - ring->readpos)
	{
		ring->fillcount -= size;
		ring->readpos += size;
		if (ring->readpos >= ring->capacity)
			ring->readpos = 0;
		return size;
	}
	else
	{
		ring->fillcount -= size;
		ring->readpos = size - (ring->capacity - ring->readpos);
		return size;
	}
}

#if TEST_RINGBUFFER

#define ringbuffer_writeVal(ring,var) ringbuffer_write(ring, &var, sizeof(var))
#define ringbuffer_readVal(ring,var) ringbuffer_read(ring, &var, sizeof(var))

void ringbuffer_write8(RingBuffer* ring, unsigned char val)
{
	utilshared_check(ringbuffer_writeVal(ring, val)==sizeof(val));
}

unsigned char ringbuffer_read8(RingBuffer* ring)
{
	unsigned char val;
	utilshared_check(ringbuffer_readVal(ring, val)==sizeof(val));
	return val;
}

void ringbuffer_write32(RingBuffer* ring, unsigned int val)
{
	utilshared_check(ringbuffer_writeVal(ring, val)==sizeof(val));
}

unsigned int ringbuffer_read32(RingBuffer* ring)
{
	unsigned int val;
	utilshared_check(ringbuffer_readVal(ring, val)==sizeof(val));
	return val;
}

void test_ringbuffer()
{
	RingBuffer ring;
	char buf[5000];
	//ringbuffer_create(&ring);
	ringbuffer_createWithCustom(&ring, buf, sizeof(buf));

	const int iterations=1000;

	for(int i=0; i<iterations; i++) {
		ringbuffer_write8(&ring, (unsigned char)i);
		ringbuffer_write32(&ring, i);
	}

	int i=0;
	while(ringbuffer_getUsedSize(&ring))
	{
		utilshared_check(ringbuffer_read8(&ring)==(unsigned char)i);
		utilshared_check(ringbuffer_read32(&ring)==i);
		i++;
	}

	utilshared_check(i==iterations);

}
#endif


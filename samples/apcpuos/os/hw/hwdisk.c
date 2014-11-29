/*******************************************************************************
* Disk Controller driver
*******************************************************************************/

#include "hwdisk.h"
#include "kernel/kerneldebug.h"
#include <string_shared.h>

//
// IRQ Reasons
//
#define HW_DKC_IRQREASON_FINISHED 0 // Operation finished
#define HW_DKC_IRQREASON_MOUNT 1    // Disk was mounted
#define HW_DKC_IRQREASON_UNMOUNT 2  // Disk was unmounted

//
// IRQ Errors
//
#define HW_DKC_ERROR_NOMEDIA 1
#define HW_DKC_ERROR_BUSY 2
#define HW_DKC_ERROR_WRITEPROTECTED 3
#define HW_DKC_ERROR_INVALIDSECTOR 4
#define HW_DKC_ERROR_UNKNOWN 5

typedef struct hw_dkc_Disk {
	u32 writeCount; // How many write operations
	u32 writeBytes; // How many bytes written	
	u32 readCount; // How many read operations
	u32 readBytes; // How many bytes read
	u32 numSectors;
	u32 sectorSize;
	u32 status;	
} hw_dkc_Disk;

typedef struct hw_dkc_Drv {
	hw_Drv base;
	// Extra fields required by the device goes below...
	hw_dkc_Disk disks[HW_DKC_MAXDISKS];
} hw_dkc_Drv;

static hw_dkc_Drv drv;
static void hw_dkc_irqHandler(u32 reason, u32 data1, u32 data2);
static bool hw_dkc_query(u32 diskNum);

#define hw_dkc_isWriteProtected(dsk) \
	(dsk->status & HW_DKC_FLAG_WRITEPROTECTED)
	
#define hw_dkc_isReading(dsk) \
	(dsk->status & HW_DKC_FLAG_READING)
	
#define hw_dkc_isWriting(dsk) \
	(dsk->status & HW_DKC_FLAG_WRITING)
	
#define hw_dkc_isBusy(dsk)         \
	(dsk->status &                  \
		(HW_DKC_FLAG_READING |     \
		 HW_DKC_FLAG_WRITING)      \
		)

#define hw_dkc_isPresent(dsk) \
	(dsk->status & HW_DKC_FLAG_PRESENT)
	
#define hw_dkc_canRead(dsk)         \
	((dsk->status &                  \
		(HW_DKC_FLAG_READING |      \
		 HW_DKC_FLAG_WRITING |      \
		 HW_DKC_FLAG_PRESENT)       \
		) == (HW_DKC_FLAG_PRESENT))
		
#define hw_dkc_canWrite(dsk)            \
	((dsk->status &                      \
		(HW_DKC_FLAG_WRITEPROTECTED |   \
		 HW_DKC_FLAG_READING |          \
		 HW_DKC_FLAG_WRITING |          \
		 HW_DKC_FLAG_PRESENT)           \
		) == (HW_DKC_FLAG_PRESENT))

#define hw_dkc_setFlag(dsk, flag) \
	((dsk)->status = (dsk)->status | (flag))
	
#define hw_dkc_clearFlag(dsk, flag) \
	((dsk)->status = (dsk)->status & (~(flag)))
	

hw_Drv* hw_dkc_ctor(hw_BusId busid)
{
	drv.base.irqHandler = hw_dkc_irqHandler;
	hw_dkc_setIRQMode(TRUE);

	for(u32 diskNum=0; diskNum<HW_DKC_MAXDISKS; diskNum++) {
		hw_dkc_Disk* dsk = &drv.disks[diskNum];
		krn_bootLog("\n");
		krn_bootLog("  Disk %d: ", diskNum);
		if (hw_dkc_query(diskNum)) {
			krn_bootLog("%3.2fMB (sectors=%d sectorSize=%d)..."
				, ((float)dsk->numSectors*dsk->sectorSize) / (1024.0*1024.0)
				, dsk->numSectors, dsk->sectorSize);
		} else {
			krn_bootLog("No media detected...");		
		}
	}
	
	return &drv.base;
}

void hw_dkc_dtor(hw_Drv* drv)
{
	// TODO : Fill me
}

static hw_dkc_Disk* hw_dkc_getDisk(u32 diskNum)
{
	kernel_check(diskNum<HW_DKC_MAXDISKS);
	return &drv.disks[diskNum];
}

static void hw_dkc_irqHandler(u32 reason, u32 data1, u32 data2)
{
	u32 diskNum = data1;
	hw_dkc_Disk* dsk = hw_dkc_getDisk(diskNum);
	
	KERNEL_DEBUG("DKC IRQ %d, %d, %d. Current status=0x%08x", reason, data1,
		data2, dsk->status);

	switch(reason) {
	case HW_DKC_IRQREASON_FINISHED:
		
		// Because we can call the "sync" versions of read/write, the flags will
		// be reset at that point, and thus this check doesn't really apply
		//kernel_check(hw_dkc_isBusy(dsk));
		
		// If the operation failed, it was some unknown error, since typical
		kernel_check(data2==HWERR_SUCCESS);
		hw_dkc_clearFlag(dsk, HW_DKC_FLAG_READING|HW_DKC_FLAG_WRITING);
		break;
	case HW_DKC_IRQREASON_MOUNT:
		kernel_check( !hw_dkc_isPresent(dsk) );
		kernel_check( hw_dkc_query(diskNum) );
		KERNEL_DEBUG("Disk %d mounted: %3.2fMB (sectors=%d sectorSize=%d)"
			, diskNum
			, ((float)dsk->numSectors*dsk->sectorSize) / (1024.0*1024.0)
			, dsk->numSectors, dsk->sectorSize);
		break;
	case HW_DKC_IRQREASON_UNMOUNT:
		kernel_check( hw_dkc_isPresent(dsk) );
		hw_dkc_clearFlag( dsk, HW_DKC_FLAG_PRESENT);
		KERNEL_DEBUG("Disk %d unmounted", diskNum);
		break;
	}
	
	KERNEL_DEBUG("    New status=0x%08x", dsk->status);
}

static bool hw_dkc_query(u32 diskNum)
{
	hw_dkc_Disk* dsk = hw_dkc_getDisk(diskNum);
	memset(dsk, 0, sizeof(*dsk));		

	hw_HwiData hwi;
	hwi.regs[0] = diskNum;
	HWERROR err = hw_hwiFull(HWBUS_DKC, HW_DKC_FUNC_QUERY, &hwi);
	if (err==HW_DKC_ERROR_NOMEDIA) {
		return FALSE;
	} else {
		dsk->status = hwi.regs[0];
		hw_dkc_setFlag(dsk, HW_DKC_FLAG_PRESENT);
		dsk->numSectors = hwi.regs[1];
		dsk->sectorSize = hwi.regs[2];
		return TRUE;
	}
}

void hw_dkc_sync(u32 diskNum)
{
	hw_dkc_Disk* dsk = hw_dkc_getDisk(diskNum);
	while(hw_dkc_isBusy(dsk)) {
		hw_dkc_query(diskNum);
	}
}

void hw_dkc_read_sync(u32 diskNum, u32 sectorNum, char* data, int size)
{
	hw_dkc_read(diskNum, sectorNum, data, size);
	hw_dkc_sync(diskNum);
}

void hw_dkc_write_sync(u32 diskNum, u32 sectorNum, const char* data, int size)
{
	hw_dkc_write(diskNum, sectorNum, data, size);
	hw_dkc_sync(diskNum);
}

void hw_dkc_read(u32 diskNum, u32 sectorNum, char* data, int size)
{
	hw_dkc_Disk* dsk = hw_dkc_getDisk(diskNum);
	kernel_check(hw_dkc_canRead(dsk));
	
	hw_HwiData hwi;
	hwi.regs[0] = (sectorNum << 8) | diskNum;
	hwi.regs[1] = (u32)data;
	hwi.regs[2] = size;
	HWERROR err = hw_hwiFull(HWBUS_DKC, HW_DKC_FUNC_READSECTOR, &hwi);
	kernel_check(err==HWERR_SUCCESS);
	hw_dkc_setFlag(dsk, HW_DKC_FLAG_READING);
}

void hw_dkc_write(u32 diskNum, u32 sectorNum, const char* data, int size)
{
	hw_dkc_Disk* dsk = hw_dkc_getDisk(diskNum);
	kernel_check(hw_dkc_canWrite(dsk));
	
	hw_HwiData hwi;
	hwi.regs[0] = (sectorNum << 8) | diskNum;
	hwi.regs[1] = (u32)data;
	hwi.regs[2] = size;
	HWERROR err = hw_hwiFull(HWBUS_DKC, HW_DKC_FUNC_WRITESECTOR, &hwi);
	kernel_check(err==HWERR_SUCCESS);
	hw_dkc_setFlag(dsk, HW_DKC_FLAG_WRITING);
}

u32 hw_dkc_getFlags(u32 diskNum)
{
	hw_dkc_Disk* dsk = hw_dkc_getDisk(diskNum);
	kernel_check(hw_dkc_canWrite(dsk));
	return dsk->status;
}

void hw_dkc_setCustomFlags(u32 diskNum, u32 flags)
{
	hw_dkc_Disk* dsk = hw_dkc_getDisk(diskNum);
	kernel_check(hw_dkc_canWrite(dsk));
	hw_dkc_setFlag(dsk, flags & (~HW_DKC_INTERNAL_FLAGS));
}

void hw_dkc_clearFlags(u32 diskNum, u32 flags)
{
	hw_dkc_Disk* dsk = hw_dkc_getDisk(diskNum);
	kernel_check(hw_dkc_canWrite(dsk));
	hw_dkc_clearFlag(dsk, flags & HW_DKC_INTERNAL_FLAGS);
}


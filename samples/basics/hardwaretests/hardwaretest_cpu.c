#include "hwcpu.h"
#include "hardwaretest_cpu.h"
#include <hwscreen.h>
#include <assert.h>

typedef struct CpuDriver
{
	Driver base;
} CpuDriver;

static CpuDriver cpuDriver;

void cpu_handleGeneric(u32 data0, u32 data1, u32 data2, u32 data3)
{
	always_assert(0);
}

InterruptHandler cpuHandlers[HWCPU_INTERRUPT_MAX] =
{
	&cpu_handleGeneric,
	&cpu_handleGeneric,
	&cpu_handleGeneric,
	&cpu_handleGeneric,
	&cpu_handleGeneric
};

static void hardwareTest_cpu(void);
void hardwareTest_cpu_init(DeviceTest* data)
{
	cpuDriver.base.handlers = cpuHandlers;
	cpuDriver.base.numHanders = sizeof(cpuHandlers)/sizeof(InterruptHandler);
	data->driver = &cpuDriver.base;
	data->testFunc = &hardwareTest_cpu;
}

static void hardwareTest_cpu(void)
{
	scr_printf("CPU Tests\n");
	cpu_enableIRQ();
	cpu_disableIRQ();
	cpu_enableIRQ();
	u32 ram = cpu_getRamAmount();
	check(ram>0 && (ram%1024==0));
	scr_printf("	RAM = %d bytes (%d Kbytes)\n", ram, ram/1024);
	check(cpu_getIRQQueueSize()==0);
}


#include <errno.h>
#include <kernel.h>
#include <libmc.h>
#include <malloc.h>
#include <string.h>
#include <stdio.h>
#include <usbhdfsd-common.h>

#include "sysman/sysinfo.h"

#include "main.h"
#include "ident.h"
#include "system.h"
#include "DeviceSupport.h"

static int DevicePollingThreadID = -1, MainThreadID = -1;
static volatile int DevicePollingThreadCommand;
static void *DevicePollingThreadStack = NULL;
int UsbReadyStatus                    = 0;

enum DEVICE_POLLING_THREAD_COMMANDS
{
    DEVICE_POLLING_THREAD_CMD_NONE = 0,
    DEVICE_POLLING_THREAD_CMD_STOP
};

static int McUnitStatus[2];
static int MassUnitStatus[1];

static int IsMcUnitReady(int unit)
{
    int type, free, format, result;

    type = MC_TYPE_NONE;
    if (mcGetInfo(unit, 0, &type, &free, &format) == 0)
    {
        mcSync(0, NULL, &result);

        result = (type == MC_TYPE_PS2) ? 1 : 0;
    }
    else
        result = 0;

    return result;
}

static int IsMassUnitReady(int unit)
{
    return UsbReadyStatus;
}

int GetHasDeviceSufficientSpace(const char *device, int unit, const struct RequiredFileSpaceStat *RequiredSpaceStats, unsigned int NumFileEntries)
{
    int result;

    if (!strcmp(device, "mc"))
    {
        result = HasMcUnitSufficientSpace(unit, RequiredSpaceStats, NumFileEntries);
    }
    else if (!strcmp(device, "mass"))
    {
        result = HasMassUnitSufficientSpace(unit, RequiredSpaceStats, NumFileEntries);
    }
    else if (!strcmp(device, "host"))
    {
        result = HasHostUnitSufficientSpace(unit, RequiredSpaceStats, NumFileEntries);
    }
    else
        result = -ENODEV;

    return result;
}

int HasMcUnitSufficientSpace(int unit, const struct RequiredFileSpaceStat *RequiredSpaceStats, unsigned int NumFileEntries)
{
    unsigned int TotalRequiredSpace, AvailableSpace, NumFiles, NumDirectories, i;
    int type, free, format, result;

    type = MC_TYPE_NONE;
    if (mcGetInfo(unit, 0, &type, &free, &format) == 0)
    {
        mcSync(0, NULL, &result);

        if (type == MC_TYPE_PS2)
        {
            NumFiles = NumDirectories = 0;
            TotalRequiredSpace        = 0;

            for (i = 0; i < NumFileEntries; i++)
            {
                if (RequiredSpaceStats[i].IsFile)
                {
                    NumFiles++;
                    TotalRequiredSpace += RequiredSpaceStats[i].length;
                }
                else
                    NumDirectories++;
            }

            /* I do not like this, but the Sony documentation implies that we can (And have to?) assume that the memory card has a cluster size of 2. */
            AvailableSpace = free * 1024;
            TotalRequiredSpace += (NumFiles + NumDirectories + 1) / 2;
            TotalRequiredSpace += NumDirectories * 2;

            result = AvailableSpace >= TotalRequiredSpace;
        }
        else
            result = -ENXIO;
    }
    else
        result = 0;

    return result;
}

int HasMassUnitSufficientSpace(int unit, const struct RequiredFileSpaceStat *RequiredSpaceStats, unsigned int NumFileEntries)
{
    return 1;
}

int HasHostUnitSufficientSpace(int unit, const struct RequiredFileSpaceStat *RequiredSpaceStats, unsigned int NumFileEntries)
{
    return 1;
}

static void DevicePollingThread(void)
{
    int done;

    done = 0;
    while (!done)
    {
        //Process commands.
        if (DevicePollingThreadCommand != DEVICE_POLLING_THREAD_CMD_NONE)
        {
            if (DevicePollingThreadCommand == DEVICE_POLLING_THREAD_CMD_STOP)
            {
                WakeupThread(MainThreadID);
                done = 1;
                continue;
            }

            DevicePollingThreadCommand = DEVICE_POLLING_THREAD_CMD_NONE;
        }

        //Update the status of all units of all devices.
        MassUnitStatus[0] = IsMassUnitReady(0);
        McUnitStatus[0]   = IsMcUnitReady(0);
        McUnitStatus[1]   = IsMcUnitReady(1);
    }
}

int StartDevicePollingThread(void)
{
    DevicePollingThreadCommand = DEVICE_POLLING_THREAD_CMD_NONE;
    MainThreadID               = GetThreadId();

    DevicePollingThreadStack   = memalign(64, 0x800);
    return (DevicePollingThreadID = SysCreateThread(&DevicePollingThread, DevicePollingThreadStack, 0x800, NULL, 0x78));
}

int StopDevicePollingThread(void)
{
    DevicePollingThreadCommand = DEVICE_POLLING_THREAD_CMD_STOP;
    SleepThread(); //Wait for acknowledgement.

    if (DevicePollingThreadID >= 0)
    {
        TerminateThread(DevicePollingThreadID);
        DeleteThread(DevicePollingThreadID);
        DevicePollingThreadID = -1;
    }

    if (DevicePollingThreadStack != NULL)
    {
        free(DevicePollingThreadStack);
        DevicePollingThreadStack = NULL;
    }

    return 0;
}

int GetIsDeviceUnitReady(const char *device, int unit)
{
    int result;

    if (strcmp(device, "mc") == 0)
    {
        result = McUnitStatus[unit];
    }
    else if (strcmp(device, "mass") == 0)
    {
        result = MassUnitStatus[unit];
    }
    else if (strcmp(device, "host") == 0)
    {
        result = 1;
    }
    else
        result = -ENODEV;

    return result;
}

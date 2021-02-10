#include <kernel.h>
#include <fileio.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#include "system.h"

extern void *_gp;

int GetBootDeviceID(void)
{
    static int BootDevice = -2;
    char path[256];
    int result;

    if (BootDevice < BOOT_DEVICE_UNKNOWN)
    {
        getcwd(path, sizeof(path));

        if (!strncmp(path, "mc0:", 4))
            result = BOOT_DEVICE_MC0;
        else if (!strncmp(path, "mc1:", 4))
            result = BOOT_DEVICE_MC1;
        else if (!strncmp(path, "mass:", 5) || !strncmp(path, "mass0:", 6))
            result = BOOT_DEVICE_MASS;
#ifdef DSNET_HOST_SUPPORT
        else if (!strncmp(path, "host:", 5) || !strncmp(path, "host0:", 5))
            result = BOOT_DEVICE_HOST;
#endif
        else
            result = BOOT_DEVICE_UNKNOWN;

        BootDevice = result;
    }
    else
        result = BootDevice;

    return result;
}

int RemoveFolder(const char *path)
{
    fio_dirent_t dirent;
    char *fullpath;
    int fd, result;

    result = 0;
    if ((fd = fioDopen(path)) >= 0)
    {
        while (fioDread(fd, &dirent) > 0 && result >= 0)
        {
            if (!strcmp(dirent.name, ".") || !strcmp(dirent.name, ".."))
                continue;

            if ((fullpath = malloc(strlen(path) + strlen(dirent.name) + 2)) != NULL)
            {
                sprintf(fullpath, "%s/%s", path, dirent.name);

                if (FIO_S_ISDIR(dirent.stat.mode))
                {
                    result = RemoveFolder(fullpath);
                }
                else
                {
                    result = fioRemove(fullpath);
                }

                free(fullpath);
            }
        }

        fioDclose(fd);

        if (result >= 0)
            fioRmdir(path);
    }
    else
        result = fd;

    return result;
}

int SysCreateThread(void *function, void *stack, unsigned int StackSize, void *arg, int priority)
{
    ee_thread_t ThreadData;
    int ThreadID;

    ThreadData.func             = function;
    ThreadData.stack            = stack;
    ThreadData.stack_size       = StackSize;
    ThreadData.gp_reg           = &_gp;
    ThreadData.initial_priority = priority;
    ThreadData.attr = ThreadData.option = 0;

    if ((ThreadID = CreateThread(&ThreadData)) >= 0)
    {
        if (StartThread(ThreadID, arg) < 0)
        {
            DeleteThread(ThreadID);
            ThreadID = -1;
        }
    }

    return ThreadID;
}

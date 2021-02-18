/*
	platform.c	- Contains platform-specific functions.
*/

#include <errno.h>
#include <windows.h>

#include "platform.h"

int GetUsername(char *buffer, unsigned int BufferSize)
{
    DWORD lpnSize;
    lpnSize = BufferSize;
    return (GetUserNameA(buffer, &lpnSize) == 0 ? EIO : 0);
}

int GetLocalhostName(char *buffer, unsigned int BufferSize)
{
    DWORD lpnSize;
    lpnSize = BufferSize;
    return (GetComputerNameA(buffer, &lpnSize) == 0 ? EIO : 0);
}

// macros for converting between an integer and a BCD number
#ifndef btoi
#define btoi(b) ((b) / 16 * 10 + (b) % 16) // BCD to int
#endif
#ifndef itob
#define itob(i) ((i) / 10 * 16 + (i) % 10) // int to BCD
#endif

/* Converts the specified value to a value that looks the same in base 16. E.g. It converts 2012 in decimal to 0x2012 in hexadecimal. */
static unsigned short int ConvertToBase16(unsigned short int value)
{
    unsigned short int result;

    result = value + value / 10 * 0x06;
    result += value / 100 * 0x60;
    return (result + value / 1000 * 0x600);
}

unsigned int GetSystemDate(void)
{
    SYSTEMTIME SystemTime;
    GetSystemTime(&SystemTime);

    return (((unsigned int)ConvertToBase16(SystemTime.wYear)) << 16 | ConvertToBase16(SystemTime.wMonth) << 8 | ConvertToBase16(SystemTime.wDay));
}

unsigned int GetFileCreationDate(const char *path)
{
    HANDLE hFile;
    FILETIME CreationTime;
    SYSTEMTIME CreationSystemTime;

    if ((hFile = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL)) != INVALID_HANDLE_VALUE)
    {
        GetFileTime(hFile, &CreationTime, NULL, NULL);
        CloseHandle(hFile);

        FileTimeToSystemTime(&CreationTime, &CreationSystemTime);
    }
    else
        GetSystemTime(&CreationSystemTime);

    return (((unsigned int)ConvertToBase16(CreationSystemTime.wYear)) << 16 | ConvertToBase16(CreationSystemTime.wMonth) << 8 | ConvertToBase16(CreationSystemTime.wDay));
}

int GetCurrentWorkingDirectory(char *buffer, unsigned int BufferSize)
{
    return (GetCurrentDirectoryA(BufferSize, buffer) == 0 ? EIO : 0);
}

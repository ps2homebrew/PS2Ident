#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <kernel.h>
#include <sifcmd.h>
#include <libmc.h>

#include "main.h"
#include "ps1.h"
#include "OSDInit.h"

struct PS1DRV{
	char ver[32];
	int major;
	int minor;
	int region;
};

static struct PS1DRV ps1drv = {"", 0, 0, 0};
static char ps1ver_uni[] = "rom0:PS1VER?";

int PS1DRVInit(void)
{
	const char *pChar;
	int fd, result;

	ps1drv.ver[0] = '\0';

	fd = _ps2sdk_open("rom0:PS1ID", O_RDONLY);

	if(fd < 0)
		return -1;

	_ps2sdk_read(fd, ps1drv.ver, sizeof(ps1drv.ver));
	_ps2sdk_close(fd);

	pChar = ps1drv.ver;
	ps1drv.major = atoi(pChar);

	for(pChar = ps1drv.ver; *pChar >= '0' && *pChar <= '9'; pChar++)
	{};

	if(*pChar == '.')
	{
		ps1drv.minor = atoi(pChar + 1);

		for( ; *pChar >= '0' && *pChar <= '9'; pChar++)
		{};

		result = 0;
		ps1drv.region = *pChar;
	}
	else
	{	//Missing dot
		ps1drv.region = 0;
		ps1drv.major = 0;
		ps1drv.minor = 0;
		result = -1;
	}

	if(result == 0)
	{
		if(OSDGetPS1DRVRegion(&ps1ver_uni[11]) == 0 || (fd = _ps2sdk_open(ps1ver_uni, O_RDONLY)) < 0)
			fd = _ps2sdk_open("rom0:PS1VER", O_RDONLY);

		if(fd >= 0)
		{
			result = _ps2sdk_read(fd, ps1drv.ver, sizeof(ps1drv.ver) - 1);
			_ps2sdk_close(fd);

			//NULL-terminate, only if non-error
			ps1drv.ver[result >= 0 ? result : 0] = '\0';
		}
	}

	//The original always returned -1. But we'll return something more meaningful.
	return result;
}

const char *PS1DRVGetVersion(void)
{
	if(ps1drv.ver[0] == '\0')
	{	//rom0:PS1VER is missing (not present in the earliest ROMs)
		return(OSDGetConsoleRegion() == CONSOLE_REGION_JAPAN ? "1.01" : "1.10");
	}

	return ps1drv.ver;
}

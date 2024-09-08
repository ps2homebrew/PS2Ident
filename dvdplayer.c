#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <kernel.h>
#include <sifcmd.h>
#include <libmc.h>

#include "main.h"
#include "dvdplayer.h"
#include "OSDInit.h"

#define DVD_PLAYER_VER_LEN 32

struct DVDPlayer
{
    char ver[DVD_PLAYER_VER_LEN];
    int major, minor;
    int region;
};

static struct DVDPlayer ROMDVDPlayer;

static char dvdid[]  = "rom1:DVDID?";
static char dvdver[] = "rom1:DVDVER?";

extern char ConsoleROMVER[ROMVER_MAX_LEN];

int DVDPlayerInit(void)
{
    const char *pChar;
    char id[32];
    int fd, result;

    ROMDVDPlayer.ver[0] = '\0';
    ROMDVDPlayer.minor  = -1;
    ROMDVDPlayer.major  = -1;
    ROMDVDPlayer.region = -1;

    if (OSDGetDVDPlayerRegion(&dvdid[10]) == 0 || (fd = _ps2sdk_open(dvdid, O_RDONLY)) < 0)
        fd = _ps2sdk_open("rom1:DVDID", O_RDONLY);

    if (fd < 0)
    { //Not having a DVD player is not an error.

        //0100J and 0101J has no ROM DVD Player. This rule is copied from the HDD Browser, as it is absent in the normal browser.
        if (ConsoleROMVER[0] == '0' && ConsoleROMVER[1] == '1' && ConsoleROMVER[2] == '0' && (ConsoleROMVER[3] == '0' || ConsoleROMVER[3] == '1') && ConsoleROMVER[4] == 'J')
            ROMDVDPlayer.region = 'J';
        //I guess, this would be another possibility for such a case, but not used in practice:
        //	ROMDVDPlayer.region = -2;
        if (ConsoleROMVER[5] == 'Z' && ConsoleROMVER[4] == 'T') ROMDVDPlayer.region = 'J'; //namco arcade also lacks this
        return 0;
    }

    _ps2sdk_read(fd, id, sizeof(id));
    _ps2sdk_close(fd);

    ROMDVDPlayer.major = atoi(id);

    for (pChar = id; *pChar >= '0' && *pChar <= '9'; pChar++)
    {
    };

    if (*pChar == '.')
    {
        ROMDVDPlayer.minor = atoi(pChar + 1);

        for (; *pChar >= '0' && *pChar <= '9'; pChar++)
        {
        };

        result              = 0;
        ROMDVDPlayer.region = *pChar;
    }
    else
    { //Missing dot
        ROMDVDPlayer.major  = 0;
        ROMDVDPlayer.minor  = 0;
        ROMDVDPlayer.region = 0;
        result              = -1;
    }

    if (result == 0)
    {
        if (OSDGetDVDPlayerRegion(&dvdver[11]) == 0 || (fd = _ps2sdk_open(dvdver, O_RDONLY)) < 0)
            fd = _ps2sdk_open("rom1:DVDVER", O_RDONLY);

        if (fd >= 0)
        {
            result = _ps2sdk_read(fd, ROMDVDPlayer.ver, sizeof(ROMDVDPlayer.ver));
            _ps2sdk_close(fd);

            //NULL-terminate, only if non-error
            ROMDVDPlayer.ver[result >= 0 ? result : 0] = '\0';
        }
    }

    //The original always returned -1. But we'll return something more meaningful.
    return (result >= 0 ? 0 : -1);
}

const char *DVDPlayerGetVersion(void)
{
    //DVD Player Update does not exist. Use DVD ROM version.
    return ROMDVDPlayer.ver;
}

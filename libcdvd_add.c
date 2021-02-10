#include <kernel.h>
#include <sifrpc.h>
#include <string.h>
#include <libcdvd.h>

#include "libcdvd_add.h"

static unsigned char MECHACON_CMD_S36_supported = 0, MECHACON_CMD_S27_supported = 0;

//Initialize add-on functions. Currently only retrieves the MECHACON's version to determine what sceCdAltGetRegionParams() should do.
int cdInitAdd(void)
{
    int result, status, i;
    u8 MECHA_version_data[3];
    unsigned int MECHA_version;

    //Like how CDVDMAN checks sceCdMV(), do not continuously attempt to get the MECHACON version because some consoles (e.g. DTL-H301xx) can't return one.
    for (i = 0; i <= 100; i++)
    {
        if ((result = sceCdMV(MECHA_version_data, &status)) != 0 && ((status & 0x80) == 0))
        {
            MECHA_version              = MECHA_version_data[2] | ((unsigned int)MECHA_version_data[1] << 8) | ((unsigned int)MECHA_version_data[0] << 16);
            MECHACON_CMD_S36_supported = (0x5FFFF < MECHA_version); //v6.0 and later
            MECHACON_CMD_S27_supported = (0x501FF < MECHA_version); //v5.2 and later
            return 0;
        }
    }

    //	printf("Failed to get MECHACON version: %d 0x%x\n", result, status);

    return -1;
}

/*
	 This function provides an equivalent of the sceCdGetRegionParams function from the newer CDVDMAN modules. The old CDVDFSV and CDVDMAN modules don't support this S-command.
	It's supported by only slimline consoles, and returns regional information (e.g. MECHACON version, MG region mask, DVD player region letter etc.).
*/
int sceCdAltReadRegionParams(u8 *data, u32 *stat)
{
    unsigned char RegionData[15];
    int result;

    memset(data, 0, 13);
    if (MECHACON_CMD_S36_supported)
    {
        if ((result = sceCdApplySCmd(0x36, NULL, 0, RegionData, sizeof(RegionData))) != 0)
        {
            *stat = RegionData[0];
            memcpy(data, &RegionData[1], 13);
        }
    }
    else
    {
        *stat  = 0x100;
        result = 1;
    }

    return result;
}

/*
	This function provides an equivalent of the sceCdMV function from the newer CDVDMAN modules.
	When EELOADCNF is used (In the case of not resetting the IOP and the previous program used EELOADCNF), the data output won't be right because EELOADCNF causes XCDVDMAN to be used with the old CDVDFSV module.
	One extra byte will be returned to CDVDFSV when sceCdMV completes. It doesn't seem to cause a crash despite such a condition considered a buffer overflow, but the data returned to the EE will be wrong:
		The old specs (Followed by CDVDFSV and the EE-side client) mean that only 3 bytes should be returned instead of 4, so the returned data (to the EE-side program) will be offset by 1 and the trailing byte will be truncated.

	On the SCPH-10000 and SCPH-15000, EELOADCNF doesn't exist and hence this behaviour won't exist.
*/
int sceCdAltMV(u8 *buffer, u32 *stat)
{
    int result;
    unsigned char subcommand, output[4];

    subcommand = 0;
    if ((result = sceCdApplySCmd(0x03, &subcommand, sizeof(subcommand), output, sizeof(output))) != 0)
    {
        *stat = output[0] & 0x80;
        output[0] &= 0x7F;
        memcpy(buffer, output, sizeof(output));
    }

    return result;
}

/* Thanks to krat0s researches this seems to return DSP version  */
int sceCdAltMV2(u8 *buffer, u32 *stat)
{
    int result;
    unsigned char subcommand, output[3];

    subcommand = 1;
    if ((result = sceCdApplySCmd(0x03, &subcommand, sizeof(subcommand), out_buffer, sizeof(out_buffer))) != 0)
    {
        *stat = output[0];
    }

    /* 2 bytes: minor and major version */
    memcpy(buffer, output, sizeof(output));

    return result;
}

int sceCdAltRM(char *ModelName, u32 *stat)
{
    unsigned char rdata[9];
    unsigned char sdata;
    int result1, result2;

    sdata   = 0;
    result1 = sceCdApplySCmd(0x17, &sdata, 1, rdata, 9);

    *stat   = rdata[0];
    memcpy(ModelName, &rdata[1], 8);

    sdata   = 8;
    result2 = sceCdApplySCmd(0x17, &sdata, 1, rdata, 9);

    *stat |= rdata[0];
    memcpy(&ModelName[8], &rdata[1], 8);

    return ((result1 != 0 && result2 != 0) ? 1 : 0);
}

int sceCdAltReadRenewalDate(void *buffer, u32 *stat)
{
    int result;
    unsigned char subcommand, out_buffer[16];

    subcommand = 0xFD;
    if ((result = sceCdApplySCmd(0x03, &subcommand, sizeof(subcommand), out_buffer, sizeof(out_buffer))) != 0)
    {
        *stat = out_buffer[0];
    }

    memcpy(buffer, &out_buffer[1], 5);

    return result;
}

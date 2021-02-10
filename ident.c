#include <kernel.h>
#include <errno.h>
#include <sifrpc.h>
#include <loadfile.h>
#include <libcdvd.h>
#include <libpad.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <sbv_patches.h>
#include <osd_config.h>
#include <timer.h>
#include <limits.h>

#include <libgs.h>

#include <speedregs.h>
#include <smapregs.h>
#include <dev9regs.h>

#include "sysman/sysinfo.h"
#include "SYSMAN_rpc.h"

#include "main.h"
#include "ident.h"
#include "pad.h"
#include "graphics.h"
#include "libcdvd_add.h"
#include "dvdplayer.h"
#include "OSDInit.h"
#include "ps1.h"
#include "modelname.h"

#include "UI.h"
#include "menu.h"
#include "crc16.h"
#include "dbms.h"

extern struct UIDrawGlobal UIDrawGlobal;

#define GS_REG_CSR (volatile u64 *)0x12001000 // System Status

int GetEEInformation(struct SystemInformation *SystemInformation)
{
    unsigned short int revision;
    unsigned int value;

    revision                                          = GetCop0(15);
    SystemInformation->mainboard.ee.implementation    = revision >> 8;
    SystemInformation->mainboard.ee.revision          = revision & 0xFF;

    asm("cfc1 %0, $0\n"
        : "=r"(revision)
        :);
    SystemInformation->mainboard.ee.FPUImplementation = revision >> 8;
    SystemInformation->mainboard.ee.FPURevision       = revision & 0xFF;

    value                                             = GetCop0(16);
    SystemInformation->mainboard.ee.ICacheSize        = value >> 9 & 3;
    SystemInformation->mainboard.ee.DCacheSize        = value >> 6 & 3;
    SystemInformation->mainboard.ee.RAMSize           = GetMemorySize();
    SystemInformation->mainboard.MachineType          = MachineType();

    revision                                          = (*GS_REG_CSR) >> 16;
    SystemInformation->mainboard.gs.revision          = revision & 0xFF;
    SystemInformation->mainboard.gs.id                = revision >> 8;

    ee_kmode_enter();
    SystemInformation->EE_F520 = *(volatile unsigned int *)0xB000F520;
    SystemInformation->EE_F540 = *(volatile unsigned int *)0xB000F540;
    SystemInformation->EE_F550 = *(volatile unsigned int *)0xB000F550;
    ee_kmode_exit();

    return 0;
}

static u16 CalculateCRCOfROM(void *buffer1, void *buffer2, void *start, unsigned int length)
{
    u16 crc;
    unsigned int i, size, prevSize;
    void *pDestBuffer, *pSrcBuffer;

    for (i = 0, prevSize = size, crc = CRC16_INITIAL_CHECKSUM, pDestBuffer = buffer1, pSrcBuffer = start; i < length; i += size, pSrcBuffer += size)
    {
        size = length - i > MEM_IO_BLOCK_SIZE ? MEM_IO_BLOCK_SIZE : length - i;

        SysmanSync(0);
        while (SysmanReadMemory(pSrcBuffer, pDestBuffer, size, 1) != 0)
            nopdelay();

        pDestBuffer = (pDestBuffer == buffer1) ? buffer2 : buffer1;
        if (i > 0)
            crc = CalculateCRC16(UNCACHED_SEG(pDestBuffer), prevSize, crc);
        prevSize = size;
    }

    pDestBuffer = (pDestBuffer == buffer1) ? buffer2 : buffer1;
    SysmanSync(0);
    return ReflectAndXORCRC16(CalculateCRC16(UNCACHED_SEG(pDestBuffer), prevSize, crc));
}

int CheckROM(const struct PS2IDBMainboardEntry *entry)
{
    const struct PS2IDBMainboardEntry *other;

    if ((other = PS2IDBMS_LookupMatchingROM(entry)) != NULL)
    {
        if ((entry->BOOT_ROM.IsExists && (other->BOOT_ROM.crc16 != entry->BOOT_ROM.crc16)) || (other->DVD_ROM.IsExists && (other->DVD_ROM.crc16 != entry->DVD_ROM.crc16)))
        {
            printf("CheckROM: ROM mismatch:\n");
            if (entry->BOOT_ROM.IsExists)
                printf("\tBOOT: 0x%04x 0x%04x\n", other->BOOT_ROM.crc16, entry->BOOT_ROM.crc16);
            if (other->DVD_ROM.IsExists)
                printf("\tDVD: 0x%04x 0x%04x\n", other->DVD_ROM.crc16, entry->DVD_ROM.crc16);

            return 1;
        }
    }

    return 0;
}

int GetPeripheralInformation(struct SystemInformation *SystemInformation)
{
    t_SysmanHardwareInfo hwinfo;
    int result, fd, i;
    u32 stat;
    void *buffer1, *buffer2;
    char *pNewline;

    SysmanGetHardwareInfo(&hwinfo);

    memcpy(&SystemInformation->mainboard.iop, &hwinfo.iop, sizeof(SystemInformation->mainboard.iop));
    memcpy(SystemInformation->ROMs, hwinfo.ROMs, sizeof(SystemInformation->ROMs));
    memcpy(&SystemInformation->erom, &hwinfo.erom, sizeof(SystemInformation->erom));
    memcpy(&SystemInformation->mainboard.BOOT_ROM, &hwinfo.BOOT_ROM, sizeof(SystemInformation->mainboard.BOOT_ROM));
    memcpy(&SystemInformation->mainboard.DVD_ROM, &hwinfo.DVD_ROM, sizeof(SystemInformation->mainboard.DVD_ROM));
    memcpy(&SystemInformation->mainboard.ssbus, &hwinfo.ssbus, sizeof(SystemInformation->mainboard.ssbus));
    memcpy(&SystemInformation->mainboard.iLink, &hwinfo.iLink, sizeof(SystemInformation->mainboard.iLink));
    memcpy(&SystemInformation->mainboard.usb, &hwinfo.usb, sizeof(SystemInformation->mainboard.usb));
    memcpy(&SystemInformation->mainboard.spu2, &hwinfo.spu2, sizeof(SystemInformation->mainboard.spu2));
    SystemInformation->mainboard.BoardInf         = hwinfo.BoardInf;
    SystemInformation->mainboard.MPUBoardID       = hwinfo.MPUBoardID;
    SystemInformation->mainboard.ROMGEN_MonthDate = hwinfo.ROMGEN_MonthDate;
    SystemInformation->mainboard.ROMGEN_Year      = hwinfo.ROMGEN_Year;
    SystemInformation->mainboard.status           = 0;

    buffer1                                       = memalign(64, MEM_IO_BLOCK_SIZE);
    buffer2                                       = memalign(64, MEM_IO_BLOCK_SIZE);

    if (SystemInformation->mainboard.BOOT_ROM.IsExists)
    {
        SystemInformation->mainboard.BOOT_ROM.crc16 = CalculateCRCOfROM(buffer1, buffer2, (void *)SystemInformation->mainboard.BOOT_ROM.StartAddress, SystemInformation->mainboard.BOOT_ROM.size);
        DEBUG_PRINTF("BOOT ROM CRC16: 0x%04x\n", SystemInformation->mainboard.BOOT_ROM.crc16);
    }

    if (SystemInformation->mainboard.DVD_ROM.IsExists)
    {
        SystemInformation->mainboard.DVD_ROM.crc16 = CalculateCRCOfROM(buffer1, buffer2, (void *)SystemInformation->mainboard.DVD_ROM.StartAddress, SystemInformation->mainboard.DVD_ROM.size);
        DEBUG_PRINTF("DVD ROM CRC16: 0x%04x\n", SystemInformation->mainboard.DVD_ROM.crc16);
    }

    free(buffer1);
    free(buffer2);

    //Initialize model name
    if (ModelNameInit() == 0)
    {
        //Get model name
        strncpy(SystemInformation->mainboard.ModelName, ModelNameGet(), sizeof(SystemInformation->mainboard.ModelName) - 1);
        SystemInformation->mainboard.ModelName[sizeof(SystemInformation->mainboard.ModelName) - 1] = '\0';
    }
    else
    {
        SystemInformation->mainboard.status |= PS2IDB_STAT_ERR_MNAME;
        SystemInformation->mainboard.ModelName[0] = '\0';
    }

    //Get DVD Player version
    strncpy(SystemInformation->DVDPlayerVer, DVDPlayerGetVersion(), sizeof(SystemInformation->DVDPlayerVer) - 1);
    SystemInformation->DVDPlayerVer[sizeof(SystemInformation->DVDPlayerVer) - 1] = '\0';
    if ((pNewline = strrchr(SystemInformation->DVDPlayerVer, '\n')) != NULL)
        *pNewline = '\0'; //The DVD player version may have a newline in it.

    //Get PS1DRV version
    strncpy(SystemInformation->PS1DRVVer, PS1DRVGetVersion(), sizeof(SystemInformation->PS1DRVVer) - 1);
    SystemInformation->PS1DRVVer[sizeof(SystemInformation->PS1DRVVer) - 1] = '\0';

    memset(SystemInformation->ConsoleID, 0, sizeof(SystemInformation->ConsoleID));
    memset(SystemInformation->iLinkID, 0, sizeof(SystemInformation->iLinkID));
    memset(SystemInformation->SMAP_MAC_address, 0, sizeof(SystemInformation->SMAP_MAC_address));
    memset(SystemInformation->mainboard.MECHACONVersion, 0, sizeof(SystemInformation->mainboard.MECHACONVersion));
    memset(SystemInformation->DSPVersion, 0, sizeof(SystemInformation->DSPVersion));
    memset(SystemInformation->mainboard.MRenewalDate, 0, sizeof(SystemInformation->mainboard.MRenewalDate));

    if (sceCdAltMV(SystemInformation->mainboard.MECHACONVersion, &stat) == 0 || (stat & 0x80) != 0)
    {
        printf("Failed to read MECHACON version. Stat: %x\n", stat);
        SystemInformation->mainboard.status |= PS2IDB_STAT_ERR_MVER;
    }
    if (sceCdAltMV2(SystemInformation->DSPVersion, &stat) == 0 || (stat & 0x80) != 0)
    {
        printf("Failed to read DSP version. Stat: %x\n", stat);
    }
    if (sceCdReadConsoleID(SystemInformation->ConsoleID, &result) == 0 || (result & 0x80))
    {
        printf("Failed to read console ID. Stat: %x\n", result);
        SystemInformation->mainboard.status |= PS2IDB_STAT_ERR_CONSOLEID;
    }
    if (sceCdRI(SystemInformation->iLinkID, &result) == 0 || (result & 0x80))
    {
        printf("Failed to read i.Link ID. Stat: %x\n", result);
        SystemInformation->mainboard.status |= PS2IDB_STAT_ERR_ILINKID;
    }
    if (SystemInformation->mainboard.MECHACONVersion[1] >= 5)
    { //v5.x MECHACON (SCPH-50000 and later) supports Mechacon Renewal Date.
        if (sceCdAltReadRenewalDate(SystemInformation->mainboard.MRenewalDate, &result) == 0 || (result & 0x80))
        {
            printf("Failed to read M Renewal Date. Stat: %x\n", result);
            SystemInformation->mainboard.status |= PS2IDB_STAT_ERR_MRENEWDATE;
        }
    }
    SysmanGetMACAddress(SystemInformation->SMAP_MAC_address);

    SystemInformation->mainboard.ADD010 = 0xFFFF;
    if (GetADD010(SystemInformation->mainboard.MECHACONVersion[1] >= 5 ? 0x001 : 0x010, &SystemInformation->mainboard.ADD010) != 0)
    {
        printf("Failed to read ADD0x010.\n");
        SystemInformation->mainboard.status |= PS2IDB_STAT_ERR_ADD010;
    }

    //Get the mainboard and chassis names, MODEL ID, console MODEL ID and EMCS ID.
    SystemInformation->mainboard.ModelID[0]    = SystemInformation->iLinkID[1];
    SystemInformation->mainboard.ModelID[1]    = SystemInformation->iLinkID[2];
    SystemInformation->mainboard.ModelID[2]    = SystemInformation->iLinkID[3];
    SystemInformation->mainboard.ConModelID[0] = SystemInformation->ConsoleID[0];
    SystemInformation->mainboard.ConModelID[1] = SystemInformation->ConsoleID[1];
    SystemInformation->mainboard.EMCSID        = SystemInformation->ConsoleID[7];
    strcpy(SystemInformation->mainboard.MainboardName, GetMainboardModelDesc(&SystemInformation->mainboard));
    strcpy(SystemInformation->chassis, GetChassisDesc(&SystemInformation->mainboard));

    CheckROM(&SystemInformation->mainboard);

    return 0;
}

int DumpRom(const char *filename, const struct SystemInformation *SystemInformation, struct DumpingStatus *DumpingStatus, unsigned int DumpingRegion)
{
    FILE *file;
    int result = 0;
    unsigned int BytesToRead, BytesRemaining, ROMSize, prevSize;
    const unsigned char *MemDumpStart;
    void *buffer1, *buffer2, *pBuffer;

    switch (DumpingRegion)
    {
        case DUMP_REGION_BOOT_ROM:
            ROMSize      = SystemInformation->mainboard.BOOT_ROM.size;
            MemDumpStart = (const unsigned char *)SystemInformation->mainboard.BOOT_ROM.StartAddress;
            break;
        case DUMP_REGION_DVD_ROM:
            ROMSize      = SystemInformation->mainboard.DVD_ROM.size;
            MemDumpStart = (const unsigned char *)SystemInformation->mainboard.DVD_ROM.StartAddress;
            break;
        default:
            return -EINVAL;
    }

    buffer1        = memalign(64, MEM_IO_BLOCK_SIZE);
    buffer2        = memalign(64, MEM_IO_BLOCK_SIZE);

    BytesRemaining = ROMSize;
    if ((file = fopen(filename, "wb")) != NULL)
    {
        for (pBuffer = buffer1, prevSize = BytesRemaining; BytesRemaining > 0; MemDumpStart += BytesToRead, BytesRemaining -= BytesToRead)
        {
            BytesToRead = BytesRemaining > MEM_IO_BLOCK_SIZE ? MEM_IO_BLOCK_SIZE : BytesRemaining;

            SysmanSync(0);
            while (SysmanReadMemory(MemDumpStart, pBuffer, BytesToRead, 1) != 0)
                nopdelay();

            RedrawDumpingScreen(SystemInformation, DumpingStatus);
            pBuffer = pBuffer == buffer1 ? buffer2 : buffer1;
            if (BytesRemaining < ROMSize)
            {
                if (fwrite(UNCACHED_SEG(pBuffer), 1, prevSize, file) != prevSize)
                {
                    result = -EIO;
                    break;
                }

                DumpingStatus[DumpingRegion].progress = 1.00f - (float)BytesRemaining / ROMSize;
            }
            prevSize = BytesToRead;
        }

        if (result == 0)
        {
            pBuffer = pBuffer == buffer1 ? buffer2 : buffer1;
            SysmanSync(0);

            if (fwrite(UNCACHED_SEG(pBuffer), 1, prevSize, file) == prevSize)
                DumpingStatus[DumpingRegion].progress = 1.00f - (float)BytesRemaining / ROMSize;
            else
                result = -EIO;
        }

        fclose(file);
    }
    else
        result = -ENOENT;

    DumpingStatus[DumpingRegion].status = (result == 0) ? 1 : result;

    free(buffer1);
    free(buffer2);

    return result;
}

int GetADD010(u16 address, u16 *word)
{
    unsigned char stat;

    if (sceCdReadNVM(address, word, &stat) != 1 || stat != 0)
        return -1;

    return 0;
}

int DumpMECHACON_EEPROM(const char *filename)
{
    FILE *file;
    int result;
    unsigned char stat;
    unsigned short int i;
    static unsigned short int IOBuffer[512];

    result = 0;
    if ((file = fopen(filename, "wb")) != NULL)
    {
        for (i = 0; i < 512; i++)
        {
            if (sceCdReadNVM(i, &IOBuffer[i], &stat) != 1 || stat != 0)
            {
                result = -EIO;
                break;
            }
        }

        if (fwrite(IOBuffer, 1, sizeof(IOBuffer), file) != sizeof(IOBuffer))
        {
            result = EIO;
        }
        fclose(file);
    }
    else
        result = -ENOENT;

    return result;
}

int WriteNewMainboardDBRecord(const char *path, const struct PS2IDBMainboardEntry *SystemInformation)
{
    FILE *file;
    int result;
    struct PS2IDB_NewMainboardEntryHeader header;

    if ((file = fopen(path, "wb")) != NULL)
    {
        header.magic[0] = '2';
        header.magic[1] = 'N';
        header.version  = PS2IDB_NEWENT_FORMAT_VERSION;
        if (fwrite(&header, sizeof(struct PS2IDB_NewMainboardEntryHeader), 1, file) == 1)
        {
            result = fwrite(SystemInformation, sizeof(struct PS2IDBMainboardEntry), 1, file) == 1 ? 0 : EIO;
        }
        else
            result = EIO;

        fclose(file);
    }
    else
        result = EIO;

    return result;
}

const char *GetiLinkSpeedDesc(unsigned char speed)
{
    static const char *speeds[] = {
        "S100",
        "S200",
        "S400",
        "Unknown"};

    if (speed > 3)
        speed = 3;

    return speeds[speed];
}

const char *GetiLinkComplianceLvlDesc(unsigned char level)
{
    static const char *levels[] = {
        "IEEE1394-1995",
        "IEEE1394A-2000",
        "Unknown"};

    if (level > 2)
        level = 2;

    return levels[level];
}

const char *GetiLinkVendorDesc(unsigned int vendor)
{
    const char *description;

    switch (vendor)
    {
        case 0x00A0B8:
            description = "LSI Logic";
            break;
        default:
            description = "Unknown";
    }

    return description;
}

const char *GetSSBUSIFDesc(unsigned char revision)
{
    const char *description;

    if ((description = PS2IDBMS_LookupComponentModel(PS2IDB_COMPONENT_SSBUSIF, revision)) == NULL)
    {
        description = "Missing";
    }

    return description;
}

const char *GetSPEEDDesc(unsigned short int revision)
{
    const char *description;

    if ((description = PS2IDBMS_LookupComponentModel(PS2IDB_COMPONENT_SPEED, revision)) == NULL)
    {
        description = "Missing";
    }

    return description;
}


const char *GetSPEEDCapsDesc(unsigned short int caps)
{
    static char capsbuffer[64];
    unsigned int i;
    unsigned char capability, NumCapabilities;
    static const char *capabilities[] = {
        "SMAP",
        "ATA",
        "Unknown",
        "UART",
        "DVR",
        "Flash",
        "Unknown"};

    if (caps != 0)
    {
        capsbuffer[0] = '\0';
        for (i = 0, NumCapabilities = 0; i < 8; i++)
        {
            if (caps >> i & 1)
            {
                if (NumCapabilities > 0)
                    strcat(capsbuffer, ", ");

                capability = (i < 6) ? i : 6;
                strcat(capsbuffer, capabilities[capability]);
                NumCapabilities++;
            }
        }
    }
    else
        strcpy(capsbuffer, "None");

    return capsbuffer;
}

const char *GetPHYVendDesc(unsigned int oui)
{
    const char *description;

    if ((description = PS2IDBMS_LookupComponentModel(PS2IDB_COMPONENT_ETH_PHY_VEND, oui)) == NULL)
    {
        description = "Unknown";
    }

    return description;
}

const char *GetPHYModelDesc(unsigned int oui, unsigned char model)
{
    unsigned int revision;
    const char *description;

    if ((description = PS2IDBMS_LookupComponentModel(PS2IDB_COMPONENT_ETH_PHY_MODEL, oui << 8 | model)) == NULL)
    {
        description = "Unknown";
    }

    return description;
}

const char *GetGSChipDesc(unsigned short int revision)
{
    const char *description;

    if ((description = PS2IDBMS_LookupComponentModel(PS2IDB_COMPONENT_GS, revision)) == NULL)
    {
        description = "Missing";
    }

    return description;
}

const char *GetEEChipDesc(unsigned short int revision)
{
    const char *description;

    if ((description = PS2IDBMS_LookupComponentModel(PS2IDB_COMPONENT_EE, revision)) == NULL)
    {
        description = "Missing";
    }

    return description;
}

const char *GetIOPChipDesc(unsigned short int revision)
{
    const char *description;

    if ((description = PS2IDBMS_LookupComponentModel(PS2IDB_COMPONENT_IOP, revision)) == NULL)
    {
        description = "Missing";
    }

    return description;
}

const char *GetSPU2ChipDesc(unsigned short int revision)
{
    const char *description;

    if ((description = PS2IDBMS_LookupComponentModel(PS2IDB_COMPONENT_SPU2, revision)) == NULL)
    {
        description = "Missing";
    }

    return description;
}

const char *GetMECHACONChipDesc(unsigned short int revision)
{
    const char *description;

    if ((description = PS2IDBMS_LookupComponentModel(PS2IDB_COMPONENT_MECHACON, revision)) == NULL)
    {
        description = "Missing";
    }

    return description;
}

const char *GetSystemTypeDesc(unsigned char type)
{
    const char *description;

    if ((description = PS2IDBMS_LookupComponentModel(PS2IDB_COMPONENT_SYSTEM_TYPE, type)) == NULL)
    {
        description = "Unknown";
    }

    return description;
}

const char *GetRegionDesc(unsigned char region)
{
    const char *description;

    if ((description = PS2IDBMS_LookupComponentModel(PS2IDB_COMPONENT_MG_REGION, region)) == NULL)
    {
        description = "Unknown";
    }

    return description;
}

const char *GetMainboardModelDesc(const struct PS2IDBMainboardEntry *SystemInformation)
{
    const char *description;
    const struct PS2IDBMainboardEntry *ModelData;

    if ((ModelData = PS2IDBMS_LookupMainboardModel(SystemInformation)) != NULL)
        description = ModelData->MainboardName;
    else if (!strncmp(SystemInformation->romver, "0170", 4) || !strncmp(SystemInformation->romver, "0190", 4))
        description = "Missing Sticker"; //SCPH-5xxxx can be retrieved from sticker
    else
        description = "Missing";


    return description;
}

const char *GetMRPDesc(unsigned short int id)
{
    const char *description;

    if ((description = PS2IDBMS_LookupComponentModel(PS2IDB_COMPONENT_MRP_BOARD, id & 0xF8)) == NULL)
    {
        description = "Unknown";
    }

    return description;
}

const char *GetChassisDesc(const struct PS2IDBMainboardEntry *SystemInformation)
{
    const char *description;

    if (!strcmp(SystemInformation->MainboardName, "GH-001") || !strcmp(SystemInformation->MainboardName, "GH-003"))
        description = "A-chassis"; //SCPH-10000 and SCPH-15000
    else if (!strcmp(SystemInformation->MainboardName, "GH-003") && strncmp("0101", SystemInformation->romver, 4))
        description = "A-chassis+"; //SCPH-18000 with GH-003
    else if (!strcmp(SystemInformation->MainboardName, "GH-008"))
        description = "AB-chassis"; //SCPH-18000
    else if (!strcmp(SystemInformation->MainboardName, "GH-004") || !strcmp(SystemInformation->MainboardName, "GH-005"))
        description = "B-chassis"; //SCPH-30000
    else if (!strcmp(SystemInformation->MainboardName, "GH-006") || !strcmp(SystemInformation->MainboardName, "GH-007"))
        description = "C-chassis"; //SCPH-30000
    else if (!strcmp(SystemInformation->MainboardName, "GH-010") || !strcmp(SystemInformation->MainboardName, "GH-011") || !strcmp(SystemInformation->MainboardName, "GH-012") || !strcmp(SystemInformation->MainboardName, "GH-013") || !strcmp(SystemInformation->MainboardName, "GH-014") || !strcmp(SystemInformation->MainboardName, "GH-016"))
        description = "D-chassis"; //SCPH-30000, SCPH-30000R and SCPH-35000
    else if (!strcmp(SystemInformation->MainboardName, "GH-015"))
        description = "F-chassis"; //SCPH-30000 and SCPH-30000R
    else if (!strcmp(SystemInformation->MainboardName, "GH-016"))
        description = "DR-chassis"; //SCPH-30000
    else if (!strcmp(SystemInformation->MainboardName, "GH-017") || !strcmp(SystemInformation->MainboardName, "GH-019") || !strcmp(SystemInformation->MainboardName, "GH-022"))
        description = "G-chassis"; //SCPH-37000 and SCPH-39000
    else if (!strcmp(SystemInformation->MainboardName, "GH-023"))
        description = "H-chassis"; //SCPH-50000
    else if (!strcmp(SystemInformation->MainboardName, "GH-026"))
        description = "I-chassis"; //SCPH-50000a
    else if (!strcmp(SystemInformation->MainboardName, "GH-029"))
        description = "J-chassis"; //SCPH-50000b
    else if (!strncmp(SystemInformation->MainboardName, "GH-032", 6) || !strncmp(SystemInformation->MainboardName, "GH-035", 6))
        description = "K-chassis"; //SCPH-70000
    else if (!strncmp(SystemInformation->MainboardName, "GH-037", 6) || !strncmp(SystemInformation->MainboardName, "GH-040", 6) || !strncmp(SystemInformation->MainboardName, "GH-041", 6))
        description = "L-chassis"; //SCPH-75000
    else if (!strncmp(SystemInformation->MainboardName, "GH-051", 6) || !strncmp(SystemInformation->MainboardName, "GH-052", 6))
        description = "M-chassis"; //SCPH-77000
    else if (!strncmp(SystemInformation->MainboardName, "GH-061", 6) || !strncmp(SystemInformation->MainboardName, "GH-062", 6))
        description = "N-chassis"; //SCPH-79000
    else if (!strncmp(SystemInformation->MainboardName, "GH-070", 6) || !strncmp(SystemInformation->MainboardName, "GH-071", 6))
        description = "P-chassis"; //SCPH-90000, TVcombo
    else if (!strncmp(SystemInformation->MainboardName, "GH-072", 6))
        description = "R-chassis"; //SCPH-90000
    else if (!strncmp(SystemInformation->MainboardName, "XPD-", 4))
        description = "X-chassis"; //PSX
    else if (!strncmp(SystemInformation->romver, "0170", 4) || !strncmp(SystemInformation->romver, "0190", 4))
        description = "Missing Sticker"; //SCPH-5xxxx can be retrieved from sticker
    else
        description = "Unknown";

    return description;
}

const char *GetModelIDDesc(unsigned int id)
{
    const char *description;

    if ((description = PS2IDBMS_LookupComponentModel(PS2IDB_COMPONENT_MODEL_ID, id)) == NULL)
    {
        description = "Missing Sticker";
    }

    return description;
}

const char *GetEMCSIDDesc(unsigned char id)
{
    const char *description;

    if ((description = PS2IDBMS_LookupComponentModel(PS2IDB_COMPONENT_EMCS_ID, id)) == NULL)
    {
        description = "Missing Sticker";
    }

    return description;
}

const char *GetADD010Desc(unsigned short int id)
{
    const char *description;

    if ((description = PS2IDBMS_LookupComponentModel(PS2IDB_COMPONENT_ADD010, id)) == NULL)
    {
        description = "Unknown";
    }

    return description;
}

unsigned int CalculateCPUCacheSize(unsigned char value)
{ //2^(12+value)
    return (1U << (12 + value));
}

int WriteSystemInformation(FILE *stream, const struct SystemInformation *SystemInformation)
{
    unsigned int i, modelID;
    unsigned short int conModelID;
    int MayBeModded;
    const char *dvdplVer;

    MayBeModded = CheckROM(&SystemInformation->mainboard);

    //Header
    fputs("Log file generated by Playstation 2 Ident v" PS2IDENT_VERSION ", built on "__DATE__
          " "__TIME__
          "\r\n\r\n",
          stream);
    fprintf(stream, "ROMVER:\t%s\r\n", SystemInformation->mainboard.romver);

    //ROM region sizes
    fprintf(stream, "ROM region sizes:\r\n");
    for (i = 0; i <= 2; i++)
    {
        fprintf(stream, "\tROM%u:\t", i);
        if (SystemInformation->ROMs[i].IsExists)
            fprintf(stream, "%p (%u bytes)\r\n", SystemInformation->ROMs[i].StartAddress, SystemInformation->ROMs[i].size);
        else
            fputs("<Not detected>\r\n", stream);
    }
    fprintf(stream, "\tEROM:\t");
    if (SystemInformation->erom.IsExists)
        fprintf(stream, "%p (%u bytes)\r\n", SystemInformation->erom.StartAddress, SystemInformation->erom.size);
    else
        fprintf(stream, "<Not detected>\r\n");

    //Physical ROM chip sizes
    fputs("ROM chip sizes:\r\n"
          "\tBoot ROM:\t",
          stream);
    if (SystemInformation->mainboard.BOOT_ROM.IsExists)
    {
        fprintf(stream, "%p (%u Mbit)\tCRC16: 0x%04x\r\n",
                SystemInformation->mainboard.BOOT_ROM.StartAddress, SystemInformation->mainboard.BOOT_ROM.size / 1024 / 128,
                SystemInformation->mainboard.BOOT_ROM.crc16);
    }
    else
        fputs("<Not detected>\r\n", stream);

    fputs("\tDVD ROM:\t", stream);
    if (SystemInformation->mainboard.DVD_ROM.IsExists)
    {
        fprintf(stream, "%p (%u Mbit)\tCRC16: 0x%04x\r\n",
                SystemInformation->mainboard.DVD_ROM.StartAddress, SystemInformation->mainboard.DVD_ROM.size / 1024 / 128,
                SystemInformation->mainboard.DVD_ROM.crc16);
    }
    else
        fputs("<Not detected>\r\n", stream);

    //Version numbers
    dvdplVer = SystemInformation->DVDPlayerVer[0] == '\0' ? "-" : SystemInformation->DVDPlayerVer;
    fprintf(stream, "DVD Player:\t%s\r\n"
                    "PS1DRV:\t\t%s\r\n",
            dvdplVer, SystemInformation->PS1DRVVer);

    //Chip revisions
    fprintf(stream, "EE:\r\n"
                    "\tImplementation:\t\t0x%02x\r\n"
                    "\tRevision:\t\t%u.%u (%s)\r\n"
                    "\tEE_F520:\t\t0x%08x\r\n"
                    "\tEE_F540:\t\t0x%08x\r\n"
                    "\tEE_F550:\t\t0x%08x\r\n"
                    "\tFPU implementation:\t0x%02x\r\n"
                    "\tFPU revision:\t\t%u.%u\r\n"
                    "\tICache size:\t\t0x%02x (%u KB)\r\n"
                    "\tDCache size:\t\t0x%02x (%u KB)\r\n"
                    "\tRAM size:\t\t%u bytes\r\n",
            SystemInformation->mainboard.ee.implementation, SystemInformation->mainboard.ee.revision >> 4, SystemInformation->mainboard.ee.revision & 0xF, GetEEChipDesc((unsigned short int)(SystemInformation->mainboard.ee.implementation) << 8 | SystemInformation->mainboard.ee.revision),
            SystemInformation->EE_F520, SystemInformation->EE_F540, SystemInformation->EE_F550,
            SystemInformation->mainboard.ee.FPUImplementation, SystemInformation->mainboard.ee.FPURevision >> 4, SystemInformation->mainboard.ee.FPURevision & 0xF,
            SystemInformation->mainboard.ee.ICacheSize, CalculateCPUCacheSize(SystemInformation->mainboard.ee.ICacheSize) / 1024,
            SystemInformation->mainboard.ee.DCacheSize, CalculateCPUCacheSize(SystemInformation->mainboard.ee.DCacheSize) / 1024,
            SystemInformation->mainboard.ee.RAMSize);

    fprintf(stream, "IOP:\r\n"
                    "\tRevision:\t\t0x%04x (%s)\r\n"
                    "\tRAM size:\t\t%u bytes\r\n"
                    "\tSSBUS I/F revision:\t%u.%u (%s)\r\n",
            SystemInformation->mainboard.iop.revision,
            GetIOPChipDesc(SystemInformation->mainboard.iop.revision), SystemInformation->mainboard.iop.RAMSize,
            SystemInformation->mainboard.ssbus.revision >> 4, SystemInformation->mainboard.ssbus.revision & 0xF,
            GetSSBUSIFDesc(SystemInformation->mainboard.ssbus.revision));

    fputs("\tAIF revision:\t\t", stream);
    if (SystemInformation->mainboard.ssbus.status & PS2DB_SSBUS_HAS_AIF)
        fprintf(stream, "%u\r\n", SystemInformation->mainboard.ssbus.AIFRevision);
    else
        fputs("<Not detected>\r\n", stream);

    fputs("Mainboard:\r\n"
          "\tModel name:\t\t",
          stream);
    if (!(SystemInformation->mainboard.status & PS2IDB_STAT_ERR_MNAME))
        fprintf(stream, "%s\r\n", SystemInformation->mainboard.ModelName);
    else
        fputs("-\r\n", stream);

    fprintf(stream, "\tMainboard model:\t%s\r\n"
                    "\tChassis:\t\t%s\r\n"
                    "\tROMGEN:\t\t\t%04x-%04x\r\n"
                    "\tMachine type:\t\t0x%08x\r\n"
                    "\tBoardInf:\t\t0x%02x (%s)\r\n"
                    "\tMPU Board ID:\t\t0x%04x\r\n"
                    "\tSPU2 revision:\t\t0x%02x (%s)\r\n",
            SystemInformation->mainboard.MainboardName, SystemInformation->chassis,
            SystemInformation->mainboard.ROMGEN_MonthDate, SystemInformation->mainboard.ROMGEN_Year, SystemInformation->mainboard.MachineType,
            SystemInformation->mainboard.BoardInf, GetMRPDesc(SystemInformation->mainboard.BoardInf), SystemInformation->mainboard.MPUBoardID,
            SystemInformation->mainboard.spu2.revision, GetSPU2ChipDesc(SystemInformation->mainboard.spu2.revision));

    if (!(SystemInformation->mainboard.status & PS2IDB_STAT_ERR_MVER))
    {
        fprintf(stream, "\tMECHACON revision:\t%u.%02u (%s)\r\n"
                        "\tDSP revision:\t\t%u.%u\r\n"
                        "\tMagicGate region:\t0x%02x (%s)\r\n"
                        "\tSystem type:\t\t0x%02x (%s)\r\n",
                SystemInformation->mainboard.MECHACONVersion[1], SystemInformation->mainboard.MECHACONVersion[2], GetMECHACONChipDesc((unsigned int)(SystemInformation->mainboard.MECHACONVersion[1]) << 8 | (unsigned int)(SystemInformation->mainboard.MECHACONVersion[2])),
                SystemInformation->DSPVersion[1], SystemInformation->DSPVersion[2],
                SystemInformation->mainboard.MECHACONVersion[0], GetRegionDesc(SystemInformation->mainboard.MECHACONVersion[0]),
                SystemInformation->mainboard.MECHACONVersion[3], GetSystemTypeDesc(SystemInformation->mainboard.MECHACONVersion[3]));
    }
    else
    {
        fputs("\tMECHACON revision:\t-.-\r\n"
              "\tDSP revision:\t-.-\r\ns"
              "\tMagicGate region:\t-\r\n"
              "\tSystem type:\t\t-\r\n",
              stream);
    }

    fputs("\tADD0x010:\t\t", stream);
    if (!(SystemInformation->mainboard.status & PS2IDB_STAT_ERR_ADD010))
    {
        fprintf(stream, "0x%04x (%s)\r\n",
                SystemInformation->mainboard.ADD010, GetADD010Desc(SystemInformation->mainboard.ADD010));
    }
    else
    {
        fputs("-\r\n", stream);
    }

    fprintf(stream, "\tM Renewal Date:\t\t");
    if (SystemInformation->mainboard.MECHACONVersion[1] < 5 || (SystemInformation->mainboard.status & PS2IDB_STAT_ERR_MRENEWDATE))
        fprintf(stream, "----/--/-- --:--\r\n");
    else
        fprintf(stream, "20%02x/%02x/%02x %02x:%02x\r\n", SystemInformation->mainboard.MRenewalDate[0], SystemInformation->mainboard.MRenewalDate[1], SystemInformation->mainboard.MRenewalDate[2], SystemInformation->mainboard.MRenewalDate[3], SystemInformation->mainboard.MRenewalDate[4]);

    //i.Link ID
    fputs("\tModel ID:\t\t", stream);
    if (!(SystemInformation->mainboard.status & PS2IDB_STAT_ERR_ILINKID))
    {
        modelID = SystemInformation->mainboard.ModelID[0] | SystemInformation->mainboard.ModelID[1] << 8 | SystemInformation->mainboard.ModelID[2] << 16;
        fprintf(stream, "0x%06x (%s)\r\n", modelID, GetModelIDDesc(modelID));
    }
    else
    {
        fputs("-\r\n", stream);
    }

    //Console ID
    if (!(SystemInformation->mainboard.status & PS2IDB_STAT_ERR_CONSOLEID))
    {
        conModelID = SystemInformation->mainboard.ConModelID[0] | SystemInformation->mainboard.ConModelID[1] << 8;
        fprintf(stream, "\tConsole Model ID:\t0x%04x\r\n"
                        "\tEMCS ID:\t\t0x%02x (%s)\r\n",
                conModelID,
                SystemInformation->mainboard.EMCSID,
                GetEMCSIDDesc(SystemInformation->mainboard.EMCSID));
    }
    else
    {
        fputs("\tConsole Model ID:\t-\r\n"
              "\tEMCS ID:\t\t-\r\n",
              stream);
    }

    fprintf(stream, "\tUSB HC revision:\t%u.%u\r\n"
                    "\tGS revision:\t\t%u.%u (%s)\r\n"
                    "\tGS ID:\t\t\t0x%02x\r\n",
            SystemInformation->mainboard.usb.HcRevision >> 4, SystemInformation->mainboard.usb.HcRevision & 0xF,
            SystemInformation->mainboard.gs.revision >> 4, SystemInformation->mainboard.gs.revision & 0xF,
            GetGSChipDesc((u16)(SystemInformation->mainboard.gs.id) << 8 | SystemInformation->mainboard.gs.revision),
            SystemInformation->mainboard.gs.id);


    if (SystemInformation->mainboard.ssbus.status & PS2DB_SSBUS_HAS_SPEED)
    {
        fprintf(stream, "\tSPEED revision:\t\t0x%04x (%s)\r\n"
                        "\tSPEED capabilities:\t%04x.%04x (%s)\r\n",
                SystemInformation->mainboard.ssbus.SPEED.rev1, GetSPEEDDesc(SystemInformation->mainboard.ssbus.SPEED.rev1), SystemInformation->mainboard.ssbus.SPEED.rev3, SystemInformation->mainboard.ssbus.SPEED.rev8, GetSPEEDCapsDesc(SystemInformation->mainboard.ssbus.SPEED.rev3));
        fprintf(stream, "\tPHY OUI:\t\t0x%06x (%s)\r\n"
                        "\tPHY model:\t\t0x%02x (%s)\r\n"
                        "\tPHY revision:\t\t0x%02x\r\n",
                SystemInformation->mainboard.ssbus.SPEED.SMAP_PHY_OUI, GetPHYVendDesc(SystemInformation->mainboard.ssbus.SPEED.SMAP_PHY_OUI), SystemInformation->mainboard.ssbus.SPEED.SMAP_PHY_VMDL, GetPHYModelDesc(SystemInformation->mainboard.ssbus.SPEED.SMAP_PHY_OUI, SystemInformation->mainboard.ssbus.SPEED.SMAP_PHY_VMDL), SystemInformation->mainboard.ssbus.SPEED.SMAP_PHY_REV);
    }
    else
    {
        fprintf(stream, "\t***No expansion device connected***\r\n");
    }

    fprintf(stream, "i.Link:\r\n"
                    "\tPorts:\t\t\t%u\r\n"
                    "\tMax speed:\t\t%u (%s)\r\n"
                    "\tCompliance level:\t%u (%s)\r\n"
                    "\tVendor ID:\t\t0x%06x (%s)\r\n"
                    "\tProduct ID:\t\t0x%06x\r\n",
            SystemInformation->mainboard.iLink.NumPorts,
            SystemInformation->mainboard.iLink.MaxSpeed,
            GetiLinkSpeedDesc(SystemInformation->mainboard.iLink.MaxSpeed),
            SystemInformation->mainboard.iLink.ComplianceLevel,
            GetiLinkComplianceLvlDesc(SystemInformation->mainboard.iLink.ComplianceLevel),
            SystemInformation->mainboard.iLink.VendorID,
            GetiLinkVendorDesc(SystemInformation->mainboard.iLink.VendorID),
            SystemInformation->mainboard.iLink.ProductID);

    if (SystemInformation->mainboard.status || MayBeModded)
    {
        fprintf(stream, "Remarks:\r\n");

        if (SystemInformation->mainboard.status & PS2IDB_STAT_ERR_MVER)
            fprintf(stream, "\tUnable to get MECHACON version.\r\n");
        if (SystemInformation->mainboard.status & PS2IDB_STAT_ERR_MNAME)
            fprintf(stream, "\tUnable to get model name.\r\n");
        if (SystemInformation->mainboard.status & PS2IDB_STAT_ERR_MRENEWDATE)
            fprintf(stream, "\tUnable to get M renewal date.\r\n");
        if (SystemInformation->mainboard.status & PS2IDB_STAT_ERR_ILINKID)
            fprintf(stream, "\tUnable to get i.Link ID.\r\n");
        if (SystemInformation->mainboard.status & PS2IDB_STAT_ERR_CONSOLEID)
            fprintf(stream, "\tUnable to get console ID.\r\n");
        if (SystemInformation->mainboard.status & PS2IDB_STAT_ERR_ADD010)
            fprintf(stream, "\tUnable to get ADD0x010.\r\n");
        if (MayBeModded)
            fprintf(stream, "\tROM may not be clean.\r\n");
    }

    return 0;
}

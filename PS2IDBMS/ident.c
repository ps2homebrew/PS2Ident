#include <stdio.h>
#include <math.h>
#include <string.h>

#include "ps2db0113.h"
#include "../dbms.h"
#include "ident.h"

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
        description = "Unknown";
    }

    return description;
}

const char *GetSPEEDDesc(unsigned short int revision)
{
    const char *description;

    if ((description = PS2IDBMS_LookupComponentModel(PS2IDB_COMPONENT_SPEED, revision)) == NULL)
    {
        description = "Unknown";
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
        description = "Unknown";
    }

    return description;
}

const char *GetEEChipDesc(unsigned short int revision)
{
    const char *description;

    if ((description = PS2IDBMS_LookupComponentModel(PS2IDB_COMPONENT_EE, revision)) == NULL)
    {
        description = "Unknown";
    }

    return description;
}

const char *GetIOPChipDesc(unsigned short int revision)
{
    const char *description;

    if ((description = PS2IDBMS_LookupComponentModel(PS2IDB_COMPONENT_IOP, revision)) == NULL)
    {
        description = "Unknown";
    }

    return description;
}

const char *GetSPU2ChipDesc(unsigned short int revision)
{
    const char *description;

    if ((description = PS2IDBMS_LookupComponentModel(PS2IDB_COMPONENT_SPU2, revision)) == NULL)
    {
        description = "Unknown";
    }

    return description;
}

const char *GetMECHACONChipDesc(unsigned short int revision)
{
    const char *description;

    if ((description = PS2IDBMS_LookupComponentModel(PS2IDB_COMPONENT_MECHACON, revision)) == NULL)
    {
        description = "Unknown";
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
    else if (!strcmp(SystemInformation->MainboardName, "GH-010") || !strcmp(SystemInformation->MainboardName, "GH-011") || !strcmp(SystemInformation->MainboardName, "GH-012") || !strcmp(SystemInformation->MainboardName, "GH-013") || !strcmp(SystemInformation->MainboardName, "GH-014"))
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
        description = "P-chassis"; //SCPH-90000
    else if (!strncmp(SystemInformation->MainboardName, "GH-072", 6))
        description = "R-chassis"; //SCPH-90000
    else if (!strncmp(SystemInformation->MainboardName, "XPD-", 4))
        description = "X-chassis"; //PSX
    else
        description = "Unknown";

    return description;
}

const char *GetModelIDDesc(unsigned int id)
{
    const char *description;

    if ((description = PS2IDBMS_LookupComponentModel(PS2IDB_COMPONENT_MODEL_ID, id)) == NULL)
    {
        description = "Unknown";
    }

    return description;
}

const char *GetEMCSIDDesc(unsigned char id)
{
    const char *description;

    if ((description = PS2IDBMS_LookupComponentModel(PS2IDB_COMPONENT_EMCS_ID, id)) == NULL)
    {
        description = "Unknown";
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
{
    return ((unsigned int)pow(2, 12.0 + value));
}

int WriteSystemInformation(FILE *stream, const struct PS2IDBMainboardEntry *SystemInformation)
{
    unsigned int modelID;
    unsigned short int conModelID;

    fprintf(stream, "ROMVER:\t%s\r\n", SystemInformation->romver);
    fprintf(stream, "ROM chip sizes:\r\n");
    fprintf(stream, "\tBoot ROM:\t");
    if (SystemInformation->BOOT_ROM.IsExists)
        fprintf(stream, "%u (%u Mbit)\tCRC16: 0x%04x\r\n", SystemInformation->BOOT_ROM.StartAddress, SystemInformation->BOOT_ROM.size / 1024 / 128, SystemInformation->BOOT_ROM.crc16);
    else
        fprintf(stream, "<Not detected>\r\n");

    fprintf(stream, "\tDVD ROM:\t");
    if (SystemInformation->DVD_ROM.IsExists)
        fprintf(stream, "%u (%u Mbit)\tCRC16: 0x%04x\r\n", SystemInformation->DVD_ROM.StartAddress, SystemInformation->DVD_ROM.size / 1024 / 128, SystemInformation->DVD_ROM.crc16);
    else
        fprintf(stream, "<Not detected>\r\n");

    fprintf(stream, "EE:\r\n"
                    "\tImplementation:\t\t0x%02x\r\n"
                    "\tRevision:\t\t%u.%u (%s)\r\n"
                    "\tFPU implementation:\t0x%02x\r\n"
                    "\tFPU revision:\t\t%u.%u\r\n"
                    "\tICache size:\t\t0x%02x (%u KB)\r\n"
                    "\tDCache size:\t\t0x%02x (%u KB)\r\n"
                    "\tRAM size:\t\t%u bytes\r\n",
            SystemInformation->ee.implementation, SystemInformation->ee.revision >> 4, SystemInformation->ee.revision & 0xF, GetEEChipDesc((unsigned short int)(SystemInformation->ee.implementation) << 8 | SystemInformation->ee.revision), SystemInformation->ee.FPUImplementation, SystemInformation->ee.FPURevision >> 4, SystemInformation->ee.FPURevision & 0xF,
            SystemInformation->ee.ICacheSize, CalculateCPUCacheSize(SystemInformation->ee.ICacheSize) / 1024,
            SystemInformation->ee.DCacheSize, CalculateCPUCacheSize(SystemInformation->ee.DCacheSize) / 1024,
            SystemInformation->ee.RAMSize);

    fprintf(stream, "IOP:\r\n"
                    "\tRevision:\t\t0x%04x (%s)\r\n"
                    "\tRAM size:\t\t%u bytes\r\n",
            SystemInformation->iop.revision, GetIOPChipDesc(SystemInformation->iop.revision), SystemInformation->iop.RAMSize);

    modelID    = SystemInformation->ModelID[0] | SystemInformation->ModelID[1] << 8 | SystemInformation->ModelID[2] << 16;
    conModelID = SystemInformation->ConModelID[0] | SystemInformation->ConModelID[1] << 8;
    fprintf(stream, "Mainboard:\r\n"
                    "\tModel name:\t\t%s\r\n"
                    "\tModel ID:\t\t0x%06x (%s)\r\n"
                    "\tConsole Model ID:\t0x%04x\r\n"
                    "\tEMCS ID:\t\t0x%02x (%s)\r\n"
                    "\tMainboard model:\t%s\r\n"
                    "\tChassis:\t\t%s\r\n"
                    "\tROMGEN:\t\t\t%04x-%04x\r\n"
                    "\tMachine type:\t\t0x%08x\r\n"
                    "\tMPU Board ID:\t\t0x%04x\r\n"
                    "\tBoardInf:\t\t0x%02x (%s)\r\n"
                    "\tSPU2 revision:\t\t0x%02x (%s)\r\n"
                    "\tMECHACON version:\tv%u.%u (%s)\r\n"
                    "\tMagicGate region:\t0x%02x (%s)\r\n"
                    "\tSystem type:\t\t0x%02x (%s)\r\n"
                    "\tADD0x010:\t\t0x%04x (%s)\r\n"
                    "\tM Renewal Date:\t\t",
            SystemInformation->ModelName, modelID, GetModelIDDesc(modelID), conModelID, SystemInformation->EMCSID, GetEMCSIDDesc(SystemInformation->EMCSID), SystemInformation->MainboardName, GetChassisDesc(SystemInformation),
            SystemInformation->ROMGEN_MonthDate, SystemInformation->ROMGEN_Year, SystemInformation->MachineType, SystemInformation->MPUBoardID, SystemInformation->BoardInf, GetMRPDesc(SystemInformation->BoardInf), SystemInformation->spu2.revision, GetSPU2ChipDesc(SystemInformation->spu2.revision),
            SystemInformation->MECHACONVersion[1], SystemInformation->MECHACONVersion[2], GetMECHACONChipDesc((unsigned int)(SystemInformation->MECHACONVersion[1]) << 8 | SystemInformation->MECHACONVersion[2]),
            SystemInformation->MECHACONVersion[0], GetRegionDesc(SystemInformation->MECHACONVersion[0]), SystemInformation->MECHACONVersion[3], GetSystemTypeDesc(SystemInformation->MECHACONVersion[3]),
            SystemInformation->ADD010, GetADD010Desc(SystemInformation->ADD010));

    if (SystemInformation->MECHACONVersion[1] < 5 || (SystemInformation->status & PS2IDB_STAT_ERR_MRENEWDATE))
        fprintf(stream, "----/--/-- --:--");
    else
        fprintf(stream, "20%02x/%02x/%02x %02x:%02x", SystemInformation->MRenewalDate[0], SystemInformation->MRenewalDate[1], SystemInformation->MRenewalDate[2], SystemInformation->MRenewalDate[3], SystemInformation->MRenewalDate[4]);
    fprintf(stream, "\r\n");

    fprintf(stream, "\tUSB HCI revision:\t%u.%u\r\n"
                    "\tGS revision:\t\t0x%02x (%s)\r\n"
                    "\tGS ID:\t\t\t0x%02x\r\n",
            SystemInformation->usb.HcRevision >> 4, SystemInformation->usb.HcRevision & 0xF, SystemInformation->gs.revision, GetGSChipDesc((unsigned short int)(SystemInformation->gs.id) << 8 | SystemInformation->gs.revision), SystemInformation->gs.id);

    fprintf(stream, "SSBUS:\r\n"
                    "\tSSBUS I/F revision:\t0x%02x (%s)\r\n",
            SystemInformation->ssbus.revision, GetSSBUSIFDesc(SystemInformation->ssbus.revision));

    fprintf(stream, "\tAIF revision:\t\t");
    if (SystemInformation->ssbus.status & PS2DB_SSBUS_HAS_AIF)
        fprintf(stream, "%u", SystemInformation->ssbus.AIFRevision);
    else
        fprintf(stream, "<Not detected>");
    fprintf(stream, "\r\n");

    if (SystemInformation->ssbus.status & PS2DB_SSBUS_HAS_SPEED)
    {
        fprintf(stream, "\tSPEED revision:\t\t0x%04x (%s)\r\n"
                        "\tSPEED capabilities:\t%04x.%04x (%s)\r\n",
                SystemInformation->ssbus.SPEED.rev1, GetSPEEDDesc(SystemInformation->ssbus.SPEED.rev1), SystemInformation->ssbus.SPEED.rev3, SystemInformation->ssbus.SPEED.rev8, GetSPEEDCapsDesc(SystemInformation->ssbus.SPEED.rev3));

        fprintf(stream, "\tPHY OUI:\t\t0x%06x (%s)\r\n"
                        "\tPHY model:\t\t0x%02x (%s)\r\n"
                        "\tPHY revision:\t\t0x%02x\r\n",
                SystemInformation->ssbus.SPEED.SMAP_PHY_OUI, GetPHYVendDesc(SystemInformation->ssbus.SPEED.SMAP_PHY_OUI), SystemInformation->ssbus.SPEED.SMAP_PHY_VMDL, GetPHYModelDesc(SystemInformation->ssbus.SPEED.SMAP_PHY_OUI, SystemInformation->ssbus.SPEED.SMAP_PHY_VMDL), SystemInformation->ssbus.SPEED.SMAP_PHY_REV);
    }
    else
    {
        fprintf(stream, "\t***No integrated SPEED device***\r\n");
    }

    fprintf(stream, "i.Link:\r\n"
                    "\tPorts:\t\t\t%u\r\n"
                    "\tMax speed:\t\t%u (%s)\r\n"
                    "\tCompliance level:\t%u (%s)\r\n"
                    "\tVendor ID:\t\t0x%06x (%s)\r\n"
                    "\tProduct ID:\t\t0x%06x\r\n",
            SystemInformation->iLink.NumPorts, SystemInformation->iLink.MaxSpeed, GetiLinkSpeedDesc(SystemInformation->iLink.MaxSpeed), SystemInformation->iLink.ComplianceLevel, GetiLinkComplianceLvlDesc(SystemInformation->iLink.ComplianceLevel), SystemInformation->iLink.VendorID, GetiLinkVendorDesc(SystemInformation->iLink.VendorID), SystemInformation->iLink.ProductID);

    if (SystemInformation->status)
    {
        fprintf(stream, "Remarks:\r\n");

        if (SystemInformation->status & PS2IDB_STAT_ERR_MVER)
            fprintf(stream, "\tUnable to get MECHACON version.\r\n");
        if (SystemInformation->status & PS2IDB_STAT_ERR_MNAME)
            fprintf(stream, "\tUnable to get model name.\r\n");
        if (SystemInformation->status & PS2IDB_STAT_ERR_MRENEWDATE)
            fprintf(stream, "\tUnable to get M renewal date.\r\n");
        if (SystemInformation->status & PS2IDB_STAT_ERR_ILINKID)
            fprintf(stream, "\tUnable to get i.Link ID.\r\n");
        if (SystemInformation->status & PS2IDB_STAT_ERR_CONSOLEID)
            fprintf(stream, "\tUnable to get console ID.\r\n");
        if (SystemInformation->status & PS2IDB_STAT_ERR_ADD010)
            fprintf(stream, "\tUnable to get ADD0x010.\r\n");
        if (SystemInformation->status & PS2IDB_STAT_ERR_MODDED)
            fprintf(stream, "\tConsole is modded.\r\n");
    }

    return 0;
}

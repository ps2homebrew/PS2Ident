#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>

#include "ps2db0113.h"
#include "../dbms.h"
#include "ident.h"

static void ClearInput(void)
{
    while (getchar() != '\n') {};
}

static unsigned int GetWord(int base)
{
    char buffer[256], *ptr;

    fgets(buffer, sizeof(buffer), stdin);
    while ((ptr = strpbrk(buffer, "\n\r")) != NULL)
        *ptr = '\0';
    return strtoul(buffer, NULL, base);
}

static void GetString(char *buffer, int len)
{
    char *ptr;

    fgets(buffer, len, stdin);
    while ((ptr = strpbrk(buffer, "\n\r")) != NULL)
        *ptr = '\0';
}

static void AddMainboardModel(void)
{
    unsigned short int HalfValue;
    unsigned int value;
    struct PS2IDBMainboardEntry NewEntry;
    int result;

    memset(&NewEntry, 0, sizeof(NewEntry));

    printf("\nBoard information section\n");

    printf("Status: ");
    NewEntry.status = GetWord(0);

    printf("Model name: ");
    GetString(NewEntry.ModelName, sizeof(NewEntry.ModelName));

    printf("Contributor's name: ");
    GetString(NewEntry.ContributorName, sizeof(NewEntry.ContributorName));

    printf("Model ID: ");
    value               = GetWord(16);
    NewEntry.ModelID[0] = value & 0xFF;
    NewEntry.ModelID[1] = (value >> 8) & 0xFF;
    NewEntry.ModelID[2] = (value >> 16) & 0xFF;

    printf("Console Model ID: ");
    value                  = GetWord(16);
    NewEntry.ConModelID[0] = value & 0xFF;
    NewEntry.ConModelID[1] = (value >> 8) & 0xFF;

    printf("EMCS ID: ");
    value           = GetWord(16);
    NewEntry.EMCSID = value;

    printf("Mainboard name: ");
    GetString(NewEntry.MainboardName, sizeof(NewEntry.MainboardName));

    printf("ROMVER: ");
    GetString(NewEntry.romver, sizeof(NewEntry.romver));

    printf("MECHACON region: ");
    do
    {
        HalfValue = GetWord(0);
    } while (HalfValue < 0 || HalfValue >= PS2IDBMS_GetNumDatabaseRecords(PS2IDB_COMPONENT_MG_REGION));
    NewEntry.MECHACONVersion[0] = (u8)HalfValue;

    printf("MECHACON major: ");
    NewEntry.MECHACONVersion[1] = GetWord(0);

    printf("MECHACON minor: ");
    NewEntry.MECHACONVersion[2] = GetWord(0);

    printf("MECHACON console type: ");
    NewEntry.MECHACONVersion[3] = GetWord(0);

    printf("ADD0x010: ");
    NewEntry.ADD010 = GetWord(0);

    printf("MECHACON renewal year (since 2000): ");
    NewEntry.MRenewalDate[0] = GetWord(0);

    printf("MECHACON renewal month: ");
    NewEntry.MRenewalDate[1] = GetWord(0);

    printf("MECHACON renewal date: ");
    NewEntry.MRenewalDate[2] = GetWord(0);

    printf("MECHACON renewal hour: ");
    NewEntry.MRenewalDate[3] = GetWord(0);

    printf("MECHACON renewal minute: ");
    NewEntry.MRenewalDate[4] = GetWord(0);

    printf("BoardInf: ");
    NewEntry.BoardInf = GetWord(0);

    printf("ROMGEN year (YYYY): ");
    while ((NewEntry.ROMGEN_Year = GetWord(16)) == 0) {};

    printf("ROMGEN month & date (MMDD): ");
    while ((NewEntry.ROMGEN_MonthDate = GetWord(16)) == 0) {};

    printf("Machine type: ");
    NewEntry.MachineType = GetWord(0);

    printf("\nSPU2 section\n");

    printf("SPU2 revision: ");
    while ((NewEntry.spu2.revision = GetWord(0)) == 0) {};

    printf("\nUSB section\n");

    printf("USB HC revision: ");
    while ((NewEntry.usb.HcRevision = GetWord(0)) == 0) {};

    printf("\ni.Link section\n");

    printf("iLink ports: ");
    NewEntry.iLink.NumPorts = GetWord(0);

    printf("iLink maximum speed (0=S100, 1=S200, 2=S400): ");
    NewEntry.iLink.MaxSpeed = GetWord(0);

    printf("iLink compliance level: ");
    NewEntry.iLink.ComplianceLevel = GetWord(0);

    printf("iLink vendor ID: ");
    NewEntry.iLink.VendorID = GetWord(0);

    printf("iLink product ID: ");
    NewEntry.iLink.ProductID = GetWord(0);

    printf("\nDEV9 section\n");

    NewEntry.ssbus.status = 0;

    printf("SSBUS I/F revision: ");
    while ((NewEntry.ssbus.revision = GetWord(0)) == 0) {};

    printf("Does it have an AIF controller? [y/n] ");
    do
    {
        result = getchar();
    } while (result != 'n' && result != 'y');
    // getchar();

    if (result == 'y')
    {
        NewEntry.ssbus.status |= PS2DB_SSBUS_HAS_AIF;

        printf("AIF controller revision: ");
        while ((NewEntry.ssbus.AIFRevision = GetWord(0)) == 0) {};
    }
    else
    {
        NewEntry.ssbus.AIFRevision = 0;
    }

    printf("Does it have an integrated SPEED device? [y/n] ");
    do
    {
        result = getchar();
    } while (result != 'n' && result != 'y');
    // getchar();

    if (result == 'y')
    {
        NewEntry.ssbus.status |= PS2DB_SSBUS_HAS_SPEED;

        printf("SPEED rev1 (revision): ");
        while ((NewEntry.ssbus.SPEED.rev1 = GetWord(0)) == 0) {};

        printf("SPEED rev3 (capabilities): ");
        while ((NewEntry.ssbus.SPEED.rev3 = GetWord(0)) == 0) {};

        printf("SPEED rev8: ");
        while ((NewEntry.ssbus.SPEED.rev8 = GetWord(0)) == 0) {};

        printf("SPEED PHY OUI: ");
        while ((NewEntry.ssbus.SPEED.SMAP_PHY_OUI = GetWord(0)) == 0) {};

        printf("SPEED PHY model: ");
        while ((NewEntry.ssbus.SPEED.SMAP_PHY_VMDL = GetWord(0)) == 0) {};

        printf("SPEED PHY revision: ");
        while ((NewEntry.ssbus.SPEED.SMAP_PHY_REV = GetWord(0)) == 0) {};
    }

    printf("\nGS section\n");

    printf("GS revision: ");
    while ((NewEntry.gs.revision = GetWord(0)) == 0) {};

    printf("GS ID: ");
    while ((NewEntry.gs.id = GetWord(16)) == 0) {};

    printf("\nIOP section\n");

    printf("IOP revision: ");
    while ((NewEntry.iop.revision = GetWord(0)) == 0) {};

    printf("IOP RAM size (bytes): ");
    while ((NewEntry.iop.RAMSize = GetWord(0)) == 0) {};

    printf("\nEE section\n");

    printf("EE implementation: ");
    while ((NewEntry.ee.implementation = GetWord(0)) == 0) {};

    printf("EE revision: ");
    while ((NewEntry.ee.revision = GetWord(0)) == 0) {};

    printf("EE FPU implementation: ");
    while ((NewEntry.ee.FPUImplementation = GetWord(0)) == 0) {};

    printf("EE FPU revision: ");
    while ((NewEntry.ee.FPURevision = GetWord(0)) == 0) {};

    printf("MPU Board ID: ");
    while ((NewEntry.MPUBoardID = GetWord(16)) == 0) {};

    printf("EE ICache size (2^(12+IC)): ");
    while ((NewEntry.ee.ICacheSize = GetWord(0)) == 0) {};

    printf("EE DCache size (2^(12+DC)): ");
    while ((NewEntry.ee.DCacheSize = GetWord(0)) == 0) {};

    printf("EE RAM size (bytes): ");
    while ((NewEntry.ee.RAMSize = GetWord(0)) == 0) {};

    printf("\nROM section\n");

    NewEntry.BOOT_ROM.IsExists = 1;
    printf("Boot ROM address: ");
    while ((NewEntry.BOOT_ROM.StartAddress = GetWord(0)) == 0) {};
    printf("Boot ROM size (bytes): ");
    while ((NewEntry.BOOT_ROM.size = GetWord(0)) == 0) {};
    printf("BOOT ROM CRC16: ");
    NewEntry.BOOT_ROM.crc16 = GetWord(16);

    printf("Does it have a DVD ROM chip? [y/n] ");
    do
    {
        result = getchar();
    } while (result != 'n' && result != 'y');
    // getchar();

    if (result == 'y')
    {
        NewEntry.DVD_ROM.IsExists = 1;
        printf("DVD ROM address: ");
        while ((NewEntry.DVD_ROM.StartAddress = GetWord(0)) == 0) {};
        printf("DVD ROM size: ");
        while ((NewEntry.DVD_ROM.size = GetWord(0)) == 0) {};
        printf("DVD ROM CRC16: ");
        while ((NewEntry.DVD_ROM.crc16 = GetWord(16)) == 0) {};
    }

    printf("Is the console modded? [y,n] ");
    do
    {
        result = getchar();
    } while (result != 'n' && result != 'y');
    // getchar();

    if (result == 'y')
        NewEntry.status |= PS2IDB_STAT_ERR_MODDED;

    printf("\nEnd of data entry.\n"
           "Adding entry...");

    if ((result = PS2IDBMS_AddMainboardModel(&NewEntry)) == 0)
    {
        printf("done!\n");
    }
    else
    {
        printf("failed!\n");
    }
}

static void DisplayMainboardList(void)
{
    unsigned int i, NumEntriesInDatabase, id;
    const struct PS2IDBMainboardEntry *entry;

    if ((NumEntriesInDatabase = PS2IDBMS_GetNumDatabaseRecords(PS2IDB_COMPONENT_MAINBOARD)) > 0)
    {
        printf("Model list (%u records):\n", NumEntriesInDatabase);

        for (i = 0; i < NumEntriesInDatabase; i++)
        {
            entry = PS2IDBMS_GetMainboardDatabaseRecord(i);
            printf("\t%03u. %s | %s\n", i + 1, entry->ModelName, entry->MainboardName);
        }

        printf("Model? (enter 0 to quit) ");
        while ((id = GetWord(0)) != 0)
        {
            if (id > 0 && id <= NumEntriesInDatabase)
            {
                WriteSystemInformation(stdout, PS2IDBMS_GetMainboardDatabaseRecord(id - 1));
            }
            else
            {
                printf("Invalid ID entered.\n");
            }

            printf("ID? (enter 0 to quit) ");
        }
    }
    else
    {
        printf("No entries in database.\n");
    }
}

static void GenerateMainboardCSV(void)
{
    unsigned int i, NumEntriesInDatabase, modelID;
    unsigned short int conModelID;
    const struct PS2IDBMainboardEntry *entry;
    char FileName[128];
    FILE *file;

    if ((NumEntriesInDatabase = PS2IDBMS_GetNumDatabaseRecords(PS2IDB_COMPONENT_MAINBOARD)) > 0)
    {
        printf("File to save to (leave blank to cancel): ");
        fgets(FileName, sizeof(FileName), stdin);
        FileName[strlen(FileName) - 1] = '\0';
        if (strlen(FileName) > 0)
        {
            if ((file = fopen(FileName, "w")) != NULL)
            {
                printf("Generating CSV...");

                fprintf(file, "Model;Model ID;Console Model ID;EMCS ID;ROMVER;ROMGEN;Type;Region;MECHACON;M Renewal Date;Mainboard;ADD0x010;Chassis;Machine type;BoardInf"
                              ";SPU2;Boot ROM;Boot ROM CRC16;DVD ROM;DVD ROM CRC16;USB;GS ID; GS revision;SSBUS I/F;AIF controller;SPEED revision;SPEED capabilities;"
                              "PHY OUI;PHY model;PHY revision;i.Link ports;i.Link maximum speed;i.Link compliance level;"
                              "i.Link vendor ID;i.Link product ID;EE implementation;EE revision;FPU implementation;"
                              "FPU revision;MPU Board ID;EE ICache size;EE DCache size;EE RAM size;IOP revision;IOP RAM size;Contributor;Remarks\n");
                for (i = 0; i < NumEntriesInDatabase; i++)
                {
                    entry      = PS2IDBMS_GetMainboardDatabaseRecord(i);
                    modelID    = entry->ModelID[0] | entry->ModelID[1] << 8 | entry->ModelID[2] << 16;
                    conModelID = entry->ConModelID[0] | entry->ConModelID[1] << 8;
                    fprintf(file, "\"%s\";\"0x%06x | %s\";0x%04x;\"0x%02x | %s\";\"%s\""
                                  ";%04x-%04x;\"0x%02x | %s\";\"0x%02x | %s\""
                                  ";\"v%u.%02u | %s\""
                                  ";",
                            entry->ModelName, modelID, GetModelIDDesc(modelID), conModelID, entry->EMCSID, GetEMCSIDDesc(entry->EMCSID), entry->romver,
                            entry->ROMGEN_MonthDate, entry->ROMGEN_Year,
                            entry->MECHACONVersion[3], GetSystemTypeDesc(entry->MECHACONVersion[3]), entry->MECHACONVersion[0], GetRegionDesc(entry->MECHACONVersion[0]),
                            entry->MECHACONVersion[1], entry->MECHACONVersion[2], GetMECHACONChipDesc((unsigned int)(entry->MECHACONVersion[1]) << 8 | entry->MECHACONVersion[2]));

                    if (entry->MECHACONVersion[1] < 5 || (entry->status & PS2IDB_STAT_ERR_MRENEWDATE))
                        fprintf(file, "\"----/--/-- --:--\"");
                    else
                        fprintf(file, "\"20%02x/%02x/%02x %02x:%02x\"", entry->MRenewalDate[0], entry->MRenewalDate[1], entry->MRenewalDate[2], entry->MRenewalDate[3], entry->MRenewalDate[4]);

                    fprintf(file, ";\"0x%04x | %s\";\"%s\";\"%s\""
                                  ";0x%08x;\"0x%02x | %s\";\"0x%02x | %s\""
                                  ";0x%08x | %uMbit; 0x%04x;",
                            entry->ADD010, GetADD010Desc(entry->ADD010), entry->MainboardName, GetChassisDesc(entry),
                            entry->MachineType, entry->BoardInf, GetMRPDesc(entry->BoardInf), entry->spu2.revision, GetSPU2ChipDesc(entry->spu2.revision),
                            entry->BOOT_ROM.StartAddress, entry->BOOT_ROM.size * 8 / 1024 / 1024, entry->BOOT_ROM.crc16);

                    if (entry->DVD_ROM.IsExists)
                    {
                        fprintf(file, "0x%08x | %uMbit; 0x%04x;", entry->DVD_ROM.StartAddress, entry->DVD_ROM.size * 8 / 1024 / 1024, entry->DVD_ROM.crc16);
                    }
                    else
                        fprintf(file, "-;-;");

                    fprintf(file, "%u.%u;0x%02x;\"%u.%02u | %s\";\"0x%02x | %s\";", 
                        (entry->usb.HcRevision & 0xF0) >> 4, entry->usb.HcRevision & 0xF, entry->gs.id, 
                        (entry->gs.revision & 0xF0) >> 4, entry->gs.revision & 0xF, GetGSChipDesc((unsigned short int)(entry->gs.id) << 8 | entry->gs.revision), 
                        entry->ssbus.revision, GetSSBUSIFDesc(entry->ssbus.revision));
                    if (entry->ssbus.status & PS2DB_SSBUS_HAS_AIF)
                    {
                        fprintf(file, "%u;", entry->ssbus.AIFRevision);
                    }
                    else
                    {
                        fprintf(file, "-;");
                    }

                    if (entry->ssbus.status & PS2DB_SSBUS_HAS_SPEED)
                    {
                        fprintf(file, "\"0x%04x | %s\";\"%04x.%04x | %s\";\"0x%06x | %s\";\"0x%02x | %s\";0x%02x;", 
                            entry->ssbus.SPEED.rev1, GetSPEEDDesc(entry->ssbus.SPEED.rev1), entry->ssbus.SPEED.rev3, entry->ssbus.SPEED.rev8, GetSPEEDCapsDesc(entry->ssbus.SPEED.rev3),
                            entry->ssbus.SPEED.SMAP_PHY_OUI, GetPHYVendDesc(entry->ssbus.SPEED.SMAP_PHY_OUI), 
                            entry->ssbus.SPEED.SMAP_PHY_VMDL, GetPHYModelDesc(entry->ssbus.SPEED.SMAP_PHY_OUI, entry->ssbus.SPEED.SMAP_PHY_VMDL), entry->ssbus.SPEED.SMAP_PHY_REV);
                    }
                    else
                        fprintf(file, "-;-;-;-;-;");

                    fprintf(file, "%u;\"%u | %s\";\"0x%02x | %s\";\"0x%08x | %s\";0x%08x;0x%02x;\"%u.%02u | %s\";0x%02x;%u.%u;0x%04x;%uKB;%uKB;\"%u bytes\";\"%u.%02u | %s\";\"%u bytes\";\"%s\";", 
                        entry->iLink.NumPorts, entry->iLink.MaxSpeed, GetiLinkSpeedDesc(entry->iLink.MaxSpeed), entry->iLink.ComplianceLevel, GetiLinkComplianceLvlDesc(entry->iLink.ComplianceLevel), 
                        entry->iLink.VendorID, GetiLinkVendorDesc(entry->iLink.VendorID), entry->iLink.ProductID, 
                        entry->ee.implementation, (entry->ee.revision & 0xF0) >> 4, entry->ee.revision & 0xF, GetEEChipDesc((unsigned short int)(entry->ee.implementation) << 8 | entry->ee.revision), 
                        entry->ee.FPUImplementation, (entry->ee.FPURevision & 0xF0) >> 4, entry->ee.FPURevision & 0xF, entry->MPUBoardID, 
                        CalculateCPUCacheSize(entry->ee.ICacheSize) / 1024, CalculateCPUCacheSize(entry->ee.DCacheSize) / 1024, entry->ee.RAMSize, 
                        (entry->iop.revision & 0xF0) >> 4, entry->iop.revision & 0xF, GetIOPChipDesc(entry->iop.revision), entry->iop.RAMSize, entry->ContributorName);

                    if (entry->status)
                    {
                        int count = 0;

                        fprintf(file, "\"");

                        if (entry->status & PS2IDB_STAT_ERR_MVER)
                        {
                            fprintf(file, "Unable to get MECHACON version");
                            count++;
                        }
                        if (entry->status & PS2IDB_STAT_ERR_MNAME)
                        {
                            if (count > 0)
                                fprintf(file, ", ");
                            fprintf(file, "Unable to get model name");
                            count++;
                        }
                        if (entry->status & PS2IDB_STAT_ERR_MRENEWDATE)
                        {
                            if (count > 0)
                                fprintf(file, ", ");
                            fprintf(file, "Unable to get M renewal date");
                            count++;
                        }
                        if (entry->status & PS2IDB_STAT_ERR_ILINKID)
                        {
                            if (count > 0)
                                fprintf(file, ", ");
                            fprintf(file, "Unable to get i.Link ID");
                            count++;
                        }
                        if (entry->status & PS2IDB_STAT_ERR_CONSOLEID)
                        {
                            if (count > 0)
                                fprintf(file, ", ");
                            fprintf(file, "Unable to get console ID");
                        }
                        if (entry->status & PS2IDB_STAT_ERR_ADD010)
                        {
                            if (count > 0)
                                fprintf(file, ", ");
                            fprintf(file, "Unable to get ADD0x010");
                        }
                        if (entry->status & PS2IDB_STAT_ERR_MODDED)
                        {
                            if (count > 0)
                                fprintf(file, ", ");
                            fprintf(file, "Console is modded");
                        }

                        fprintf(file, "\"");
                    }

                    fprintf(file, "\n");
                }

                fclose(file);
                printf("done!\n");
            }
            else
            {
                printf("Cannot create file: %s\n", FileName);
            }
        }
    }
    else
    {
        printf("No entries in database.\n");
    }
}

static void UpdateModelEntry(void)
{
    unsigned int id;
    const struct PS2IDBMainboardEntry *entry;
    struct PS2IDBMainboardEntry NewEntry;
    int result;

    printf("Enter the ID of the model entry to update (enter 0 to cancel): ");
    if ((id = GetWord(0)) > 0)
    {
        if ((entry = PS2IDBMS_GetMainboardDatabaseRecord(id - 1)) != NULL)
        {
            printf("Selected record: %d. (%s) %s by %s\n\nNew mainboard name (leave blank to cancel): ", id, entry->ModelName, entry->MainboardName, entry->ContributorName);
            memcpy(&NewEntry, entry, sizeof(struct PS2IDBMainboardEntry));
            GetString(NewEntry.MainboardName, sizeof(NewEntry.MainboardName));
            if (strlen(NewEntry.MainboardName) > 0)
            {
                printf("New contributor's name (leave blank to cancel): ");
                GetString(NewEntry.ContributorName, sizeof(NewEntry.ContributorName));
                if (strlen(NewEntry.ContributorName) > 0)
                {
                    printf("Is the console modded? [y,n] ");
                    do
                    {
                        result = getchar();
                    } while (result != 'n' && result != 'y');
                    // getchar();

                    if (result == 'y')
                        NewEntry.status |= PS2IDB_STAT_ERR_MODDED;
                    else
                        NewEntry.status &= ~PS2IDB_STAT_ERR_MODDED;

                    printf("Updating...");
                    if ((result = PS2IDBMS_UpdateMainboardModel(id - 1, &NewEntry)) == 0)
                    {
                        printf("done!\n");
                    }
                    else
                    {
                        printf("failed!\n");
                    }
                }
            }
        }
        else
        {
            printf("Invalid ID entered.\n");
        }
    }
}

static void DeleteModelEntry(void)
{
    unsigned int id;
    const struct PS2IDBMainboardEntry *entry;
    int result;

    printf("Enter the ID of the model entry to delete (enter 0 to cancel): ");
    if ((id = GetWord(0)) > 0)
    {
        if ((entry = PS2IDBMS_GetMainboardDatabaseRecord(id - 1)) != NULL)
        {
            printf("Selected record: %d. %s (%s)\n\nDelete? [y/n] ", id, entry->ModelName, entry->MainboardName);
            if (getchar() == 'y')
            {
                printf("Deleting...");
                if ((result = PS2IDBMS_DeleteMainboardRecord(id - 1)) == 0)
                {
                    printf("done!\n");
                }
                else
                {
                    printf("failed!\n");
                }
            }

            ClearInput();
        }
        else
        {
            printf("Invalid ID entered.\n");
        }
    }
}

static const char *ComponentTypes[PS2IDB_COMPONENT_COUNT] = {
    "Mainboard",
    "EE",
    "GS",
    "IOP",
    "SSBUS I/F",
    "SPU2",
    "MECHACON",
    "SPEED",
    "Ethernet PHY vendor",
    "System type",
    "Ethernet PHY model",
    "MagicGate region",
    "MRP board",
    "Model ID",
    "EMCS ID",
    "ADD0x010"};

static void AddComponentEntry(int id)
{
    struct PS2IDBComponentEntry NewEntry;

    memset(&NewEntry, 0, sizeof(NewEntry));

    printf("Enter component model name (text field). Leave blank to cancel: ");
    GetString(NewEntry.name, sizeof(NewEntry.name));
    if (strlen(NewEntry.name) > 0)
    {
        printf("Enter component revision (hex or dec number, example 0x00 or 0): ");
        NewEntry.revision = GetWord(0);

        if (PS2IDBMS_AddModel(id, &NewEntry) == EEXIST)
        {
            printf("Component already exists.\n");
        }
    }
}

static void ListComponentModels(int id)
{
    unsigned int i, NumEntriesInDatabase;
    const struct PS2IDBComponentEntry *entry;

    if ((NumEntriesInDatabase = PS2IDBMS_GetNumDatabaseRecords(id)) > 0)
    {
        printf("Model list (%u records):\n", NumEntriesInDatabase);

        for (i = 0; i < NumEntriesInDatabase; i++)
        {
            entry = PS2IDBMS_GetDatabaseRecord(id, i);
            printf("\t%03u. 0x%04x | %s\n", i + 1, entry->revision, entry->name);
        }
    }
    else
    {
        printf("No entries in database.\n");
    }
}

static void UpdateComponentEntry(int id)
{
    int index;
    const struct PS2IDBComponentEntry *entry;
    struct PS2IDBComponentEntry NewEntry;

    printf("Enter id of component to update (enter 0 to cancel): ");
    if ((index = GetWord(0)) > 0)
    {
        if ((entry = PS2IDBMS_GetDatabaseRecord(id, index - 1)) != NULL)
        {
            printf("Selected component is: %d. %s\t- 0x%04x\nNew model (leave blank to cancel): ", index, entry->name, entry->revision);
            memcpy(&NewEntry, entry, sizeof(struct PS2IDBComponentEntry));
            GetString(NewEntry.name, sizeof(NewEntry.name));
            if (strlen(NewEntry.name) > 0)
                PS2IDBMS_UpdateModel(id, index - 1, &NewEntry);
        }
        else
        {
            printf("Component does not exist.\n");
        }
    }
}

static void DeleteComponentEntry(int id)
{
    int index;
    const struct PS2IDBComponentEntry *entry;

    printf("Enter id of component to delete (enter 0 to cancel): ");
    if ((index = GetWord(0)) > 0)
    {
        if ((entry = PS2IDBMS_GetDatabaseRecord(id, index - 1)) != NULL)
        {
            printf("Selected component is: %d. %s\t- 0x%04x\nDelete? [y/n] ", index, entry->name, entry->revision);
            if (getchar() == 'y')
            {
                PS2IDBMS_DeleteRecord(id, index - 1);
            }

            ClearInput();
        }
        else
        {
            printf("Component does not exist.\n");
        }
    }
}

static void AddComponent(void)
{
    int i, choice;

    printf("Select component type to add:\n");
    for (i = 0; i < PS2IDB_COMPONENT_COUNT; i++)
        printf("\t%02d. %s\n", i + 1, ComponentTypes[i]);
    printf("Enter choice (Enter 0 to quit): ");
    choice = GetWord(0) - 1;

    if (choice >= 0 && choice < PS2IDB_COMPONENT_COUNT)
    {
        switch (choice)
        {
            case PS2IDB_COMPONENT_MAINBOARD:
                AddMainboardModel();
                break;
            default:
                AddComponentEntry(choice);
        }
    }
}

static void ListComponent(void)
{
    int i, choice;

    printf("Select component type to list:\n");
    for (i = 0; i < PS2IDB_COMPONENT_COUNT; i++)
        printf("\t%d. %s\n", i + 1, ComponentTypes[i]);
    printf("Enter choice (Enter 0 to quit): ");
    choice = GetWord(0) - 1;

    if (choice >= 0 && choice < PS2IDB_COMPONENT_COUNT)
    {
        switch (choice)
        {
            case PS2IDB_COMPONENT_MAINBOARD:
                DisplayMainboardList();
                break;
            default:
                ListComponentModels(choice);
        }
    }
}

static void DeleteComponent(void)
{
    int i, choice;

    printf("Select component type to delete:\n");
    for (i = 0; i < PS2IDB_COMPONENT_COUNT; i++)
        printf("\t%d. %s\n", i + 1, ComponentTypes[i]);
    printf("Enter choice (Enter 0 to quit): ");
    choice = GetWord(0) - 1;

    if (choice >= 0 && choice < PS2IDB_COMPONENT_COUNT)
    {
        switch (choice)
        {
            case PS2IDB_COMPONENT_MAINBOARD:
                DeleteModelEntry();
                break;
            default:
                DeleteComponentEntry(choice);
        }
    }
}

static void UpdateComponent(void)
{
    int i, choice;

    printf("Select component type to update:\n");
    for (i = 0; i < PS2IDB_COMPONENT_COUNT; i++)
        printf("\t%d. %s\n", i + 1, ComponentTypes[i]);
    printf("Enter choice (Enter 0 to quit): ");
    choice = GetWord(0) - 1;

    if (choice >= 0 && choice < PS2IDB_COMPONENT_COUNT)
    {
        switch (choice)
        {
            case PS2IDB_COMPONENT_MAINBOARD:
                UpdateModelEntry();
                break;
            default:
                UpdateComponentEntry(choice);
        }
    }
}

static const struct PS2IDBMainboardEntry *_ImportMainboardModel(const char *filename, const char *ModelName, const char *ContributorName, short int isModded, int *pResult)
{
    FILE *file;
    int result, size;
    struct PS2IDB_NewMainboardEntryHeader header;
    struct PS2IDBMainboardEntry NewEntry;
    const struct PS2IDBMainboardEntry *entry;

    entry = NULL;
    if ((file = fopen(filename, "rb")) != NULL)
    {
        fseek(file, 0, SEEK_END);
        size = ftell(file);
        rewind(file);

        if (size == sizeof(struct PS2IDB_NewMainboardEntryHeader) + sizeof(struct PS2IDBMainboardEntry))
        {
            if (fread(&header, sizeof(struct PS2IDB_NewMainboardEntryHeader), 1, file) == 1)
            {
                if (header.magic[0] == '2' && header.magic[1] == 'N' && header.version == PS2IDB_NEWENT_FORMAT_VERSION)
                {
                    if (fread(&NewEntry, sizeof(struct PS2IDBMainboardEntry), 1, file) == 1)
                    {
                        strncpy(NewEntry.MainboardName, ModelName, sizeof(NewEntry.MainboardName) - 1);
                        NewEntry.MainboardName[sizeof(NewEntry.MainboardName) - 1] = '\0';
                        strncpy(NewEntry.ContributorName, ContributorName, sizeof(NewEntry.ContributorName) - 1);
                        NewEntry.ContributorName[sizeof(NewEntry.ContributorName) - 1] = '\0';

                        if (isModded)
                            NewEntry.status |= PS2IDB_STAT_ERR_MODDED;

                        if ((result = PS2IDBMS_AddMainboardModel(&NewEntry)) == 0)
                            entry = &NewEntry;
                        else if (result == EEXIST)
                            entry = PS2IDBMS_LookupMainboardModel(&NewEntry);
                    }
                    else
                        result = EIO;
                }
                else
                    result = EINVAL;
            }
            else
                result = EIO;
        }
        else
            result = EINVAL;

        fclose(file);
    }
    else
        result = ENOENT;

    *pResult = result;

    return entry;
}

static int ImportMainboardModel(const char *filename, const char *ModelName, const char *ContributorName, short int isModded)
{
    int result;
    const struct PS2IDBMainboardEntry *entry;

    printf("Importing %s...", filename);

    entry = _ImportMainboardModel(filename, ModelName, ContributorName, isModded, &result);
    switch (result)
    {
        case 0:
            printf("done!\n");
            break;
        case ENOENT:
            printf("file not found.\n");
            break;
        case EEXIST:
            printf("already added.\nExisting model: %s - %s\n", entry->ModelName, entry->MainboardName);
            break;
        case EINVAL:
            printf("invalid file format.\n");
            break;
        case EIO:
            printf("I/O error.\n");
            break;
        default:
            printf("failed!\n");
    }

    return result;
}

static void ImportMainboardModelPrompt(void)
{
    char path[256];
    char MainboardModel[16], ContributorName[16];
    int result;
    short int isModded;

    printf("Filename of file to import (leave blank to cancel): ");
    GetString(path, sizeof(path));
    if (strlen(path) > 0)
    {
        printf("Mainboard model (leave blank to cancel): ");
        GetString(MainboardModel, sizeof(MainboardModel));
        if (strlen(MainboardModel))
        {
            printf("Contributor's name (leave blank to cancel): ");
            GetString(ContributorName, sizeof(ContributorName));
            if (strlen(ContributorName))
            {
                printf("Is the console modded? [y/n] ");
                do
                {
                    result = getchar();
                } while (result != 'n' && result != 'y');
                // getchar();

                isModded = (result == 'y');

                ImportMainboardModel(path, MainboardModel, ContributorName, isModded);
            }
        }
    }
}

static void DisplaySyntaxHelp(void)
{
    printf("Syntax: PS2IDBMS <Database file> <options>\n"
           "\tOptions:\n"
           "\t\t/add <model data file> <model name> <contributor's name> [/modded]\n"
           "\nValues within [] are optional.\n");
}

int main(int argc, char *argv[])
{
    unsigned char done;
    int result, choice, i, j;
    short int isModded;

    printf("PlayStation 2 Ident model DBMS v1.06.1\n"
           "--------------------------------------\n\n");

    for (i = 2; i < argc; i++)
    {
        if (strcmp(argv[i], "/add") == 0 && i + 4 <= argc)
        {
            i++;

            isModded = 0;
            result   = 0;
            //Check for additional options.
            for (j = i + 3; j < argc; j++)
            {
                if (!strcmp(argv[j], "/modded"))
                {
                    isModded = 1;
                }
                else
                {
                    //Unrecognized option.
                    result = -EINVAL;
                    break;
                }
            }

            if (result != 0)
                break;

            PS2IDBMS_LoadDatabase(argv[1]);
            if ((result = ImportMainboardModel(argv[i], argv[i + 1], argv[i + 2], isModded)) == 0)
            {
                printf("Saving...");
                printf((result = PS2IDBMS_SaveDatabase(argv[1])) == 0 ? "done!\n" : "failed!\n");
            }
            PS2IDBMS_UnloadDatabase();

            return result;
        }
        else
            break;
    }

    if (i != argc)
    {
        DisplaySyntaxHelp();
        return EINVAL;
    }

    if ((result = PS2IDBMS_LoadDatabase(argv[1])) == 0 || result == ENOENT)
    {
        if (result == ENOENT)
        {
            printf("Warning: database %s could not be found. Creating a blank database.\n", argv[1]);
        }

        done = 0;
        do
        {
            do
            {
                printf("\nMain menu:\n"
                       "\t1. Import mainboard model\n"
                       "\t2. Add component\n"
                       "\t3. View component list\n"
                       "\t4. Update component/mainboard entry\n"
                       "\t5. Delete component\n"
                       "\t6. Generate mainboard model spreadsheet\n"
                       "\t7. Save and quit\n"
                       "\t8. Quit without saving\n"
                       "Your choice: ");

                if (scanf("%d", &choice) != 1)
                    choice = -1;
            } while (choice < 1 || choice > 8);
            ClearInput();

            switch (choice)
            {
                case 1:
                    ImportMainboardModelPrompt();
                    break;
                case 2:
                    AddComponent();
                    break;
                case 3:
                    ListComponent();
                    break;
                case 4:
                    UpdateComponent();
                    break;
                case 5:
                    DeleteComponent();
                    break;
                case 6:
                    GenerateMainboardCSV();
                    break;
                case 7:
                    printf("Saving...");
                    if ((result = PS2IDBMS_SaveDatabase(argv[1])) == 0)
                    {
                        printf("done!\n");
                    }
                    else
                    {
                        printf("failed!\n");
                    }
                    //Fall through.
                default:
                    done = 1;
            }
        } while (!done);

        PS2IDBMS_UnloadDatabase();
    }
    else
    {
        printf("Can't load database %s: %d.\n", argv[1], result);
    }

    return result;
}

struct RequiredFileSpaceStat
{
    unsigned int IsFile;
    unsigned int length;
};

struct SystemInformation
{
    struct PS2IDBMainboardEntry mainboard;
    t_PS2DBROMHardwareInfo ROMs[3];
    t_PS2DBROMHardwareInfo erom;
    unsigned char ConsoleID[8];
    unsigned char iLinkID[8];
    unsigned char SMAP_MAC_address[6];
    char chassis[14];
    char DVDPlayerVer[32];
    char PS1DRVVer[32];
    u8 DSPVersion[2];
    unsigned int EE_F520;
    unsigned int EE_F540;
    unsigned int EE_F550;
};

struct DumpingStatus
{
    float progress;
    int status; // 0 = In progress, 1 = complete, <0 = failed.
};

int GetEEInformation(struct SystemInformation *SystemInformation);
int GetPeripheralInformation(struct SystemInformation *SystemInformation);

const char *GetiLinkSpeedDesc(unsigned char speed);
const char *GetiLinkComplianceLvlDesc(unsigned char level);
const char *GetiLinkVendorDesc(unsigned int vendor);
const char *GetSSBUSIFDesc(unsigned char revision);
const char *GetSPEEDDesc(unsigned short int revision);
const char *GetSPEEDCapsDesc(unsigned short int caps);
const char *GetPHYVendDesc(unsigned int oui);
const char *GetPHYModelDesc(unsigned int oui, unsigned char model);
const char *GetGSChipDesc(unsigned short int revision);
const char *GetEEChipDesc(unsigned short int revision);
const char *GetIOPChipDesc(unsigned short int revision);
const char *GetSPU2ChipDesc(unsigned short int revision);
const char *GetMECHACONChipDesc(unsigned short int revision);
const char *GetSystemTypeDesc(unsigned char type);
const char *GetRegionDesc(unsigned char region);
const char *GetMainboardModelDesc(const struct PS2IDBMainboardEntry *SystemInformation);
const char *GetMRPDesc(unsigned short int id);
const char *GetChassisDesc(const struct PS2IDBMainboardEntry *SystemInformation);
const char *GetModelIDDesc(unsigned int id);
const char *GetEMCSIDDesc(unsigned char id);
const char *GetADD010Desc(unsigned short int id);

int GetADD010(u16 address, u16 *word);

unsigned int CalculateCPUCacheSize(unsigned char value);

int DumpRom(const char *filename, const struct SystemInformation *SystemInformation, struct DumpingStatus *DumpingStatus, unsigned int DumpingRegion);
int WriteNewMainboardDBRecord(const char *path, const struct PS2IDBMainboardEntry *SystemInformation);
int DumpMECHACON_EEPROM(const char *filename);
int WriteSystemInformation(FILE *stream, const struct SystemInformation *SystemInformation);

int CheckROM(const struct PS2IDBMainboardEntry *entry);

enum DUMP_REGIONS
{
    DUMP_REGION_BOOT_ROM = 0,
    DUMP_REGION_DVD_ROM,
    DUMP_REGION_EEPROM,
    DUMP_REGION_COUNT
};

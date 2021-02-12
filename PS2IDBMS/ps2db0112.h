#pragma pack(push, 1)

struct PS2IDBMainboardEntry112
{
    t_PS2DBROMHardwareInfo BOOT_ROM;
    t_PS2DBROMHardwareInfo DVD_ROM;
    t_PS2DBEEHardwareInfo ee;
    t_PS2DBIOPHardwareInfo iop;
    t_PS2DBGSHardwareInfo gs;
    t_PS2DBSSBUSHardwareInfo ssbus;
    t_PS2DBiLinkHardwareInfo iLink;
    t_PS2DBUSBHardwareInfo usb;
    t_PS2DBSPU2HardwareInfo spu2;

    u32 MachineType; //The value returned through the MachineType() syscall.
    u16 ROMGEN_MonthDate;
    u16 ROMGEN_Year;
    u16 MPUBoardID;
    u16 BoardInf;
    u8 MECHACONVersion[4]; // RR MM DD TT, where RR = (7-bit) region, MM = major revision, mm = minor revision, TT = system type.
    s8 ModelName[16];
    s8 romver[16];
    s8 MainboardName[16];
    u8 ModelID[3];
    u8 EMCSID;
    u8 ConModelID[2];
    u8 MRenewalDate[5];
    u8 status;
    //Known as "ADD0x10" in the SONY service tools, it's word 0x10 of the EEPROM from older consoles and word 0x01 of Dragon units. It's used to identify important revisions.
    u16 ADD010;
    u16 padding;
};
#define PS2IDB_FORMAT_VERSION_0112 0x0112

#pragma pack(pop)

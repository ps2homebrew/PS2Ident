#include <kernel.h>
#include <errno.h>
#include <libpad.h>
#include <stdio.h>
#include <string.h>
#include <timer.h>
#include <limits.h>
#include <wchar.h>

#include <libgs.h>

#include <speedregs.h>
#include <dev9regs.h>

#include "sysman/sysinfo.h"
#include "SYSMAN_rpc.h"

#include "main.h"
#include "ident.h"
#include "dbms.h"
#include "DeviceSupport.h"
#include "pad.h"
#include "graphics.h"

#include "UI.h"
#include "menu.h"

#include "font.h"

extern struct UIDrawGlobal UIDrawGlobal;
extern GS_IMAGE BackgroundTexture;
extern struct ClutImage PadLayoutTexture;
extern GS_IMAGE DeviceIconTexture;

extern unsigned short int SelectButton, CancelButton;

static int DumpSystemROM(const char *path, const struct SystemInformation *SystemInformation);

#define MAIN_MENU_BTN_DUMP 0xFF //Special button!

enum EEGS_ID
{
    EEGS_ID_EE_IMPL = 1,
    EEGS_ID_EE_REV_MINOR,
    EEGS_ID_EE_REV_MAJOR,
    EEGS_ID_EE_NAME,
    EEGS_ID_EE_FPU_IMPL,
    EEGS_ID_EE_FPU_REV_MINOR,
    EEGS_ID_EE_FPU_REV_MAJOR,
    EEGS_ID_EE_ICACHE_SIZE,
    EEGS_ID_EE_DCACHE_SIZE,
    EEGS_ID_EE_RAM_SIZE,

    EEGS_ID_GS_REV_MAJOR,
    EEGS_ID_GS_REV_MINOR,
    EEGS_ID_GS_NAME,
    EEGS_ID_GS_ID
};

enum IOPSPU2_ID
{
    IOPSPU2_ID_IOP_REV_MAJOR = 1,
    IOPSPU2_ID_IOP_REV_MINOR,
    IOPSPU2_ID_IOP_NAME,
    IOPSPU2_ID_IOP_RAM_SIZE,

    IOPSPU2_ID_SPU2_REV,
    IOPSPU2_ID_SPU2_NAME
};

enum BOARD_ID
{
    BOARD_ID_MODEL_NAME = 1,
    BOARD_ID_BOARD_NAME,
    BOARD_ID_CHASSIS_NAME,
    BOARD_ID_MACHINE_TYPE,
    BOARD_ID_MPU_ID,
    BOARD_ID_BOARD_INF,
    BOARD_ID_MRP_DESC,
    BOARD_ID_SSBUSIF_REV_MINOR,
    BOARD_ID_SSBUSIF_REV_MAJOR,
    BOARD_ID_SSBUSIF_NAME,
    BOARD_ID_AIF_REV
};

enum BOARD2_ID
{
    BOARD2_ID_MECHA_REV_MAJOR = 1,
    BOARD2_ID_MECHA_REV_MINOR,
    BOARD2_ID_MECHA_NAME,
    BOARD2_ID_MECHA_REGION,
    BOARD2_ID_MECHA_REGION_NAME,
    BOARD2_ID_MECHA_TYPE,
    BOARD2_ID_MECHA_TYPE_NAME,
    BOARD2_ID_ADD010,
    BOARD2_ID_ADD010_DESC,
    BOARD2_ID_MECHA_RENEWAL_YEAR,
    BOARD2_ID_MECHA_RENEWAL_MONTH,
    BOARD2_ID_MECHA_RENEWAL_DATE,
    BOARD2_ID_MECHA_RENEWAL_HOUR,
    BOARD2_ID_MECHA_RENEWAL_MINUTE,
    BOARD2_ID_SERIAL,
    BOARD2_ID_MODEL_ID,
    BOARD2_ID_MODEL_ID_DESC,
    BOARD2_ID_CON_MODEL_ID,
    BOARD2_ID_EMCS_ID,
    BOARD2_ID_EMCS_ID_DESC,
    BOARD2_ID_ILINK_ID_00,
    BOARD2_ID_ILINK_ID_01,
    BOARD2_ID_ILINK_ID_02,
    BOARD2_ID_ILINK_ID_03,
    BOARD2_ID_ILINK_ID_04,
    BOARD2_ID_ILINK_ID_05,
    BOARD2_ID_ILINK_ID_06,
    BOARD2_ID_ILINK_ID_07,
    BOARD2_ID_CONSOLE_ID_00,
    BOARD2_ID_CONSOLE_ID_01,
    BOARD2_ID_CONSOLE_ID_02,
    BOARD2_ID_CONSOLE_ID_03,
    BOARD2_ID_CONSOLE_ID_04,
    BOARD2_ID_CONSOLE_ID_05,
    BOARD2_ID_CONSOLE_ID_06,
    BOARD2_ID_CONSOLE_ID_07
};

enum ROM_ID
{
    ROM_ID_ROM0_ADDR = 1,
    ROM_ID_ROM0_SIZE,
    ROM_ID_ROM0_BYTES_LBL,
    ROM_ID_ROM1_ADDR,
    ROM_ID_ROM1_SIZE,
    ROM_ID_ROM1_BYTES_LBL,
    ROM_ID_ROM2_ADDR,
    ROM_ID_ROM2_SIZE,
    ROM_ID_ROM2_BYTES_LBL,
    ROM_ID_EROM_ADDR,
    ROM_ID_EROM_SIZE,
    ROM_ID_EROM_BYTES_LBL,
    ROM_ID_BOOT_ROM_ADDR,
    ROM_ID_BOOT_ROM_SIZE,
    ROM_ID_BOOT_ROM_MBIT_LBL,
    ROM_ID_DVD_ROM_ADDR,
    ROM_ID_DVD_ROM_SIZE,
    ROM_ID_DVD_ROM_MBIT_LBL,
    ROM_ID_ROM_UNCLEAN,
    ROM_ID_ROMVER,
    ROM_ID_ROMGEN_MMDD,
    ROM_ID_ROMGEN_YYYY,
    ROM_ID_DVDPLVER,
    ROM_ID_PS1DRVVER
};

enum DEV9_ID
{
    DEV9_ID_NO_DEVICE = 1,
    DEV9_ID_SPEED_REV_LBL,
    DEV9_ID_SPEED_REV,
    DEV9_ID_SPEED_NAME,
    DEV9_ID_SPEED_CAPS_LBL,
    DEV9_ID_SPEED_CAPS,
    DEV9_ID_SPEED_CAPS_SEP,
    DEV9_ID_SPEED_REV8,
    DEV9_ID_SPEED_CAPS_DESC,
    DEV9_ID_MAC_ADDR_LBL,
    DEV9_ID_MAC_ADDR_00,
    DEV9_ID_MAC_ADDR_01,
    DEV9_ID_MAC_ADDR_02,
    DEV9_ID_MAC_ADDR_03,
    DEV9_ID_MAC_ADDR_04,
    DEV9_ID_MAC_ADDR_05,
    DEV9_ID_MAC_ADDR_SEP_00,
    DEV9_ID_MAC_ADDR_SEP_01,
    DEV9_ID_MAC_ADDR_SEP_02,
    DEV9_ID_MAC_ADDR_SEP_03,
    DEV9_ID_MAC_ADDR_SEP_04,
    DEV9_ID_PHY_VENDOR_LBL,
    DEV9_ID_PHY_VENDOR_OUI,
    DEV9_ID_PHY_VENDOR_NAME,
    DEV9_ID_PHY_MODEL_LBL,
    DEV9_ID_PHY_MODEL,
    DEV9_ID_PHY_MODEL_DESC,
    DEV9_ID_PHY_NAME,
    DEV9_ID_PHY_REVISION_LBL,
    DEV9_ID_PHY_REVISION,
};

enum EXTBUS_ID
{
    EXTBUS_ID_USB_HC_REV_MAJOR = 1,
    EXTBUS_ID_USB_HC_REV_MINOR,
    EXTBUS_ID_ILINK_PORTS,
    EXTBUS_ID_ILINK_MAX_SPEED,
    EXTBUS_ID_ILINK_MAX_SPEED_DESC,
    EXTBUS_ID_ILINK_COMPLIANCE,
    EXTBUS_ID_ILINK_COMPLIANCE_DESC,
    EXTBUS_ID_ILINK_VENDOR_ID,
    EXTBUS_ID_ILINK_VENDOR_DESC,
    EXTBUS_ID_ILINK_PRODUCT_ID
};

static struct UIMenuItem EEGSMenuItems[] = {
    {MITEM_LABEL, 0, 0, 0, 0, 0, 0, SYS_UI_LBL_SYS_INFO}, {MITEM_BREAK}, {MITEM_BREAK},

    //EE
    {MITEM_LABEL, 0, 0, 0, 0, 0, 0, SYS_UI_LBL_EE},
    {MITEM_SEPERATOR},
    {MITEM_LABEL, 0, 0, 0, 0, 0, 0, SYS_UI_LBL_IMPLEMENTATION},
    {MITEM_TAB},
    {MITEM_TAB},
    {MITEM_VALUE, EEGS_ID_EE_IMPL, MITEM_FLAG_READONLY | MITEM_FLAG_UNIT_PREFIX, MITEM_FORMAT_HEX, 2},
    {MITEM_BREAK},
    {MITEM_LABEL, 0, 0, 0, 0, 0, 0, SYS_UI_LBL_REVISION},
    {MITEM_TAB},
    {MITEM_TAB},
    {MITEM_TAB},
    {MITEM_VALUE, EEGS_ID_EE_REV_MAJOR, MITEM_FLAG_READONLY, MITEM_FORMAT_UDEC},
    {MITEM_DOT},
    {MITEM_VALUE, EEGS_ID_EE_REV_MINOR, MITEM_FLAG_READONLY, MITEM_FORMAT_UDEC},
    {MITEM_BREAK},
    {MITEM_TAB},
    {MITEM_STRING, EEGS_ID_EE_NAME, MITEM_FLAG_READONLY},
    {MITEM_BREAK},
    {MITEM_LABEL, 0, 0, 0, 0, 0, 0, SYS_UI_LBL_FPU_IMPLEMENTATION},
    {MITEM_TAB},
    {MITEM_TAB},
    {MITEM_VALUE, EEGS_ID_EE_FPU_IMPL, MITEM_FLAG_READONLY | MITEM_FLAG_UNIT_PREFIX, MITEM_FORMAT_HEX, 2},
    {MITEM_BREAK},
    {MITEM_LABEL, 0, 0, 0, 0, 0, 0, SYS_UI_LBL_FPU_REVISION},
    {MITEM_TAB},
    {MITEM_TAB},
    {MITEM_TAB},
    {MITEM_VALUE, EEGS_ID_EE_FPU_REV_MAJOR, MITEM_FLAG_READONLY, MITEM_FORMAT_UDEC},
    {MITEM_DOT},
    {MITEM_VALUE, EEGS_ID_EE_FPU_REV_MINOR, MITEM_FLAG_READONLY, MITEM_FORMAT_UDEC},
    {MITEM_BREAK},
    {MITEM_LABEL, 0, 0, 0, 0, 0, 0, SYS_UI_LBL_ICACHE_SIZE},
    {MITEM_TAB},
    {MITEM_TAB},
    {MITEM_TAB},
    {MITEM_VALUE, EEGS_ID_EE_ICACHE_SIZE, MITEM_FLAG_READONLY, MITEM_FORMAT_UDEC},
    {MITEM_TAB},
    {MITEM_LABEL, 0, 0, 0, 0, 0, 0, SYS_UI_LBL_UNIT_KB},
    {MITEM_BREAK},
    {MITEM_LABEL, 0, 0, 0, 0, 0, 0, SYS_UI_LBL_DCACHE_SIZE},
    {MITEM_TAB},
    {MITEM_TAB},
    {MITEM_TAB},
    {MITEM_VALUE, EEGS_ID_EE_DCACHE_SIZE, MITEM_FLAG_READONLY, MITEM_FORMAT_UDEC},
    {MITEM_TAB},
    {MITEM_LABEL, 0, 0, 0, 0, 0, 0, SYS_UI_LBL_UNIT_KB},
    {MITEM_BREAK},
    {MITEM_LABEL, 0, 0, 0, 0, 0, 0, SYS_UI_LBL_RAM_SIZE},
    {MITEM_TAB},
    {MITEM_TAB},
    {MITEM_TAB},
    {MITEM_VALUE, EEGS_ID_EE_RAM_SIZE, MITEM_FLAG_READONLY, MITEM_FORMAT_UDEC},
    {MITEM_TAB},
    {MITEM_LABEL, 0, 0, 0, 0, 0, 0, SYS_UI_LBL_UNIT_BYTES},
    {MITEM_BREAK},

    {MITEM_BREAK},

    //GS
    {MITEM_LABEL, 0, 0, 0, 0, 0, 0, SYS_UI_LBL_GS},
    {MITEM_SEPERATOR},
    {MITEM_LABEL, 0, 0, 0, 0, 0, 0, SYS_UI_LBL_REVISION},
    {MITEM_TAB},
    {MITEM_TAB},
    {MITEM_TAB},
    {MITEM_VALUE, EEGS_ID_GS_REV_MAJOR, MITEM_FLAG_READONLY, MITEM_FORMAT_UDEC},
    {MITEM_DOT},
    {MITEM_VALUE, EEGS_ID_GS_REV_MINOR, MITEM_FLAG_READONLY, MITEM_FORMAT_UDEC, 0},
    {MITEM_BREAK},
    {MITEM_TAB},
    {MITEM_STRING, EEGS_ID_GS_NAME, MITEM_FLAG_READONLY},
    {MITEM_BREAK},
    {MITEM_LABEL, 0, 0, 0, 0, 0, 0, SYS_UI_LBL_ID},
    {MITEM_TAB},
    {MITEM_TAB},
    {MITEM_TAB},
    {MITEM_TAB},
    {MITEM_VALUE, EEGS_ID_GS_ID, MITEM_FLAG_READONLY | MITEM_FLAG_UNIT_PREFIX, MITEM_FORMAT_HEX, 2},
    {MITEM_BREAK},

    {MITEM_TERMINATOR}};

static struct UIMenuItem IOPSPU2MenuItems[] = {
    {MITEM_LABEL, 0, 0, 0, 0, 0, 0, SYS_UI_LBL_SYS_INFO}, {MITEM_BREAK}, {MITEM_BREAK},

    //IOP
    {MITEM_LABEL, 0, 0, 0, 0, 0, 0, SYS_UI_LBL_IOP},
    {MITEM_SEPERATOR},
    {MITEM_LABEL, 0, 0, 0, 0, 0, 0, SYS_UI_LBL_REVISION},
    {MITEM_TAB},
    {MITEM_TAB},
    {MITEM_VALUE, IOPSPU2_ID_IOP_REV_MAJOR, MITEM_FLAG_READONLY, MITEM_FORMAT_UDEC, 0},
    {MITEM_DOT},
    {MITEM_VALUE, IOPSPU2_ID_IOP_REV_MINOR, MITEM_FLAG_READONLY, MITEM_FORMAT_UDEC, 0},
    {MITEM_BREAK},
    {MITEM_TAB},
    {MITEM_STRING, IOPSPU2_ID_IOP_NAME, MITEM_FLAG_READONLY},
    {MITEM_BREAK},
    {MITEM_LABEL, 0, 0, 0, 0, 0, 0, SYS_UI_LBL_RAM_SIZE},
    {MITEM_TAB},
    {MITEM_TAB},
    {MITEM_VALUE, IOPSPU2_ID_IOP_RAM_SIZE, MITEM_FLAG_READONLY, MITEM_FORMAT_UDEC, 0},
    {MITEM_TAB},
    {MITEM_LABEL, 0, 0, 0, 0, 0, 0, SYS_UI_LBL_UNIT_BYTES},
    {MITEM_BREAK},

    {MITEM_BREAK},
    {MITEM_LABEL, 0, 0, 0, 0, 0, 0, SYS_UI_LBL_SPU2},
    {MITEM_SEPERATOR},

    //SPU
    {MITEM_LABEL, 0, 0, 0, 0, 0, 0, SYS_UI_LBL_REVISION},
    {MITEM_TAB},
    {MITEM_TAB},
    {MITEM_VALUE, IOPSPU2_ID_SPU2_REV, MITEM_FLAG_READONLY, MITEM_FORMAT_UDEC, 0},
    {MITEM_BREAK},
    {MITEM_TAB},
    {MITEM_STRING, IOPSPU2_ID_SPU2_NAME, MITEM_FLAG_READONLY},
    {MITEM_BREAK},

    {MITEM_TERMINATOR}};

static struct UIMenuItem BoardMenuItems[] = {
    {MITEM_LABEL, 0, 0, 0, 0, 0, 0, SYS_UI_LBL_SYS_INFO}, {MITEM_BREAK}, {MITEM_BREAK},

    {MITEM_LABEL, 0, 0, 0, 0, 0, 0, SYS_UI_LBL_MAINBOARD},
    {MITEM_SEPERATOR},

    {MITEM_LABEL, 0, 0, 0, 0, 0, 0, SYS_UI_LBL_MODEL},
    {MITEM_TAB},
    {MITEM_TAB},
    {MITEM_TAB},
    {MITEM_TAB},
    {MITEM_STRING, BOARD_ID_MODEL_NAME, MITEM_FLAG_READONLY},
    {MITEM_BREAK},
    {MITEM_LABEL, 0, 0, 0, 0, 0, 0, SYS_UI_LBL_MAINBOARD_MODEL},
    {MITEM_TAB},
    {MITEM_TAB},
    {MITEM_STRING, BOARD_ID_BOARD_NAME, MITEM_FLAG_READONLY},
    {MITEM_BREAK},
    {MITEM_LABEL, 0, 0, 0, 0, 0, 0, SYS_UI_LBL_CHASSIS_MODEL},
    {MITEM_TAB},
    {MITEM_TAB},
    {MITEM_TAB},
    {MITEM_STRING, BOARD_ID_CHASSIS_NAME, MITEM_FLAG_READONLY},
    {MITEM_BREAK},
    {MITEM_LABEL, 0, 0, 0, 0, 0, 0, SYS_UI_LBL_MACHINE_TYPE},
    {MITEM_TAB},
    {MITEM_TAB},
    {MITEM_TAB},
    {MITEM_VALUE, BOARD_ID_MACHINE_TYPE, MITEM_FLAG_READONLY | MITEM_FLAG_UNIT_PREFIX, MITEM_FORMAT_HEX, 8, 0},
    {MITEM_BREAK},
    {MITEM_LABEL, 0, 0, 0, 0, 0, 0, SYS_UI_LBL_MPU_BOARD_ID},
    {MITEM_TAB},
    {MITEM_TAB},
    {MITEM_TAB},
    {MITEM_VALUE, BOARD_ID_MPU_ID, MITEM_FLAG_READONLY | MITEM_FLAG_UNIT_PREFIX, MITEM_FORMAT_HEX, 4},
    {MITEM_BREAK},
    {MITEM_LABEL, 0, 0, 0, 0, 0, 0, SYS_UI_LBL_BOARD_INF},
    {MITEM_TAB},
    {MITEM_TAB},
    {MITEM_TAB},
    {MITEM_VALUE, BOARD_ID_BOARD_INF, MITEM_FLAG_READONLY | MITEM_FLAG_UNIT_PREFIX, MITEM_FORMAT_HEX, 2},
    {MITEM_TAB},
    {MITEM_STRING, BOARD_ID_MRP_DESC, MITEM_FLAG_READONLY},
    {MITEM_BREAK},
    {MITEM_LABEL, 0, 0, 0, 0, 0, 0, SYS_UI_LBL_SSBUSIF_REVISION},
    {MITEM_TAB},
    {MITEM_TAB},
    {MITEM_VALUE, BOARD_ID_SSBUSIF_REV_MAJOR, MITEM_FLAG_READONLY, MITEM_FORMAT_UDEC},
    {MITEM_DOT},
    {MITEM_VALUE, BOARD_ID_SSBUSIF_REV_MINOR, MITEM_FLAG_READONLY, MITEM_FORMAT_UDEC, 0},
    {MITEM_TAB},
    {MITEM_STRING, BOARD_ID_SSBUSIF_NAME, MITEM_FLAG_READONLY},
    {MITEM_BREAK},
    {MITEM_LABEL, 0, 0, 0, 0, 0, 0, SYS_UI_LBL_AIF_REVISION},
    {MITEM_TAB},
    {MITEM_TAB},
    {MITEM_TAB},
    {MITEM_VALUE, BOARD_ID_AIF_REV, MITEM_FLAG_READONLY, MITEM_FORMAT_UDEC},
    {MITEM_BREAK},

    {MITEM_TERMINATOR}};

static struct UIMenuItem Board2MenuItems[] = {
    {MITEM_LABEL, 0, 0, 0, 0, 0, 0, SYS_UI_LBL_SYS_INFO}, {MITEM_BREAK}, {MITEM_BREAK},

    {MITEM_LABEL, 0, 0, 0, 0, 0, 0, SYS_UI_LBL_MAINBOARD},
    {MITEM_SEPERATOR},

    {MITEM_LABEL, 0, 0, 0, 0, 0, 0, SYS_UI_LBL_MECHACON_REVISION},
    {MITEM_TAB},
    {MITEM_TAB},
    {MITEM_VALUE, BOARD2_ID_MECHA_REV_MAJOR, MITEM_FLAG_READONLY, MITEM_FORMAT_UDEC},
    {MITEM_DOT},
    {MITEM_VALUE, BOARD2_ID_MECHA_REV_MINOR, MITEM_FLAG_READONLY, MITEM_FORMAT_UDEC, 0},
    {MITEM_BREAK},
    {MITEM_TAB},
    {MITEM_TAB},
    {MITEM_TAB},
    {MITEM_TAB},
    {MITEM_STRING, BOARD2_ID_MECHA_NAME, MITEM_FLAG_READONLY},
    {MITEM_BREAK},
    {MITEM_LABEL, 0, 0, 0, 0, 0, 0, SYS_UI_LBL_MAGICGATE_REGION},
    {MITEM_TAB},
    {MITEM_TAB},
    {MITEM_VALUE, BOARD2_ID_MECHA_REGION, MITEM_FLAG_READONLY, MITEM_FORMAT_UDEC, 0},
    {MITEM_TAB},
    {MITEM_TAB},
    {MITEM_STRING, BOARD2_ID_MECHA_REGION_NAME, MITEM_FLAG_READONLY},
    {MITEM_BREAK},
    {MITEM_LABEL, 0, 0, 0, 0, 0, 0, SYS_UI_LBL_SYSTEM_TYPE},
    {MITEM_TAB},
    {MITEM_TAB},
    {MITEM_TAB},
    {MITEM_VALUE, BOARD2_ID_MECHA_TYPE, MITEM_FLAG_READONLY | MITEM_FLAG_UNIT_PREFIX, MITEM_FORMAT_HEX, 2},
    {MITEM_BREAK},
    {MITEM_TAB},
    {MITEM_TAB},
    {MITEM_TAB},
    {MITEM_TAB},
    {MITEM_STRING, BOARD2_ID_MECHA_TYPE_NAME, MITEM_FLAG_READONLY},
    {MITEM_BREAK},
    {MITEM_LABEL, 0, 0, 0, 0, 0, 0, SYS_UI_LBL_ADD010},
    {MITEM_TAB},
    {MITEM_TAB},
    {MITEM_TAB},
    {MITEM_VALUE, BOARD2_ID_ADD010, MITEM_FLAG_READONLY | MITEM_FLAG_UNIT_PREFIX, MITEM_FORMAT_HEX, 4},
    {MITEM_TAB},
    {MITEM_TAB},
    {MITEM_STRING, BOARD2_ID_ADD010_DESC, MITEM_FLAG_READONLY},
    {MITEM_BREAK},

    {MITEM_LABEL, 0, 0, 0, 0, 0, 0, SYS_UI_LBL_M_RENEWAL_DATE},
    {MITEM_TAB},
    {MITEM_TAB},
    {MITEM_VALUE, BOARD2_ID_MECHA_RENEWAL_YEAR, MITEM_FLAG_READONLY, MITEM_FORMAT_HEX, 4},
    {MITEM_SLASH},
    {MITEM_VALUE, BOARD2_ID_MECHA_RENEWAL_MONTH, MITEM_FLAG_READONLY, MITEM_FORMAT_HEX, 2},
    {MITEM_SLASH},
    {MITEM_VALUE, BOARD2_ID_MECHA_RENEWAL_DATE, MITEM_FLAG_READONLY, MITEM_FORMAT_HEX, 2},
    {MITEM_SPACE},
    {MITEM_VALUE, BOARD2_ID_MECHA_RENEWAL_HOUR, MITEM_FLAG_READONLY, MITEM_FORMAT_HEX, 2},
    {MITEM_COLON},
    {MITEM_VALUE, BOARD2_ID_MECHA_RENEWAL_MINUTE, MITEM_FLAG_READONLY, MITEM_FORMAT_HEX, 2},
    {MITEM_BREAK},
    {MITEM_SEPERATOR},
    {MITEM_BREAK},

    //IDs
    {MITEM_LABEL, 0, 0, 0, 0, 0, 0, SYS_UI_LBL_SERIAL_NUMBER},
    {MITEM_TAB},
    {MITEM_TAB},
    {MITEM_VALUE, BOARD2_ID_SERIAL, MITEM_FLAG_READONLY, MITEM_FORMAT_UDEC, 7},
    {MITEM_BREAK},
    {MITEM_LABEL, 0, 0, 0, 0, 0, 0, SYS_UI_LBL_MODEL_ID},
    {MITEM_TAB},
    {MITEM_TAB},
    {MITEM_VALUE, BOARD2_ID_MODEL_ID, MITEM_FLAG_READONLY | MITEM_FLAG_UNIT_PREFIX, MITEM_FORMAT_HEX, 6},
    {MITEM_TAB},
    {MITEM_STRING, BOARD2_ID_MODEL_ID_DESC, MITEM_FLAG_READONLY},
    {MITEM_BREAK},
    {MITEM_LABEL, 0, 0, 0, 0, 0, 0, SYS_UI_LBL_CON_MODEL_ID},
    {MITEM_TAB},
    {MITEM_TAB},
    {MITEM_VALUE, BOARD2_ID_CON_MODEL_ID, MITEM_FLAG_READONLY | MITEM_FLAG_UNIT_PREFIX, MITEM_FORMAT_HEX, 4},
    {MITEM_BREAK},
    {MITEM_LABEL, 0, 0, 0, 0, 0, 0, SYS_UI_LBL_EMCS_ID},
    {MITEM_TAB},
    {MITEM_TAB},
    {MITEM_VALUE, BOARD2_ID_EMCS_ID, MITEM_FLAG_READONLY | MITEM_FLAG_UNIT_PREFIX, MITEM_FORMAT_HEX, 2},
    {MITEM_TAB},
    {MITEM_TAB},
    {MITEM_STRING, BOARD2_ID_EMCS_ID_DESC, MITEM_FLAG_READONLY},
    {MITEM_BREAK},
    {MITEM_LABEL, 0, 0, 0, 0, 0, 0, SYS_UI_LBL_ILINK_ID},
    {MITEM_TAB},
    {MITEM_TAB},
    {MITEM_VALUE, BOARD2_ID_ILINK_ID_00, MITEM_FLAG_READONLY, MITEM_FORMAT_HEX, 2},
    {MITEM_SPACE},
    {MITEM_VALUE, BOARD2_ID_ILINK_ID_01, MITEM_FLAG_READONLY, MITEM_FORMAT_HEX, 2},
    {MITEM_SPACE},
    {MITEM_VALUE, BOARD2_ID_ILINK_ID_02, MITEM_FLAG_READONLY, MITEM_FORMAT_HEX, 2},
    {MITEM_SPACE},
    {MITEM_VALUE, BOARD2_ID_ILINK_ID_03, MITEM_FLAG_READONLY, MITEM_FORMAT_HEX, 2},
    {MITEM_SPACE},
    {MITEM_VALUE, BOARD2_ID_ILINK_ID_04, MITEM_FLAG_READONLY, MITEM_FORMAT_HEX, 2},
    {MITEM_SPACE},
    {MITEM_VALUE, BOARD2_ID_ILINK_ID_05, MITEM_FLAG_READONLY, MITEM_FORMAT_HEX, 2},
    {MITEM_SPACE},
    {MITEM_VALUE, BOARD2_ID_ILINK_ID_06, MITEM_FLAG_READONLY, MITEM_FORMAT_HEX, 2},
    {MITEM_SPACE},
    {MITEM_VALUE, BOARD2_ID_ILINK_ID_07, MITEM_FLAG_READONLY, MITEM_FORMAT_HEX, 2},
    {MITEM_BREAK},
    {MITEM_LABEL, 0, 0, 0, 0, 0, 0, SYS_UI_LBL_CONSOLE_ID},
    {MITEM_TAB},
    {MITEM_TAB},
    {MITEM_VALUE, BOARD2_ID_CONSOLE_ID_00, MITEM_FLAG_READONLY, MITEM_FORMAT_HEX, 2},
    {MITEM_SPACE},
    {MITEM_VALUE, BOARD2_ID_CONSOLE_ID_01, MITEM_FLAG_READONLY, MITEM_FORMAT_HEX, 2},
    {MITEM_SPACE},
    {MITEM_VALUE, BOARD2_ID_CONSOLE_ID_02, MITEM_FLAG_READONLY, MITEM_FORMAT_HEX, 2},
    {MITEM_SPACE},
    {MITEM_VALUE, BOARD2_ID_CONSOLE_ID_03, MITEM_FLAG_READONLY, MITEM_FORMAT_HEX, 2},
    {MITEM_SPACE},
    {MITEM_VALUE, BOARD2_ID_CONSOLE_ID_04, MITEM_FLAG_READONLY, MITEM_FORMAT_HEX, 2},
    {MITEM_SPACE},
    {MITEM_VALUE, BOARD2_ID_CONSOLE_ID_05, MITEM_FLAG_READONLY, MITEM_FORMAT_HEX, 2},
    {MITEM_SPACE},
    {MITEM_VALUE, BOARD2_ID_CONSOLE_ID_06, MITEM_FLAG_READONLY, MITEM_FORMAT_HEX, 2},
    {MITEM_SPACE},
    {MITEM_VALUE, BOARD2_ID_CONSOLE_ID_07, MITEM_FLAG_READONLY, MITEM_FORMAT_HEX, 2},
    {MITEM_BREAK},

    {MITEM_TERMINATOR}};

static struct UIMenuItem ROMMenuItems[] = {
    {MITEM_LABEL, 0, 0, 0, 0, 0, 0, SYS_UI_LBL_SYS_INFO}, {MITEM_BREAK}, {MITEM_BREAK},

    {MITEM_LABEL, 0, 0, 0, 0, 0, 0, SYS_UI_LBL_ROM_REGIONS},
    {MITEM_SEPERATOR},

    {MITEM_LABEL, 0, 0, 0, 0, 0, 0, SYS_UI_LBL_ROM0},
    {MITEM_TAB},
    {MITEM_VALUE, ROM_ID_ROM0_ADDR, MITEM_FLAG_READONLY, MITEM_FORMAT_POINTER},
    {MITEM_DASH},
    {MITEM_TAB},
    {MITEM_VALUE, ROM_ID_ROM0_SIZE, MITEM_FLAG_READONLY, MITEM_FORMAT_UDEC},
    {MITEM_TAB},
    {MITEM_LABEL, ROM_ID_ROM0_BYTES_LBL, 0, 0, 0, 0, 0, SYS_UI_LBL_UNIT_BYTES},
    {MITEM_BREAK},
    {MITEM_LABEL, 0, 0, 0, 0, 0, 0, SYS_UI_LBL_ROM1},
    {MITEM_TAB},
    {MITEM_VALUE, ROM_ID_ROM1_ADDR, MITEM_FLAG_READONLY, MITEM_FORMAT_POINTER},
    {MITEM_DASH},
    {MITEM_TAB},
    {MITEM_VALUE, ROM_ID_ROM1_SIZE, MITEM_FLAG_READONLY, MITEM_FORMAT_UDEC},
    {MITEM_TAB},
    {MITEM_LABEL, ROM_ID_ROM1_BYTES_LBL, 0, 0, 0, 0, 0, SYS_UI_LBL_UNIT_BYTES},
    {MITEM_BREAK},
    {MITEM_LABEL, 0, 0, 0, 0, 0, 0, SYS_UI_LBL_ROM2},
    {MITEM_TAB},
    {MITEM_VALUE, ROM_ID_ROM2_ADDR, MITEM_FLAG_READONLY, MITEM_FORMAT_POINTER},
    {MITEM_DASH},
    {MITEM_TAB},
    {MITEM_VALUE, ROM_ID_ROM2_SIZE, MITEM_FLAG_READONLY, MITEM_FORMAT_UDEC},
    {MITEM_TAB},
    {MITEM_LABEL, ROM_ID_ROM2_BYTES_LBL, 0, 0, 0, 0, 0, SYS_UI_LBL_UNIT_BYTES},
    {MITEM_BREAK},
    {MITEM_LABEL, 0, 0, 0, 0, 0, 0, SYS_UI_LBL_EROM},
    {MITEM_TAB},
    {MITEM_VALUE, ROM_ID_EROM_ADDR, MITEM_FLAG_READONLY, MITEM_FORMAT_POINTER},
    {MITEM_DASH},
    {MITEM_TAB},
    {MITEM_VALUE, ROM_ID_EROM_SIZE, MITEM_FLAG_READONLY, MITEM_FORMAT_UDEC},
    {MITEM_TAB},
    {MITEM_LABEL, ROM_ID_EROM_BYTES_LBL, 0, 0, 0, 0, 0, SYS_UI_LBL_UNIT_BYTES},
    {MITEM_BREAK},

    {MITEM_BREAK},
    {MITEM_LABEL, 0, 0, 0, 0, 0, 0, SYS_UI_LBL_ROM_CHIPS},
    {MITEM_SEPERATOR},

    {MITEM_LABEL, 0, 0, 0, 0, 0, 0, SYS_UI_LBL_BOOT_ROM},
    {MITEM_TAB},
    {MITEM_TAB},
    {MITEM_VALUE, ROM_ID_BOOT_ROM_ADDR, MITEM_FLAG_READONLY, MITEM_FORMAT_POINTER},
    {MITEM_DASH},
    {MITEM_TAB},
    {MITEM_VALUE, ROM_ID_BOOT_ROM_SIZE, MITEM_FLAG_READONLY, MITEM_FORMAT_UDEC},
    {MITEM_TAB},
    {MITEM_LABEL, ROM_ID_BOOT_ROM_MBIT_LBL, 0, 0, 0, 0, 0, SYS_UI_LBL_UNIT_MBIT},
    {MITEM_BREAK},
    {MITEM_LABEL, 0, 0, 0, 0, 0, 0, SYS_UI_LBL_DVD_ROM},
    {MITEM_TAB},
    {MITEM_TAB},
    {MITEM_VALUE, ROM_ID_DVD_ROM_ADDR, MITEM_FLAG_READONLY, MITEM_FORMAT_POINTER},
    {MITEM_DASH},
    {MITEM_TAB},
    {MITEM_VALUE, ROM_ID_DVD_ROM_SIZE, MITEM_FLAG_READONLY, MITEM_FORMAT_UDEC},
    {MITEM_TAB},
    {MITEM_LABEL, ROM_ID_DVD_ROM_MBIT_LBL, 0, 0, 0, 0, 0, SYS_UI_LBL_UNIT_MBIT},
    {MITEM_BREAK},
    {MITEM_LABEL, ROM_ID_ROM_UNCLEAN, 0, 0, 0, 0, 0, SYS_UI_LBL_ROM_UNCLEAN},
    {MITEM_BREAK},

    //Version information
    {MITEM_BREAK},

    {MITEM_LABEL, 0, 0, 0, 0, 0, 0, SYS_UI_LBL_ROMVER},
    {MITEM_TAB},
    {MITEM_TAB},
    {MITEM_STRING, ROM_ID_ROMVER, MITEM_FLAG_READONLY},
    {MITEM_BREAK},
    {MITEM_LABEL, 0, 0, 0, 0, 0, 0, SYS_UI_LBL_ROMGEN},
    {MITEM_TAB},
    {MITEM_TAB},
    {MITEM_VALUE, ROM_ID_ROMGEN_MMDD, MITEM_FLAG_READONLY, MITEM_FORMAT_HEX, 4},
    {MITEM_DASH},
    {MITEM_VALUE, ROM_ID_ROMGEN_YYYY, MITEM_FLAG_READONLY, MITEM_FORMAT_HEX, 4},
    {MITEM_TAB},
    {MITEM_BREAK},
    {MITEM_LABEL, 0, 0, 0, 0, 0, 0, SYS_UI_LBL_DVDPL},
    {MITEM_TAB},
    {MITEM_TAB},
    {MITEM_STRING, ROM_ID_DVDPLVER, MITEM_FLAG_READONLY},
    {MITEM_BREAK},
    {MITEM_LABEL, 0, 0, 0, 0, 0, 0, SYS_UI_LBL_PS1DRV},
    {MITEM_TAB},
    {MITEM_TAB},
    {MITEM_STRING, ROM_ID_PS1DRVVER, MITEM_FLAG_READONLY},
    {MITEM_BREAK},

    {MITEM_TERMINATOR}};

static struct UIMenuItem DEV9MenuItems[] = {
    {MITEM_LABEL, 0, 0, 0, 0, 0, 0, SYS_UI_LBL_SYS_INFO}, {MITEM_BREAK}, {MITEM_BREAK},

    {MITEM_LABEL, 0, 0, 0, 0, 0, 0, SYS_UI_LBL_DEV9},
    {MITEM_SEPERATOR},

    {MITEM_LABEL, DEV9_ID_NO_DEVICE, 0, 0, 0, 0, 0, SYS_UI_LBL_NO_EXPANSION_DEVICE},

    {MITEM_LABEL, DEV9_ID_SPEED_REV_LBL, 0, 0, 0, 0, 0, SYS_UI_LBL_SPEED_REVISION},
    {MITEM_TAB},
    {MITEM_TAB},
    {MITEM_VALUE, DEV9_ID_SPEED_REV, MITEM_FLAG_READONLY | MITEM_FLAG_UNIT_PREFIX, MITEM_FORMAT_HEX, 4},
    {MITEM_TAB},
    {MITEM_STRING, DEV9_ID_SPEED_NAME, MITEM_FLAG_READONLY},
    {MITEM_BREAK},
    {MITEM_LABEL, DEV9_ID_SPEED_CAPS_LBL, 0, 0, 0, 0, 0, SYS_UI_LBL_SPEED_CAPS},
    {MITEM_TAB},
    {MITEM_VALUE, DEV9_ID_SPEED_CAPS, MITEM_FLAG_READONLY, MITEM_FORMAT_HEX, 4},
    {MITEM_DOT, DEV9_ID_SPEED_CAPS_SEP},
    {MITEM_VALUE, DEV9_ID_SPEED_REV8, MITEM_FLAG_READONLY, MITEM_FORMAT_HEX, 4},
    {MITEM_BREAK},
    {MITEM_TAB},
    {MITEM_STRING, DEV9_ID_SPEED_CAPS_DESC, MITEM_FLAG_READONLY},
    {MITEM_BREAK},

    {MITEM_BREAK},

    {MITEM_LABEL, DEV9_ID_MAC_ADDR_LBL, 0, 0, 0, 0, 0, SYS_UI_LBL_MAC_ADDRESS},
    {MITEM_TAB},
    {MITEM_TAB},
    {MITEM_VALUE, DEV9_ID_MAC_ADDR_00, MITEM_FLAG_READONLY, MITEM_FORMAT_HEX, 2},
    {MITEM_COLON, DEV9_ID_MAC_ADDR_SEP_00},
    {MITEM_VALUE, DEV9_ID_MAC_ADDR_01, MITEM_FLAG_READONLY, MITEM_FORMAT_HEX, 2},
    {MITEM_COLON, DEV9_ID_MAC_ADDR_SEP_01},
    {MITEM_VALUE, DEV9_ID_MAC_ADDR_02, MITEM_FLAG_READONLY, MITEM_FORMAT_HEX, 2},
    {MITEM_COLON, DEV9_ID_MAC_ADDR_SEP_02},
    {MITEM_VALUE, DEV9_ID_MAC_ADDR_03, MITEM_FLAG_READONLY, MITEM_FORMAT_HEX, 2},
    {MITEM_COLON, DEV9_ID_MAC_ADDR_SEP_03},
    {MITEM_VALUE, DEV9_ID_MAC_ADDR_04, MITEM_FLAG_READONLY, MITEM_FORMAT_HEX, 2},
    {MITEM_COLON, DEV9_ID_MAC_ADDR_SEP_04},
    {MITEM_VALUE, DEV9_ID_MAC_ADDR_05, MITEM_FLAG_READONLY, MITEM_FORMAT_HEX, 2},
    {MITEM_BREAK},
    {MITEM_LABEL, DEV9_ID_PHY_VENDOR_LBL, 0, 0, 0, 0, 0, SYS_UI_LBL_PHY_VENDOR},
    {MITEM_TAB},
    {MITEM_TAB},
    {MITEM_VALUE, DEV9_ID_PHY_VENDOR_OUI, MITEM_FLAG_READONLY | MITEM_FLAG_UNIT_PREFIX, MITEM_FORMAT_HEX, 6},
    {MITEM_BREAK},
    {MITEM_TAB},
    {MITEM_STRING, DEV9_ID_PHY_VENDOR_NAME, MITEM_FLAG_READONLY},
    {MITEM_BREAK},

    {MITEM_LABEL, DEV9_ID_PHY_MODEL_LBL, 0, 0, 0, 0, 0, SYS_UI_LBL_PHY_MODEL},
    {MITEM_TAB},
    {MITEM_TAB},
    {MITEM_VALUE, DEV9_ID_PHY_MODEL, MITEM_FLAG_READONLY | MITEM_FLAG_UNIT_PREFIX, MITEM_FORMAT_HEX, 2},
    {MITEM_TAB},
    {MITEM_STRING, DEV9_ID_PHY_MODEL_DESC, MITEM_FLAG_READONLY},
    {MITEM_BREAK},
    {MITEM_LABEL, DEV9_ID_PHY_REVISION_LBL, 0, 0, 0, 0, 0, SYS_UI_LBL_PHY_REVISION},
    {MITEM_TAB},
    {MITEM_TAB},
    {MITEM_VALUE, DEV9_ID_PHY_REVISION, MITEM_FLAG_READONLY | MITEM_FLAG_UNIT_PREFIX, MITEM_FORMAT_HEX, 2},
    {MITEM_BREAK},

    {MITEM_TERMINATOR}};

static struct UIMenuItem ExtBusMenuItems[] = {
    {MITEM_LABEL, 0, 0, 0, 0, 0, 0, SYS_UI_LBL_SYS_INFO}, {MITEM_BREAK}, {MITEM_BREAK},

    {MITEM_LABEL, 0, 0, 0, 0, 0, 0, SYS_UI_LBL_USB},
    {MITEM_SEPERATOR},

    {MITEM_LABEL, 0, 0, 0, 0, 0, 0, SYS_UI_LBL_HC_REVISION},
    {MITEM_TAB},
    {MITEM_VALUE, EXTBUS_ID_USB_HC_REV_MAJOR, MITEM_FLAG_READONLY, MITEM_FORMAT_UDEC},
    {MITEM_DOT},
    {MITEM_VALUE, EXTBUS_ID_USB_HC_REV_MINOR, MITEM_FLAG_READONLY, MITEM_FORMAT_UDEC},
    {MITEM_BREAK},

    {MITEM_BREAK},
    {MITEM_LABEL, 0, 0, 0, 0, 0, 0, SYS_UI_LBL_ILINK},
    {MITEM_SEPERATOR},

    {MITEM_LABEL, 0, 0, 0, 0, 0, 0, SYS_UI_LBL_ILINK_PORTS},
    {MITEM_TAB},
    {MITEM_TAB},
    {MITEM_VALUE, EXTBUS_ID_ILINK_PORTS, MITEM_FLAG_READONLY, MITEM_FORMAT_UDEC},
    {MITEM_BREAK},
    {MITEM_LABEL, 0, 0, 0, 0, 0, 0, SYS_UI_LBL_ILINK_MAX_SPEED},
    {MITEM_TAB},
    {MITEM_VALUE, EXTBUS_ID_ILINK_MAX_SPEED, MITEM_FLAG_READONLY | MITEM_FLAG_UNIT_PREFIX, MITEM_FORMAT_HEX, 2},
    {MITEM_TAB},
    {MITEM_TAB},
    {MITEM_STRING, EXTBUS_ID_ILINK_MAX_SPEED_DESC, MITEM_FLAG_READONLY},
    {MITEM_BREAK},
    {MITEM_LABEL, 0, 0, 0, 0, 0, 0, SYS_UI_LBL_ILINK_COMPLIANCE},
    {MITEM_TAB},
    {MITEM_VALUE, EXTBUS_ID_ILINK_COMPLIANCE, MITEM_FLAG_READONLY | MITEM_FLAG_UNIT_PREFIX, MITEM_FORMAT_HEX, 2},
    {MITEM_TAB},
    {MITEM_TAB},
    {MITEM_STRING, EXTBUS_ID_ILINK_COMPLIANCE_DESC, MITEM_FLAG_READONLY},
    {MITEM_BREAK},
    {MITEM_LABEL, 0, 0, 0, 0, 0, 0, SYS_UI_LBL_ILINK_VENDOR_ID},
    {MITEM_TAB},
    {MITEM_VALUE, EXTBUS_ID_ILINK_VENDOR_ID, MITEM_FLAG_READONLY | MITEM_FLAG_UNIT_PREFIX, MITEM_FORMAT_HEX, 6},
    {MITEM_TAB},
    {MITEM_STRING, EXTBUS_ID_ILINK_VENDOR_DESC, MITEM_FLAG_READONLY},
    {MITEM_BREAK},
    {MITEM_LABEL, 0, 0, 0, 0, 0, 0, SYS_UI_LBL_ILINK_PRODUCT_ID},
    {MITEM_TAB},
    {MITEM_VALUE, EXTBUS_ID_ILINK_PRODUCT_ID, MITEM_FLAG_READONLY | MITEM_FLAG_UNIT_PREFIX, MITEM_FORMAT_HEX, 6},
    {MITEM_BREAK},
    {MITEM_TERMINATOR}};

//Forward declarations
static struct UIMenu EEGSReportMenu;
static struct UIMenu IOPSPU2ReportMenu;
static struct UIMenu BoardReportMenu;
static struct UIMenu Board2ReportMenu;
static struct UIMenu ROMReportMenu;
static struct UIMenu DEV9ReportMenu;
static struct UIMenu ExtBusReportMenu;

static struct UIMenu EEGSReportMenu    = {NULL, &IOPSPU2ReportMenu, EEGSMenuItems, {{BUTTON_TYPE_START, SYS_UI_LBL_DUMP}, {BUTTON_TYPE_SYS_CANCEL, SYS_UI_LBL_QUIT}}};
static struct UIMenu IOPSPU2ReportMenu = {&EEGSReportMenu, &BoardReportMenu, IOPSPU2MenuItems, {{BUTTON_TYPE_START, SYS_UI_LBL_DUMP}, {BUTTON_TYPE_SYS_CANCEL, SYS_UI_LBL_QUIT}}};
static struct UIMenu BoardReportMenu   = {&IOPSPU2ReportMenu, &Board2ReportMenu, BoardMenuItems, {{BUTTON_TYPE_START, SYS_UI_LBL_DUMP}, {BUTTON_TYPE_SYS_CANCEL, SYS_UI_LBL_QUIT}}};
static struct UIMenu Board2ReportMenu  = {&BoardReportMenu, &ROMReportMenu, Board2MenuItems, {{BUTTON_TYPE_START, SYS_UI_LBL_DUMP}, {BUTTON_TYPE_SYS_CANCEL, SYS_UI_LBL_QUIT}}};
static struct UIMenu ROMReportMenu     = {&BoardReportMenu, &DEV9ReportMenu, ROMMenuItems, {{BUTTON_TYPE_START, SYS_UI_LBL_DUMP}, {BUTTON_TYPE_SYS_CANCEL, SYS_UI_LBL_QUIT}}};
static struct UIMenu DEV9ReportMenu    = {&ROMReportMenu, &ExtBusReportMenu, DEV9MenuItems, {{BUTTON_TYPE_START, SYS_UI_LBL_DUMP}, {BUTTON_TYPE_SYS_CANCEL, SYS_UI_LBL_QUIT}}};
static struct UIMenu ExtBusReportMenu  = {&DEV9ReportMenu, NULL, ExtBusMenuItems, {{BUTTON_TYPE_START, SYS_UI_LBL_DUMP}, {BUTTON_TYPE_SYS_CANCEL, SYS_UI_LBL_QUIT}}};

/*
	In this whole file, some variables and values used would be:

	SelectedMenu ->	0 = The game list.
			1 = The control panel.

	SelectedMenuOption -> The element in the menu that is currently selected.
*/

#ifndef DSNET_HOST_SUPPORT
#define NUM_SUPPORTED_DEVICES 3
#else
#define NUM_SUPPORTED_DEVICES 4
#endif

struct SupportedDevice
{
    const char *name;
    const char *label, *UnitLabel;
    unsigned char unit;
    unsigned char icon;
    unsigned char IsReady;
};

static int GetUserSaveDeviceSelection(char *SelectedDevice, const struct RequiredFileSpaceStat *RequiredSpaceStats, unsigned int NumFiles)
{
    unsigned char done, NumDevicesAvailable, SelectedDeviceIndex, DeviceID;
    unsigned int PadStatus;
    int result, i, devicesInRow, deviceRow;
    GS_RGBAQ FontColour;
    static struct SupportedDevice devices[NUM_SUPPORTED_DEVICES] = {
        {
            "mc",
            NULL,
            NULL,
            0,
            DEVICE_TYPE_DISK,
            0,
        },
        {
            "mc",
            NULL,
            NULL,
            1,
            DEVICE_TYPE_DISK,
            0,
        },
        {
            "mass",
            NULL,
            NULL,
            0,
            DEVICE_TYPE_USB_DISK,
            0,
        },
#ifdef DSNET_HOST_SUPPORT
        {
            "host",
            NULL,
            NULL,
            0,
            DEVICE_TYPE_USB_DISK,
            0,
        },
#endif
    };
    static const unsigned int IconFileSelMenuDevStringIDs[NUM_SUPPORTED_DEVICES] = {
        SYS_UI_LBL_DEV_MC,
        SYS_UI_LBL_DEV_MC,
        SYS_UI_LBL_DEV_MASS,
#ifdef DSNET_HOST_SUPPORT
        SYS_UI_LBL_DEV_MASS,
#endif
    };
    static const unsigned int IconFileSelMenuDevUnitStringIDs[NUM_SUPPORTED_DEVICES] = {
        SYS_UI_LBL_MC_SLOT_1,
        SYS_UI_LBL_MC_SLOT_2,
        SYS_UI_LBL_COUNT,
#ifdef DSNET_HOST_SUPPORT
        SYS_UI_LBL_COUNT,
#endif
    };

    //Allow the user to browse for icon sets on mc0:, mc1: and mass:.

    for (i = 0; i < NUM_SUPPORTED_DEVICES; i++)
    {
        devices[i].label     = GetUILabel(IconFileSelMenuDevStringIDs[i]);
        devices[i].UnitLabel = IconFileSelMenuDevUnitStringIDs[i] != SYS_UI_LBL_COUNT ? GetUILabel(IconFileSelMenuDevUnitStringIDs[i]) : NULL;
    }

    StartDevicePollingThread();

    done                = 0;
    SelectedDeviceIndex = 0;
    result              = 0;
    while (!done)
    {
        DrawBackground(&UIDrawGlobal, &BackgroundTexture);

        for (i = 0, NumDevicesAvailable = 0; i < NUM_SUPPORTED_DEVICES; i++)
        {
            if (GetIsDeviceUnitReady(devices[i].name, devices[i].unit) == 1)
            {
                devices[i].IsReady = 1;
                NumDevicesAvailable++;
            }
            else
                devices[i].IsReady = 0;
        }

        PadStatus = ReadCombinedPadStatus();

        FontPrintf(&UIDrawGlobal, 16, 16, 1, 1.2f, GS_WHITE_FONT, GetUILabel(SYS_UI_LBL_SELECT_DEVICE));

        if (NumDevicesAvailable > 0)
        {
            // Draw the legend.
            DrawButtonLegend(&UIDrawGlobal, &PadLayoutTexture, SelectButton == PAD_CIRCLE ? BUTTON_TYPE_CIRCLE : BUTTON_TYPE_CROSS, 50, 404, 2);
            FontPrintf(&UIDrawGlobal, 75, 406, 2, 1.0f, GS_WHITE_FONT, GetUILabel(SYS_UI_LBL_OK));

            if (SelectedDeviceIndex >= NumDevicesAvailable)
            {
                SelectedDeviceIndex = NumDevicesAvailable > 0 ? NumDevicesAvailable - 1 : 0;
            }

            if (PadStatus & PAD_LEFT)
            {
                if (SelectedDeviceIndex > 0)
                    SelectedDeviceIndex--;
            }
            else if (PadStatus & PAD_RIGHT)
            {
                if (SelectedDeviceIndex < NumDevicesAvailable - 1)
                    SelectedDeviceIndex++;
            }
            if (PadStatus & PAD_UP)
            {
                if (SelectedDeviceIndex - MAX_DEVICES_IN_ROW >= 0)
                    SelectedDeviceIndex -= MAX_DEVICES_IN_ROW;
            }
            else if (PadStatus & PAD_DOWN)
            {
                if (SelectedDeviceIndex + MAX_DEVICES_IN_ROW <= NumDevicesAvailable - 1)
                    SelectedDeviceIndex += MAX_DEVICES_IN_ROW;
            }

            //Display a list of available devices and allow the user to choose a device to browse for icons from.
            for (i = 0, DeviceID = 0, devicesInRow = 0, deviceRow = 0; i < NumDevicesAvailable; DeviceID++)
            {
                if (devices[DeviceID].IsReady)
                {
                    FontColour = (i == SelectedDeviceIndex) ? GS_YELLOW_FONT : GS_BLUE_FONT;
                    DrawDeviceIcon(&UIDrawGlobal, &DeviceIconTexture, devices[DeviceID].icon, DEVICE_LIST_X + 32 + 200 * devicesInRow, DEVICE_LIST_Y + 100 * deviceRow, 1);
                    FontPrintf(&UIDrawGlobal, DEVICE_LIST_X + 200 * devicesInRow, DEVICE_LIST_Y + 48 + 100 * deviceRow, 1, 1.0f, FontColour, devices[DeviceID].label);
                    if (devices[DeviceID].UnitLabel != NULL)
                        FontPrintf(&UIDrawGlobal, DEVICE_LIST_X + 200 * devicesInRow, DEVICE_LIST_Y + 64 + 100 * deviceRow, 1, 1.0f, FontColour, devices[DeviceID].UnitLabel);
                    i++;
                    devicesInRow++;
                    if (devicesInRow == MAX_DEVICES_IN_ROW)
                    {
                        devicesInRow = 0;
                        deviceRow++;
                    }
                }
            }

            if (PadStatus & SelectButton)
            {
                for (i = 0, DeviceID = 0; DeviceID < NUM_SUPPORTED_DEVICES; DeviceID++)
                {
                    if (devices[DeviceID].IsReady)
                    {
                        if (i == SelectedDeviceIndex)
                        {
                            break;
                        }

                        i++;
                    }
                }
                SelectedDeviceIndex = DeviceID;

                StopDevicePollingThread();
                if ((result = GetHasDeviceSufficientSpace(devices[SelectedDeviceIndex].name, devices[SelectedDeviceIndex].unit, RequiredSpaceStats, NumFiles)) == 1)
                {
                    sprintf(SelectedDevice, "%s%u:", devices[SelectedDeviceIndex].name, devices[SelectedDeviceIndex].unit);
                    result = 0;
                    done   = 1;
                }
                else if (result == 0)
                {
                    DisplayErrorMessage(SYS_UI_MSG_INSUF_SPACE_ERR);
                }
                else
                {
                    DisplayErrorMessage(SYS_UI_MSG_DEV_ACC_ERR);
                }
                StartDevicePollingThread();
            }
        }

        // Draw the legend.
        DrawButtonLegend(&UIDrawGlobal, &PadLayoutTexture, CancelButton == PAD_CROSS ? BUTTON_TYPE_CROSS : BUTTON_TYPE_CIRCLE, 200, 404, 2);
        FontPrintf(&UIDrawGlobal, 240, 406, 2, 1.0f, GS_WHITE_FONT, GetUILabel(SYS_UI_LBL_CANCEL));

        if (PadStatus & CancelButton)
        {
            result = 1;
            done   = 1;
        }

        SyncFlipFB(&UIDrawGlobal);
    }

    StopDevicePollingThread();

    return result;
}

static int DumpSystemROMScreen(const struct SystemInformation *SystemInformation)
{
    char DumpPath[96];
    struct RequiredFileSpaceStat RequiredSpace[7];
    int i, result;
    unsigned int NumFiles;

    NumFiles                = 3;

    //Start with the root folder.
    RequiredSpace[0].IsFile = 0;
    RequiredSpace[0].length = 0;

    //NVRAM dump
    RequiredSpace[1].IsFile = 1;
    RequiredSpace[1].length = 1024;

    //Log file
    RequiredSpace[2].IsFile = 1;
    RequiredSpace[2].length = 2048; //Shouldn't get larger than this.

    //Boot ROM
    if (SystemInformation->mainboard.BOOT_ROM.IsExists)
    {
        RequiredSpace[NumFiles].IsFile = 1;
        RequiredSpace[NumFiles].length = SystemInformation->mainboard.BOOT_ROM.size;
        NumFiles++;
    }

    //DVD ROM
    if (SystemInformation->mainboard.DVD_ROM.IsExists)
    {
        RequiredSpace[NumFiles].IsFile = 1;
        RequiredSpace[NumFiles].length = SystemInformation->mainboard.DVD_ROM.size;
        NumFiles++;
    }

    if ((GetUserSaveDeviceSelection(DumpPath, RequiredSpace, NumFiles)) == 0)
    {
#ifndef DSNET_HOST_SUPPORT
        if (!(SystemInformation->mainboard.status & PS2IDB_STAT_ERR_CONSOLEID))
        {
            u32 serial;

            serial = ((u32)SystemInformation->ConsoleID[6]) << 16 | ((u32)SystemInformation->ConsoleID[5]) << 8 | ((u32)SystemInformation->ConsoleID[4]);
            sprintf(&DumpPath[strlen(DumpPath)], "/%s_%07u", SystemInformation->mainboard.ModelName, serial);
        }
        else
        {
            sprintf(&DumpPath[strlen(DumpPath)], "/%s_noserial", SystemInformation->mainboard.ModelName);
        }

        DisplayFlashStatusUpdate(SYS_UI_MSG_PLEASE_WAIT);

        DEBUG_PRINTF("Cleaning up %s\n", DumpPath);
        RemoveFolder(DumpPath);
        DEBUG_PRINTF("Creating %s\n", DumpPath);
        if ((result = mkdir(DumpPath) >= 0) || result == -EEXIST)
        {
            DumpSystemROM(DumpPath, SystemInformation);
        }
        else
        {
            DisplayErrorMessage(SYS_UI_MSG_DEV_ACC_ERR);
        }
#else
        DumpSystemROM(DumpPath, SystemInformation);
#endif
    }

    return 0;
}

static void LoadEEGSInformation(const struct SystemInformation *SystemInformation)
{
    //EE
    UISetValue(&EEGSReportMenu, EEGS_ID_EE_IMPL, SystemInformation->mainboard.ee.implementation);
    UISetValue(&EEGSReportMenu, EEGS_ID_EE_REV_MAJOR, SystemInformation->mainboard.ee.revision >> 4);
    UISetValue(&EEGSReportMenu, EEGS_ID_EE_REV_MINOR, SystemInformation->mainboard.ee.revision & 0xF);
    UISetString(&EEGSReportMenu, EEGS_ID_EE_NAME, GetEEChipDesc((u16)(SystemInformation->mainboard.ee.implementation) << 8 | SystemInformation->mainboard.ee.revision));
    UISetValue(&EEGSReportMenu, EEGS_ID_EE_FPU_IMPL, SystemInformation->mainboard.ee.FPUImplementation);
    UISetValue(&EEGSReportMenu, EEGS_ID_EE_FPU_REV_MAJOR, SystemInformation->mainboard.ee.FPURevision >> 4);
    UISetValue(&EEGSReportMenu, EEGS_ID_EE_FPU_REV_MINOR, SystemInformation->mainboard.ee.FPURevision & 0xF);
    UISetValue(&EEGSReportMenu, EEGS_ID_EE_ICACHE_SIZE, CalculateCPUCacheSize(SystemInformation->mainboard.ee.ICacheSize) / 1024);
    UISetValue(&EEGSReportMenu, EEGS_ID_EE_DCACHE_SIZE, CalculateCPUCacheSize(SystemInformation->mainboard.ee.DCacheSize) / 1024);
    UISetValue(&EEGSReportMenu, EEGS_ID_EE_RAM_SIZE, SystemInformation->mainboard.ee.RAMSize);

    //GS
    UISetValue(&EEGSReportMenu, EEGS_ID_GS_REV_MAJOR, SystemInformation->mainboard.gs.revision >> 4);
    UISetValue(&EEGSReportMenu, EEGS_ID_GS_REV_MINOR, SystemInformation->mainboard.gs.revision & 0xF);
    UISetString(&EEGSReportMenu, EEGS_ID_GS_NAME, GetGSChipDesc((u16)(SystemInformation->mainboard.gs.id) << 8 | SystemInformation->mainboard.gs.revision));

    UISetValue(&EEGSReportMenu, EEGS_ID_GS_ID, SystemInformation->mainboard.gs.id);
}

static void LoadIOPSPU2Information(const struct SystemInformation *SystemInformation)
{
    //IOP
    UISetValue(&IOPSPU2ReportMenu, IOPSPU2_ID_IOP_REV_MAJOR, SystemInformation->mainboard.iop.revision >> 4);
    UISetValue(&IOPSPU2ReportMenu, IOPSPU2_ID_IOP_REV_MINOR, SystemInformation->mainboard.iop.revision & 0xF);
    UISetString(&IOPSPU2ReportMenu, IOPSPU2_ID_IOP_NAME, GetIOPChipDesc(SystemInformation->mainboard.iop.revision));
    UISetValue(&IOPSPU2ReportMenu, IOPSPU2_ID_IOP_RAM_SIZE, SystemInformation->mainboard.iop.RAMSize);

    //SPU2
    UISetValue(&IOPSPU2ReportMenu, IOPSPU2_ID_SPU2_REV, SystemInformation->mainboard.spu2.revision);
    UISetString(&IOPSPU2ReportMenu, IOPSPU2_ID_SPU2_NAME, GetSPU2ChipDesc(SystemInformation->mainboard.spu2.revision));
}

static void LoadBoardInformation(const struct SystemInformation *SystemInformation)
{
    if (!(SystemInformation->mainboard.status & PS2IDB_STAT_ERR_MNAME))
    {
        UISetType(&BoardReportMenu, BOARD_ID_MODEL_NAME, MITEM_STRING);
        UISetString(&BoardReportMenu, BOARD_ID_MODEL_NAME, SystemInformation->mainboard.ModelName);
    }
    else
    {
        UISetType(&BoardReportMenu, BOARD_ID_MODEL_NAME, MITEM_DASH);
    }
    UISetString(&BoardReportMenu, BOARD_ID_BOARD_NAME, SystemInformation->mainboard.MainboardName);
    UISetString(&BoardReportMenu, BOARD_ID_CHASSIS_NAME, SystemInformation->chassis);
    UISetValue(&BoardReportMenu, BOARD_ID_MACHINE_TYPE, SystemInformation->mainboard.MachineType);
    UISetValue(&BoardReportMenu, BOARD_ID_MPU_ID, SystemInformation->mainboard.MPUBoardID);
    UISetValue(&BoardReportMenu, BOARD_ID_BOARD_INF, SystemInformation->mainboard.BoardInf);
    UISetString(&BoardReportMenu, BOARD_ID_MRP_DESC, GetMRPDesc(SystemInformation->mainboard.BoardInf));
    UISetValue(&BoardReportMenu, BOARD_ID_SSBUSIF_REV_MAJOR, SystemInformation->mainboard.ssbus.revision >> 4);
    UISetValue(&BoardReportMenu, BOARD_ID_SSBUSIF_REV_MINOR, SystemInformation->mainboard.ssbus.revision & 0xF);
    UISetString(&BoardReportMenu, BOARD_ID_SSBUSIF_NAME, GetSSBUSIFDesc(SystemInformation->mainboard.ssbus.revision));
    if (SystemInformation->mainboard.ssbus.status & PS2DB_SSBUS_HAS_AIF)
    {
        UISetType(&BoardReportMenu, BOARD_ID_AIF_REV, MITEM_VALUE);
        UISetValue(&BoardReportMenu, BOARD_ID_AIF_REV, SystemInformation->mainboard.ssbus.AIFRevision);
    }
    else
    {
        UISetType(&BoardReportMenu, BOARD_ID_AIF_REV, MITEM_DASH);
    }
}

static void LoadBoard2Information(const struct SystemInformation *SystemInformation)
{
    u32 modelID;
    u16 conModelID;

    if (!(SystemInformation->mainboard.status & PS2IDB_STAT_ERR_MVER))
    {
        UISetType(&Board2ReportMenu, BOARD2_ID_MECHA_REV_MAJOR, MITEM_VALUE);
        UISetValue(&Board2ReportMenu, BOARD2_ID_MECHA_REV_MAJOR, SystemInformation->mainboard.MECHACONVersion[1]);
        UISetType(&Board2ReportMenu, BOARD2_ID_MECHA_REV_MINOR, MITEM_VALUE);
        UISetValue(&Board2ReportMenu, BOARD2_ID_MECHA_REV_MINOR, SystemInformation->mainboard.MECHACONVersion[2]);
        UISetType(&Board2ReportMenu, BOARD2_ID_MECHA_NAME, MITEM_STRING);
        UISetString(&Board2ReportMenu, BOARD2_ID_MECHA_NAME, GetMECHACONChipDesc((u32)(SystemInformation->mainboard.MECHACONVersion[1]) << 8 | (u32)(SystemInformation->mainboard.MECHACONVersion[2])));
        UISetType(&Board2ReportMenu, BOARD2_ID_MECHA_REGION, MITEM_VALUE);
        UISetValue(&Board2ReportMenu, BOARD2_ID_MECHA_REGION, SystemInformation->mainboard.MECHACONVersion[0]);
        UISetType(&Board2ReportMenu, BOARD2_ID_MECHA_REGION_NAME, MITEM_STRING);
        UISetString(&Board2ReportMenu, BOARD2_ID_MECHA_REGION_NAME, GetRegionDesc(SystemInformation->mainboard.MECHACONVersion[0]));
        UISetType(&Board2ReportMenu, BOARD2_ID_MECHA_TYPE, MITEM_VALUE);
        UISetValue(&Board2ReportMenu, BOARD2_ID_MECHA_TYPE, SystemInformation->mainboard.MECHACONVersion[3]);
        UISetType(&Board2ReportMenu, BOARD2_ID_MECHA_TYPE_NAME, MITEM_STRING);
        UISetString(&Board2ReportMenu, BOARD2_ID_MECHA_TYPE_NAME, GetSystemTypeDesc(SystemInformation->mainboard.MECHACONVersion[3]));
    }
    else
    {
        UISetType(&Board2ReportMenu, BOARD2_ID_MECHA_REV_MAJOR, MITEM_DASH);
        UISetType(&Board2ReportMenu, BOARD2_ID_MECHA_REV_MINOR, MITEM_DASH);
        UISetType(&Board2ReportMenu, BOARD2_ID_MECHA_NAME, MITEM_DASH);
        UISetType(&Board2ReportMenu, BOARD2_ID_MECHA_REGION, MITEM_DASH);
        UISetType(&Board2ReportMenu, BOARD2_ID_MECHA_REGION_NAME, MITEM_DASH);
        UISetType(&Board2ReportMenu, BOARD2_ID_MECHA_TYPE, MITEM_DASH);
        UISetType(&Board2ReportMenu, BOARD2_ID_MECHA_TYPE_NAME, MITEM_DASH);
    }

    if (!(SystemInformation->mainboard.status & PS2IDB_STAT_ERR_ADD010))
    {
        UISetType(&Board2ReportMenu, BOARD2_ID_ADD010, MITEM_VALUE);
        UISetValue(&Board2ReportMenu, BOARD2_ID_ADD010, SystemInformation->mainboard.ADD010);
        UISetType(&Board2ReportMenu, BOARD2_ID_ADD010_DESC, MITEM_STRING);
        UISetString(&Board2ReportMenu, BOARD2_ID_ADD010_DESC, GetADD010Desc(SystemInformation->mainboard.ADD010));
    }
    else
    {
        UISetType(&Board2ReportMenu, BOARD2_ID_ADD010, MITEM_DASH);
        UISetType(&Board2ReportMenu, BOARD2_ID_ADD010_DESC, MITEM_DASH);
    }

    if (SystemInformation->mainboard.MECHACONVersion[1] < 5 || (SystemInformation->mainboard.status & PS2IDB_STAT_ERR_MRENEWDATE))
    {
        UISetType(&Board2ReportMenu, BOARD2_ID_MECHA_RENEWAL_YEAR, MITEM_DASH);
        UISetType(&Board2ReportMenu, BOARD2_ID_MECHA_RENEWAL_MONTH, MITEM_DASH);
        UISetType(&Board2ReportMenu, BOARD2_ID_MECHA_RENEWAL_DATE, MITEM_DASH);
        UISetType(&Board2ReportMenu, BOARD2_ID_MECHA_RENEWAL_HOUR, MITEM_DASH);
        UISetType(&Board2ReportMenu, BOARD2_ID_MECHA_RENEWAL_MINUTE, MITEM_DASH);
    }
    else
    {
        UISetType(&Board2ReportMenu, BOARD2_ID_MECHA_RENEWAL_YEAR, MITEM_VALUE);
        UISetValue(&Board2ReportMenu, BOARD2_ID_MECHA_RENEWAL_YEAR, SystemInformation->mainboard.MRenewalDate[0] + 0x2000);
        UISetType(&Board2ReportMenu, BOARD2_ID_MECHA_RENEWAL_MONTH, MITEM_VALUE);
        UISetValue(&Board2ReportMenu, BOARD2_ID_MECHA_RENEWAL_MONTH, SystemInformation->mainboard.MRenewalDate[1]);
        UISetType(&Board2ReportMenu, BOARD2_ID_MECHA_RENEWAL_DATE, MITEM_VALUE);
        UISetValue(&Board2ReportMenu, BOARD2_ID_MECHA_RENEWAL_DATE, SystemInformation->mainboard.MRenewalDate[2]);
        UISetType(&Board2ReportMenu, BOARD2_ID_MECHA_RENEWAL_HOUR, MITEM_VALUE);
        UISetValue(&Board2ReportMenu, BOARD2_ID_MECHA_RENEWAL_HOUR, SystemInformation->mainboard.MRenewalDate[3]);
        UISetType(&Board2ReportMenu, BOARD2_ID_MECHA_RENEWAL_MINUTE, MITEM_VALUE);
        UISetValue(&Board2ReportMenu, BOARD2_ID_MECHA_RENEWAL_MINUTE, SystemInformation->mainboard.MRenewalDate[4]);
    }

    if (!(SystemInformation->mainboard.status & PS2IDB_STAT_ERR_ILINKID))
    {
        modelID = SystemInformation->mainboard.ModelID[0] | SystemInformation->mainboard.ModelID[1] << 8 | SystemInformation->mainboard.ModelID[2] << 16;

        UISetType(&Board2ReportMenu, BOARD2_ID_MODEL_ID, MITEM_VALUE);
        UISetValue(&Board2ReportMenu, BOARD2_ID_MODEL_ID, modelID);
        UISetType(&Board2ReportMenu, BOARD2_ID_MODEL_ID_DESC, MITEM_STRING);
        UISetString(&Board2ReportMenu, BOARD2_ID_MODEL_ID_DESC, GetModelIDDesc(conModelID));

        UISetType(&Board2ReportMenu, BOARD2_ID_ILINK_ID_00, MITEM_VALUE);
        UISetValue(&Board2ReportMenu, BOARD2_ID_ILINK_ID_00, SystemInformation->iLinkID[0]);
        UISetType(&Board2ReportMenu, BOARD2_ID_ILINK_ID_01, MITEM_VALUE);
        UISetValue(&Board2ReportMenu, BOARD2_ID_ILINK_ID_01, SystemInformation->iLinkID[1]);
        UISetType(&Board2ReportMenu, BOARD2_ID_ILINK_ID_02, MITEM_VALUE);
        UISetValue(&Board2ReportMenu, BOARD2_ID_ILINK_ID_02, SystemInformation->iLinkID[2]);
        UISetType(&Board2ReportMenu, BOARD2_ID_ILINK_ID_03, MITEM_VALUE);
        UISetValue(&Board2ReportMenu, BOARD2_ID_ILINK_ID_03, SystemInformation->iLinkID[3]);
        UISetType(&Board2ReportMenu, BOARD2_ID_ILINK_ID_04, MITEM_VALUE);
        UISetValue(&Board2ReportMenu, BOARD2_ID_ILINK_ID_04, SystemInformation->iLinkID[4]);
        UISetType(&Board2ReportMenu, BOARD2_ID_ILINK_ID_05, MITEM_VALUE);
        UISetValue(&Board2ReportMenu, BOARD2_ID_ILINK_ID_05, SystemInformation->iLinkID[5]);
        UISetType(&Board2ReportMenu, BOARD2_ID_ILINK_ID_06, MITEM_VALUE);
        UISetValue(&Board2ReportMenu, BOARD2_ID_ILINK_ID_06, SystemInformation->iLinkID[6]);
        UISetType(&Board2ReportMenu, BOARD2_ID_ILINK_ID_07, MITEM_VALUE);
        UISetValue(&Board2ReportMenu, BOARD2_ID_ILINK_ID_07, SystemInformation->iLinkID[7]);
    }
    else
    {
        UISetType(&Board2ReportMenu, BOARD2_ID_MODEL_ID, MITEM_DASH);
        UISetType(&Board2ReportMenu, BOARD2_ID_MODEL_ID_DESC, MITEM_DASH);

        UISetType(&Board2ReportMenu, BOARD2_ID_ILINK_ID_00, MITEM_DASH);
        UISetType(&Board2ReportMenu, BOARD2_ID_ILINK_ID_01, MITEM_DASH);
        UISetType(&Board2ReportMenu, BOARD2_ID_ILINK_ID_02, MITEM_DASH);
        UISetType(&Board2ReportMenu, BOARD2_ID_ILINK_ID_03, MITEM_DASH);
        UISetType(&Board2ReportMenu, BOARD2_ID_ILINK_ID_04, MITEM_DASH);
        UISetType(&Board2ReportMenu, BOARD2_ID_ILINK_ID_05, MITEM_DASH);
        UISetType(&Board2ReportMenu, BOARD2_ID_ILINK_ID_06, MITEM_DASH);
        UISetType(&Board2ReportMenu, BOARD2_ID_ILINK_ID_07, MITEM_DASH);
    }

    if (!(SystemInformation->mainboard.status & PS2IDB_STAT_ERR_CONSOLEID))
    {
        conModelID = SystemInformation->mainboard.ConModelID[0] | SystemInformation->mainboard.ConModelID[1] << 8;

        UISetType(&Board2ReportMenu, BOARD2_ID_SERIAL, MITEM_VALUE);
        UISetValue(&Board2ReportMenu, BOARD2_ID_SERIAL, ((u32)SystemInformation->ConsoleID[6]) << 16 | ((u32)SystemInformation->ConsoleID[5]) << 8 | ((u32)SystemInformation->ConsoleID[4]));
        UISetType(&Board2ReportMenu, BOARD2_ID_CON_MODEL_ID, MITEM_VALUE);
        UISetValue(&Board2ReportMenu, BOARD2_ID_CON_MODEL_ID, conModelID);
        UISetType(&Board2ReportMenu, BOARD2_ID_EMCS_ID, MITEM_VALUE);
        UISetValue(&Board2ReportMenu, BOARD2_ID_EMCS_ID, SystemInformation->mainboard.EMCSID);
        UISetType(&Board2ReportMenu, BOARD2_ID_EMCS_ID_DESC, MITEM_STRING);
        UISetString(&Board2ReportMenu, BOARD2_ID_EMCS_ID_DESC, GetEMCSIDDesc(SystemInformation->mainboard.EMCSID));

        UISetType(&Board2ReportMenu, BOARD2_ID_CONSOLE_ID_00, MITEM_VALUE);
        UISetValue(&Board2ReportMenu, BOARD2_ID_CONSOLE_ID_00, SystemInformation->ConsoleID[0]);
        UISetType(&Board2ReportMenu, BOARD2_ID_CONSOLE_ID_01, MITEM_VALUE);
        UISetValue(&Board2ReportMenu, BOARD2_ID_CONSOLE_ID_01, SystemInformation->ConsoleID[1]);
        UISetType(&Board2ReportMenu, BOARD2_ID_CONSOLE_ID_02, MITEM_VALUE);
        UISetValue(&Board2ReportMenu, BOARD2_ID_CONSOLE_ID_02, SystemInformation->ConsoleID[2]);
        UISetType(&Board2ReportMenu, BOARD2_ID_CONSOLE_ID_03, MITEM_VALUE);
        UISetValue(&Board2ReportMenu, BOARD2_ID_CONSOLE_ID_03, SystemInformation->ConsoleID[3]);
        UISetType(&Board2ReportMenu, BOARD2_ID_CONSOLE_ID_04, MITEM_VALUE);
        UISetValue(&Board2ReportMenu, BOARD2_ID_CONSOLE_ID_04, SystemInformation->ConsoleID[4]);
        UISetType(&Board2ReportMenu, BOARD2_ID_CONSOLE_ID_05, MITEM_VALUE);
        UISetValue(&Board2ReportMenu, BOARD2_ID_CONSOLE_ID_05, SystemInformation->ConsoleID[5]);
        UISetType(&Board2ReportMenu, BOARD2_ID_CONSOLE_ID_06, MITEM_VALUE);
        UISetValue(&Board2ReportMenu, BOARD2_ID_CONSOLE_ID_06, SystemInformation->ConsoleID[6]);
        UISetType(&Board2ReportMenu, BOARD2_ID_CONSOLE_ID_07, MITEM_VALUE);
        UISetValue(&Board2ReportMenu, BOARD2_ID_CONSOLE_ID_07, SystemInformation->ConsoleID[7]);
    }
    else
    {
        UISetType(&Board2ReportMenu, BOARD2_ID_SERIAL, MITEM_DASH);
        UISetType(&Board2ReportMenu, BOARD2_ID_CON_MODEL_ID, MITEM_DASH);
        UISetType(&Board2ReportMenu, BOARD2_ID_EMCS_ID, MITEM_DASH);
        UISetType(&Board2ReportMenu, BOARD2_ID_EMCS_ID_DESC, MITEM_DASH);

        UISetType(&Board2ReportMenu, BOARD2_ID_CONSOLE_ID_00, MITEM_DASH);
        UISetType(&Board2ReportMenu, BOARD2_ID_CONSOLE_ID_01, MITEM_DASH);
        UISetType(&Board2ReportMenu, BOARD2_ID_CONSOLE_ID_02, MITEM_DASH);
        UISetType(&Board2ReportMenu, BOARD2_ID_CONSOLE_ID_03, MITEM_DASH);
        UISetType(&Board2ReportMenu, BOARD2_ID_CONSOLE_ID_04, MITEM_DASH);
        UISetType(&Board2ReportMenu, BOARD2_ID_CONSOLE_ID_05, MITEM_DASH);
        UISetType(&Board2ReportMenu, BOARD2_ID_CONSOLE_ID_06, MITEM_DASH);
        UISetType(&Board2ReportMenu, BOARD2_ID_CONSOLE_ID_07, MITEM_DASH);
    }
}

static void LoadROMInformation(const struct SystemInformation *SystemInformation)
{
    int MayBeModded;

    MayBeModded = CheckROM(&SystemInformation->mainboard);
    if (MayBeModded)
    {
        UISetVisible(&ROMReportMenu, ROM_ID_ROM_UNCLEAN, 1);
    }
    else
    {
        UISetVisible(&ROMReportMenu, ROM_ID_ROM_UNCLEAN, 0);
    }

    //ROM region and chip information
    if (SystemInformation->ROMs[0].IsExists)
    {
        UISetValue(&ROMReportMenu, ROM_ID_ROM0_ADDR, SystemInformation->ROMs[0].StartAddress);
        UISetValue(&ROMReportMenu, ROM_ID_ROM0_SIZE, SystemInformation->ROMs[0].size);
        UISetVisible(&ROMReportMenu, ROM_ID_ROM0_ADDR, 1);
        UISetVisible(&ROMReportMenu, ROM_ID_ROM0_SIZE, 1);
        UISetVisible(&ROMReportMenu, ROM_ID_ROM0_BYTES_LBL, 1);
    }
    else
    {
        UISetVisible(&ROMReportMenu, ROM_ID_ROM0_ADDR, 0);
        UISetVisible(&ROMReportMenu, ROM_ID_ROM0_SIZE, 0);
        UISetVisible(&ROMReportMenu, ROM_ID_ROM0_BYTES_LBL, 0);
    }

    if (SystemInformation->ROMs[1].IsExists)
    {
        UISetValue(&ROMReportMenu, ROM_ID_ROM1_ADDR, SystemInformation->ROMs[1].StartAddress);
        UISetValue(&ROMReportMenu, ROM_ID_ROM1_SIZE, SystemInformation->ROMs[1].size);
        UISetVisible(&ROMReportMenu, ROM_ID_ROM1_ADDR, 1);
        UISetVisible(&ROMReportMenu, ROM_ID_ROM1_SIZE, 1);
        UISetVisible(&ROMReportMenu, ROM_ID_ROM1_BYTES_LBL, 1);
    }
    else
    {
        UISetVisible(&ROMReportMenu, ROM_ID_ROM1_ADDR, 0);
        UISetVisible(&ROMReportMenu, ROM_ID_ROM1_SIZE, 0);
        UISetVisible(&ROMReportMenu, ROM_ID_ROM1_BYTES_LBL, 0);
    }

    if (SystemInformation->ROMs[2].IsExists)
    {
        UISetValue(&ROMReportMenu, ROM_ID_ROM2_ADDR, SystemInformation->ROMs[2].StartAddress);
        UISetValue(&ROMReportMenu, ROM_ID_ROM2_SIZE, SystemInformation->ROMs[2].size);
        UISetVisible(&ROMReportMenu, ROM_ID_ROM2_ADDR, 1);
        UISetVisible(&ROMReportMenu, ROM_ID_ROM2_SIZE, 1);
        UISetVisible(&ROMReportMenu, ROM_ID_ROM2_BYTES_LBL, 1);
    }
    else
    {
        UISetVisible(&ROMReportMenu, ROM_ID_ROM2_ADDR, 0);
        UISetVisible(&ROMReportMenu, ROM_ID_ROM2_SIZE, 0);
        UISetVisible(&ROMReportMenu, ROM_ID_ROM2_BYTES_LBL, 0);
    }

    if (SystemInformation->erom.IsExists)
    {
        UISetValue(&ROMReportMenu, ROM_ID_EROM_ADDR, SystemInformation->erom.StartAddress);
        UISetValue(&ROMReportMenu, ROM_ID_EROM_SIZE, SystemInformation->erom.size);
        UISetVisible(&ROMReportMenu, ROM_ID_EROM_ADDR, 1);
        UISetVisible(&ROMReportMenu, ROM_ID_EROM_SIZE, 1);
        UISetVisible(&ROMReportMenu, ROM_ID_EROM_BYTES_LBL, 1);
    }
    else
    {
        UISetVisible(&ROMReportMenu, ROM_ID_EROM_ADDR, 0);
        UISetVisible(&ROMReportMenu, ROM_ID_EROM_SIZE, 0);
        UISetVisible(&ROMReportMenu, ROM_ID_EROM_BYTES_LBL, 0);
    }

    if (SystemInformation->mainboard.BOOT_ROM.IsExists)
    {
        UISetValue(&ROMReportMenu, ROM_ID_BOOT_ROM_ADDR, SystemInformation->mainboard.BOOT_ROM.StartAddress);
        UISetValue(&ROMReportMenu, ROM_ID_BOOT_ROM_SIZE, SystemInformation->mainboard.BOOT_ROM.size / 1024 / 128);
        UISetVisible(&ROMReportMenu, ROM_ID_BOOT_ROM_ADDR, 1);
        UISetVisible(&ROMReportMenu, ROM_ID_BOOT_ROM_SIZE, 1);
        UISetVisible(&ROMReportMenu, ROM_ID_BOOT_ROM_MBIT_LBL, 1);
    }
    else
    {
        UISetVisible(&ROMReportMenu, ROM_ID_BOOT_ROM_ADDR, 0);
        UISetVisible(&ROMReportMenu, ROM_ID_BOOT_ROM_SIZE, 0);
        UISetVisible(&ROMReportMenu, ROM_ID_BOOT_ROM_MBIT_LBL, 0);
    }

    if (SystemInformation->mainboard.DVD_ROM.IsExists)
    {
        UISetValue(&ROMReportMenu, ROM_ID_DVD_ROM_ADDR, SystemInformation->mainboard.DVD_ROM.StartAddress);
        UISetValue(&ROMReportMenu, ROM_ID_DVD_ROM_SIZE, SystemInformation->mainboard.DVD_ROM.size / 1024 / 128);
        UISetVisible(&ROMReportMenu, ROM_ID_DVD_ROM_ADDR, 1);
        UISetVisible(&ROMReportMenu, ROM_ID_DVD_ROM_SIZE, 1);
        UISetVisible(&ROMReportMenu, ROM_ID_DVD_ROM_MBIT_LBL, 1);
    }
    else
    {
        UISetVisible(&ROMReportMenu, ROM_ID_DVD_ROM_ADDR, 0);
        UISetVisible(&ROMReportMenu, ROM_ID_DVD_ROM_SIZE, 0);
        UISetVisible(&ROMReportMenu, ROM_ID_DVD_ROM_MBIT_LBL, 0);
    }

    //Version information
    UISetString(&ROMReportMenu, ROM_ID_ROMVER, SystemInformation->mainboard.romver);
    UISetValue(&ROMReportMenu, ROM_ID_ROMGEN_MMDD, SystemInformation->mainboard.ROMGEN_MonthDate);
    UISetValue(&ROMReportMenu, ROM_ID_ROMGEN_YYYY, SystemInformation->mainboard.ROMGEN_Year);
    if (SystemInformation->DVDPlayerVer[0] != '\0')
    {
        UISetVisible(&ROMReportMenu, ROM_ID_DVDPLVER, 1);
        UISetString(&ROMReportMenu, ROM_ID_DVDPLVER, SystemInformation->DVDPlayerVer);
    }
    else
    {
        UISetVisible(&ROMReportMenu, ROM_ID_DVDPLVER, 0);
    }
    UISetString(&ROMReportMenu, ROM_ID_PS1DRVVER, SystemInformation->PS1DRVVer);
}

static void DEV9MenuDisplay(int DeviceConnected)
{
    UISetVisible(&DEV9ReportMenu, DEV9_ID_NO_DEVICE, !DeviceConnected);
    UISetVisible(&DEV9ReportMenu, DEV9_ID_SPEED_REV_LBL, DeviceConnected);
    UISetVisible(&DEV9ReportMenu, DEV9_ID_SPEED_REV, DeviceConnected);
    UISetVisible(&DEV9ReportMenu, DEV9_ID_SPEED_NAME, DeviceConnected);
    UISetVisible(&DEV9ReportMenu, DEV9_ID_SPEED_CAPS_LBL, DeviceConnected);
    UISetVisible(&DEV9ReportMenu, DEV9_ID_SPEED_CAPS, DeviceConnected);
    UISetVisible(&DEV9ReportMenu, DEV9_ID_SPEED_CAPS_SEP, DeviceConnected);
    UISetVisible(&DEV9ReportMenu, DEV9_ID_SPEED_REV8, DeviceConnected);
    UISetVisible(&DEV9ReportMenu, DEV9_ID_SPEED_CAPS_DESC, DeviceConnected);
    UISetVisible(&DEV9ReportMenu, DEV9_ID_MAC_ADDR_LBL, DeviceConnected);
    UISetVisible(&DEV9ReportMenu, DEV9_ID_MAC_ADDR_00, DeviceConnected);
    UISetVisible(&DEV9ReportMenu, DEV9_ID_MAC_ADDR_01, DeviceConnected);
    UISetVisible(&DEV9ReportMenu, DEV9_ID_MAC_ADDR_02, DeviceConnected);
    UISetVisible(&DEV9ReportMenu, DEV9_ID_MAC_ADDR_03, DeviceConnected);
    UISetVisible(&DEV9ReportMenu, DEV9_ID_MAC_ADDR_04, DeviceConnected);
    UISetVisible(&DEV9ReportMenu, DEV9_ID_MAC_ADDR_05, DeviceConnected);
    UISetVisible(&DEV9ReportMenu, DEV9_ID_MAC_ADDR_SEP_00, DeviceConnected);
    UISetVisible(&DEV9ReportMenu, DEV9_ID_MAC_ADDR_SEP_01, DeviceConnected);
    UISetVisible(&DEV9ReportMenu, DEV9_ID_MAC_ADDR_SEP_02, DeviceConnected);
    UISetVisible(&DEV9ReportMenu, DEV9_ID_MAC_ADDR_SEP_03, DeviceConnected);
    UISetVisible(&DEV9ReportMenu, DEV9_ID_MAC_ADDR_SEP_04, DeviceConnected);
    UISetVisible(&DEV9ReportMenu, DEV9_ID_PHY_VENDOR_LBL, DeviceConnected);
    UISetVisible(&DEV9ReportMenu, DEV9_ID_PHY_VENDOR_OUI, DeviceConnected);
    UISetVisible(&DEV9ReportMenu, DEV9_ID_PHY_VENDOR_NAME, DeviceConnected);
    UISetVisible(&DEV9ReportMenu, DEV9_ID_PHY_MODEL_LBL, DeviceConnected);
    UISetVisible(&DEV9ReportMenu, DEV9_ID_PHY_MODEL, DeviceConnected);
    UISetVisible(&DEV9ReportMenu, DEV9_ID_PHY_MODEL_DESC, DeviceConnected);
    UISetVisible(&DEV9ReportMenu, DEV9_ID_PHY_NAME, DeviceConnected);
    UISetVisible(&DEV9ReportMenu, DEV9_ID_PHY_REVISION_LBL, DeviceConnected);
    UISetVisible(&DEV9ReportMenu, DEV9_ID_PHY_REVISION, DeviceConnected);
}

static void LoadDEV9Information(const struct SystemInformation *SystemInformation)
{
    if (SystemInformation->mainboard.ssbus.status & PS2DB_SSBUS_HAS_SPEED)
    {
        DEV9MenuDisplay(1);
        UISetValue(&DEV9ReportMenu, DEV9_ID_SPEED_REV, SystemInformation->mainboard.ssbus.SPEED.rev1);
        UISetString(&DEV9ReportMenu, DEV9_ID_SPEED_NAME, GetSPEEDDesc(SystemInformation->mainboard.ssbus.SPEED.rev1));
        UISetValue(&DEV9ReportMenu, DEV9_ID_SPEED_CAPS, SystemInformation->mainboard.ssbus.SPEED.rev3);
        UISetValue(&DEV9ReportMenu, DEV9_ID_SPEED_REV8, SystemInformation->mainboard.ssbus.SPEED.rev8);
        UISetString(&DEV9ReportMenu, DEV9_ID_SPEED_CAPS_DESC, GetSPEEDCapsDesc(SystemInformation->mainboard.ssbus.SPEED.rev3));
        UISetValue(&DEV9ReportMenu, DEV9_ID_MAC_ADDR_00, SystemInformation->SMAP_MAC_address[0]);
        UISetValue(&DEV9ReportMenu, DEV9_ID_MAC_ADDR_01, SystemInformation->SMAP_MAC_address[1]);
        UISetValue(&DEV9ReportMenu, DEV9_ID_MAC_ADDR_02, SystemInformation->SMAP_MAC_address[2]);
        UISetValue(&DEV9ReportMenu, DEV9_ID_MAC_ADDR_03, SystemInformation->SMAP_MAC_address[3]);
        UISetValue(&DEV9ReportMenu, DEV9_ID_MAC_ADDR_04, SystemInformation->SMAP_MAC_address[4]);
        UISetValue(&DEV9ReportMenu, DEV9_ID_MAC_ADDR_05, SystemInformation->SMAP_MAC_address[5]);
        UISetValue(&DEV9ReportMenu, DEV9_ID_PHY_VENDOR_OUI, SystemInformation->mainboard.ssbus.SPEED.SMAP_PHY_OUI);
        UISetString(&DEV9ReportMenu, DEV9_ID_PHY_VENDOR_NAME, GetPHYVendDesc(SystemInformation->mainboard.ssbus.SPEED.SMAP_PHY_OUI));
        UISetValue(&DEV9ReportMenu, DEV9_ID_PHY_MODEL, SystemInformation->mainboard.ssbus.SPEED.SMAP_PHY_VMDL);
        UISetString(&DEV9ReportMenu, DEV9_ID_PHY_MODEL_DESC, GetPHYModelDesc(SystemInformation->mainboard.ssbus.SPEED.SMAP_PHY_OUI, SystemInformation->mainboard.ssbus.SPEED.SMAP_PHY_VMDL));
        UISetValue(&DEV9ReportMenu, DEV9_ID_PHY_REVISION, SystemInformation->mainboard.ssbus.SPEED.SMAP_PHY_REV);
    }
    else
    {
        DEV9MenuDisplay(0);
    }
}

static void LoadExtBusInformation(const struct SystemInformation *SystemInformation)
{
    UISetValue(&ExtBusReportMenu, EXTBUS_ID_USB_HC_REV_MAJOR, SystemInformation->mainboard.usb.HcRevision >> 4);
    UISetValue(&ExtBusReportMenu, EXTBUS_ID_USB_HC_REV_MINOR, SystemInformation->mainboard.usb.HcRevision & 0xF);
    UISetValue(&ExtBusReportMenu, EXTBUS_ID_ILINK_PORTS, SystemInformation->mainboard.iLink.NumPorts);
    UISetValue(&ExtBusReportMenu, EXTBUS_ID_ILINK_MAX_SPEED, SystemInformation->mainboard.iLink.MaxSpeed);
    UISetString(&ExtBusReportMenu, EXTBUS_ID_ILINK_MAX_SPEED_DESC, GetiLinkSpeedDesc(SystemInformation->mainboard.iLink.MaxSpeed));
    UISetValue(&ExtBusReportMenu, EXTBUS_ID_ILINK_COMPLIANCE, SystemInformation->mainboard.iLink.ComplianceLevel);
    UISetString(&ExtBusReportMenu, EXTBUS_ID_ILINK_COMPLIANCE_DESC, GetiLinkComplianceLvlDesc(SystemInformation->mainboard.iLink.ComplianceLevel));
    UISetValue(&ExtBusReportMenu, EXTBUS_ID_ILINK_VENDOR_ID, SystemInformation->mainboard.iLink.VendorID);
    UISetString(&ExtBusReportMenu, EXTBUS_ID_ILINK_VENDOR_DESC, GetiLinkVendorDesc(SystemInformation->mainboard.iLink.VendorID));
    UISetValue(&ExtBusReportMenu, EXTBUS_ID_ILINK_PRODUCT_ID, SystemInformation->mainboard.iLink.ProductID);
}

static void LoadSystemInformation(const struct SystemInformation *SystemInformation)
{
    LoadEEGSInformation(SystemInformation);
    LoadIOPSPU2Information(SystemInformation);
    LoadBoardInformation(SystemInformation);
    LoadBoard2Information(SystemInformation);
    LoadROMInformation(SystemInformation);
    LoadDEV9Information(SystemInformation);
    LoadExtBusInformation(SystemInformation);
}

static int MainMenuUpdateCallback(struct UIMenu *menu, unsigned short int frame, int selection, u32 padstatus)
{
    return ((padstatus & PAD_START) ? MAIN_MENU_BTN_DUMP : 0);
}

int MainMenu(const struct SystemInformation *SystemInformation)
{
    unsigned char done;
    short int option;
    struct UIMenu *CurrentMenu;

    LoadSystemInformation(SystemInformation);

    CurrentMenu = &EEGSReportMenu;
    option      = 0;
    done        = 0;
    while (!done)
    {
        option = UIExecMenu(CurrentMenu, option, &CurrentMenu, &MainMenuUpdateCallback);

        switch (option)
        {
            case MAIN_MENU_BTN_DUMP:
                DumpSystemROMScreen(SystemInformation);
                break;
            case 1: //User quit
                if (DisplayPromptMessage(SYS_UI_MSG_QUIT, SYS_UI_LBL_CANCEL, SYS_UI_LBL_OK) == 2)
                    done = 1;
                break;
        }
    }

    return 0;
}

#define PROGRESS_BAR_LENGTH 460
#define PROGRESS_BAR_HEIGHT 18

struct DumpRegion
{
    int IsInstalled;
    short int ProgressBarX, ProgressBarY;
    short int SizeLabelX, SizeLabelY;
    short int NameLabelX, NameLabelY;
    const char *label;
};

void RedrawDumpingScreen(const struct SystemInformation *SystemInformation, const struct DumpingStatus *DumpingStatus)
{
    unsigned int i;
    short int endX;
    char TextBuffer[32];
    GS_RGBAQ colour;
    static struct DumpRegion DumpRegions[DUMP_REGION_COUNT] = {
        {0,
         68, 164,
         400, 140,
         60, 140,
         NULL},
        {0,
         68, 208,
         400, 184,
         60, 184,
         NULL},
        {0,
         68, 256,
         400, 232,
         60, 232,
         NULL}};

    SyncFlipFB(&UIDrawGlobal);
    DrawBackground(&UIDrawGlobal, &BackgroundTexture);
    FontPrintf(&UIDrawGlobal, 10, 10, 2, 1.0f, GS_WHITE_FONT, GetUILabel(SYS_UI_LBL_DUMP_SCREEN));

    DumpRegions[DUMP_REGION_BOOT_ROM].label = GetUILabel(SYS_UI_LBL_BOOT_ROM);
    DumpRegions[DUMP_REGION_DVD_ROM].label  = GetUILabel(SYS_UI_LBL_DVD_ROM);
    DumpRegions[DUMP_REGION_EEPROM].label   = GetUILabel(SYS_UI_LBL_EEPROM);

    if (SystemInformation->mainboard.BOOT_ROM.IsExists)
    {
        sprintf(TextBuffer, "%u", SystemInformation->mainboard.BOOT_ROM.size);
        FontPrintf(&UIDrawGlobal, DumpRegions[DUMP_REGION_BOOT_ROM].SizeLabelX, DumpRegions[DUMP_REGION_BOOT_ROM].SizeLabelY, 2, 1.0f, GS_WHITE_FONT, TextBuffer);
        FontPrintf(&UIDrawGlobal, DumpRegions[DUMP_REGION_BOOT_ROM].SizeLabelX + 140, DumpRegions[DUMP_REGION_BOOT_ROM].SizeLabelY, 2, 1.0f, GS_WHITE_FONT, GetUILabel(SYS_UI_LBL_UNIT_BYTES));
        DumpRegions[DUMP_REGION_BOOT_ROM].IsInstalled = 1;
    }
    else
    {
        FontPrintf(&UIDrawGlobal, DumpRegions[DUMP_REGION_BOOT_ROM].SizeLabelX, DumpRegions[DUMP_REGION_BOOT_ROM].SizeLabelY, 2, 1.0f, GS_WHITE_FONT, GetUILabel(SYS_UI_LBL_NOT_DETECTED));
        DumpRegions[DUMP_REGION_BOOT_ROM].IsInstalled = 0;
    }

    if (SystemInformation->mainboard.DVD_ROM.IsExists)
    {
        sprintf(TextBuffer, "%u", SystemInformation->mainboard.DVD_ROM.size);
        FontPrintf(&UIDrawGlobal, DumpRegions[DUMP_REGION_DVD_ROM].SizeLabelX, DumpRegions[DUMP_REGION_DVD_ROM].SizeLabelY, 2, 1.0f, GS_WHITE_FONT, TextBuffer);
        FontPrintf(&UIDrawGlobal, DumpRegions[DUMP_REGION_DVD_ROM].SizeLabelX + 140, DumpRegions[DUMP_REGION_DVD_ROM].SizeLabelY, 2, 1.0f, GS_WHITE_FONT, GetUILabel(SYS_UI_LBL_UNIT_BYTES));
        DumpRegions[DUMP_REGION_DVD_ROM].IsInstalled = 1;
    }
    else
    {
        FontPrintf(&UIDrawGlobal, DumpRegions[DUMP_REGION_DVD_ROM].SizeLabelX, DumpRegions[DUMP_REGION_DVD_ROM].SizeLabelY, 2, 1.0f, GS_WHITE_FONT, GetUILabel(SYS_UI_LBL_NOT_DETECTED));
        DumpRegions[DUMP_REGION_DVD_ROM].IsInstalled = 0;
    }

    /* Do not forget about the MECHACON EEPROM. ;) */
    FontPrintf(&UIDrawGlobal, DumpRegions[DUMP_REGION_EEPROM].SizeLabelX, DumpRegions[DUMP_REGION_EEPROM].SizeLabelY, 2, 1.0f, GS_WHITE_FONT, "1024");
    FontPrintf(&UIDrawGlobal, DumpRegions[DUMP_REGION_EEPROM].SizeLabelX + 140, DumpRegions[DUMP_REGION_EEPROM].SizeLabelY, 2, 1.0f, GS_WHITE_FONT, GetUILabel(SYS_UI_LBL_UNIT_BYTES));
    DumpRegions[DUMP_REGION_EEPROM].IsInstalled = 1;

    for (i = 0; i < DUMP_REGION_COUNT; i++)
    {
        FontPrintf(&UIDrawGlobal, DumpRegions[i].NameLabelX, DumpRegions[i].NameLabelY, 2, 1.0f, GS_WHITE_FONT, DumpRegions[i].label);

        endX   = DumpRegions[i].ProgressBarX + PROGRESS_BAR_LENGTH * DumpingStatus[i].progress;
        colour = DumpingStatus[i].status < 0 ? GS_RED : (DumpingStatus[i].status == 0) ? GS_LGREY :
                                                                                         GS_GREEN;
        DrawSprite(&UIDrawGlobal, DumpRegions[i].ProgressBarX, DumpRegions[i].ProgressBarY, endX, DumpRegions[i].ProgressBarY + PROGRESS_BAR_HEIGHT, 3, colour);

        if (DumpRegions[i].IsInstalled)
            FontPrintf(&UIDrawGlobal, DumpRegions[i].ProgressBarX, DumpRegions[i].ProgressBarY, 2, 1.0f, GS_WHITE_FONT, DumpingStatus[i].status == 1 ? GetUILabel(SYS_UI_LBL_DONE) : (DumpingStatus[i].status < 0 ? GetUILabel(SYS_UI_LBL_FAILED) : GetUILabel(SYS_UI_LBL_DUMPING)));
    }
}

void RedrawLoadingScreen(unsigned int frame)
{
    short int xRel, x, y;
    int NumDots;
    GS_RGBAQ rgbaq;

    SyncFlipFB(&UIDrawGlobal);

    NumDots = frame % 240 / 60;

    FontPrintf(&UIDrawGlobal, 10, 10, 0, 2.5f, GS_WHITE_FONT, "PS2Ident");
    FontPrintf(&UIDrawGlobal, 16, 360, 0, 1.8f, GS_YELLOW_FONT, "" PS2IDENT_VERSION);

    x = 420;
    y = 360;
    FontPrintfWithFeedback(&UIDrawGlobal, x, y, 0, 1.8f, GS_WHITE_FONT, GetUILabel(SYS_UI_LBL_LOADING), &xRel, NULL);
    x += xRel;
    switch (NumDots)
    {
        case 1:
            FontPrintf(&UIDrawGlobal, x, y, 0, 1.8f, GS_WHITE_FONT, " .");
            break;
        case 2:
            FontPrintf(&UIDrawGlobal, x, y, 0, 1.8f, GS_WHITE_FONT, " . .");
            break;
        case 3:
            FontPrintf(&UIDrawGlobal, x, y, 0, 1.8f, GS_WHITE_FONT, " . . .");
            break;
    }

    if (frame < 60)
    { //Fade in
        rgbaq.r = 0;
        rgbaq.g = 0;
        rgbaq.b = 0;
        rgbaq.q = 0;
        rgbaq.a = 0x80 - (frame * 2);
        DrawSprite(&UIDrawGlobal, 0, 0,
                   UIDrawGlobal.width, UIDrawGlobal.height,
                   0, rgbaq);
    }
}

static int DumpSystemROM(const char *path, const struct SystemInformation *SystemInformation)
{
    char *filename;
    unsigned int PathLength, ModelNameLen, ROMVerLen;
    int result, PadStatus;
    FILE *logfile;
    struct DumpingStatus DumpingStatus[DUMP_REGION_COUNT];
    unsigned char done;

    memset(DumpingStatus, 0, sizeof(DumpingStatus));

    // Calculate the lengths of various parts of the filenames used below.
    PathLength   = strlen(path);
    ModelNameLen = strlen(SystemInformation->mainboard.ModelName);
    ROMVerLen    = strlen(SystemInformation->mainboard.romver);

    filename     = malloc(PathLength + ModelNameLen + 32);
    if (SystemInformation->mainboard.BOOT_ROM.IsExists)
    {
        DEBUG_PRINTF("Dumping Boot ROM at %p, %u bytes...", SystemInformation->mainboard.BOOT_ROM.StartAddress, SystemInformation->mainboard.BOOT_ROM.size);

#ifndef DSNET_HOST_SUPPORT
        sprintf(filename, "%s/%s_BOOT_ROM.bin", path, SystemInformation->mainboard.ModelName);
#else
        sprintf(filename, "%s%s_BOOT_ROM.bin", path, SystemInformation->mainboard.ModelName);
#endif
        if ((result = DumpRom(filename, SystemInformation, DumpingStatus, DUMP_REGION_BOOT_ROM)) == 0)
        {
            DEBUG_PRINTF("done!\n");
        }
        else
        {
            DEBUG_PRINTF("failed!\n");
        }
    }

    if (SystemInformation->mainboard.DVD_ROM.IsExists)
    {
        DEBUG_PRINTF("Dumping DVD ROM at %p, %u bytes...", SystemInformation->mainboard.DVD_ROM.StartAddress, SystemInformation->mainboard.DVD_ROM.size);

#ifndef DSNET_HOST_SUPPORT
        sprintf(filename, "%s/%s_DVD_ROM.bin", path, SystemInformation->mainboard.ModelName);
#else
        sprintf(filename, "%s%s_DVD_ROM.bin", path, SystemInformation->mainboard.ModelName);
#endif
        if ((result = DumpRom(filename, SystemInformation, DumpingStatus, DUMP_REGION_DVD_ROM)) == 0)
        {
            DEBUG_PRINTF("done!\n");
        }
        else
        {
            DEBUG_PRINTF("failed!\n");
        }
    }

#ifndef DSNET_HOST_SUPPORT
    sprintf(filename, "%s/%s_NVM.bin", path, SystemInformation->mainboard.ModelName);
#else
    sprintf(filename, "%s%s_NVM.bin", path, SystemInformation->mainboard.ModelName);
#endif
    if ((result = DumpMECHACON_EEPROM(filename)) == 0)
    {
        DumpingStatus[DUMP_REGION_EEPROM].progress = 1.00f;
        DumpingStatus[DUMP_REGION_EEPROM].status   = 1;
    }
    else
        DumpingStatus[DUMP_REGION_EEPROM].status = result;

    RedrawDumpingScreen(SystemInformation, DumpingStatus);

#ifndef DSNET_HOST_SUPPORT
    sprintf(filename, "%s/%s_specs.txt", path, SystemInformation->mainboard.ModelName);
#else
    sprintf(filename, "%s%s_specs.txt", path, SystemInformation->mainboard.ModelName);
#endif
    if ((logfile = fopen(filename, "wb")) != NULL)
    {
        WriteSystemInformation(logfile, SystemInformation);
        fclose(logfile);
    }

    //If the mainboard model is not recognized, write a new database record file to the disk.
    if (PS2IDBMS_LookupMainboardModel(&SystemInformation->mainboard) == NULL)
    {
#ifndef DSNET_HOST_SUPPORT
        sprintf(filename, "%s/%s_database.bin", path, SystemInformation->mainboard.ModelName);
#else
        sprintf(filename, "%s%s_database.bin", path, SystemInformation->mainboard.ModelName);
#endif
        WriteNewMainboardDBRecord(filename, &SystemInformation->mainboard);
    }

    free(filename);

    RedrawDumpingScreen(SystemInformation, DumpingStatus);

#ifdef DEBUG
    WriteSystemInformation(stdout, SystemInformation);
#endif

    done = 0;
    while (!done)
    {
        RedrawDumpingScreen(SystemInformation, DumpingStatus);
        // Draw the legend.
        DrawButtonLegend(&UIDrawGlobal, &PadLayoutTexture, CancelButton == PAD_CROSS ? BUTTON_TYPE_CROSS : BUTTON_TYPE_CIRCLE, 50, 404, 1);
        FontPrintf(&UIDrawGlobal, 75, 404, 1, 1.0f, GS_WHITE_FONT, GetUILabel(SYS_UI_LBL_RETURN_TO_MAIN));

        PadStatus = ReadCombinedPadStatus();

        if (PadStatus & CancelButton)
            done = 1;
    }

    return result;
}

#include <kernel.h>
#include <libcdvd.h>
#include <iopheap.h>
#include <iopcontrol.h>
#include <iopcontrol_special.h>
#include <errno.h>
#include <sifrpc.h>
#include <loadfile.h>
#include <libmc.h>
#include <stdio.h>
#include <string.h>
#include <usbhdfsd-common.h>
#include <limits.h>
#include <sys/stat.h>

#include <libgs.h>

#include "sysman/sysinfo.h"
#include "SYSMAN_rpc.h"

#include "main.h"
#include "system.h"
#include "pad.h"
#include "graphics.h"
#include "crc16.h"
#include "libcdvd_add.h"
#include "dvdplayer.h"
#include "OSDInit.h"
#include "ps1.h"
#include "modelname.h"

#include "UI.h"

#include "ident.h"

extern unsigned char SIO2MAN_irx[];
extern unsigned int size_SIO2MAN_irx;

extern unsigned char PADMAN_irx[];
extern unsigned int size_PADMAN_irx;

extern unsigned char MCMAN_irx[];
extern unsigned int size_MCMAN_irx;

extern unsigned char MCSERV_irx[];
extern unsigned int size_MCSERV_irx;

extern unsigned char POWEROFF_irx[];
extern unsigned int size_POWEROFF_irx;

extern unsigned char PS2DEV9_irx[];
extern unsigned int size_PS2DEV9_irx;

extern unsigned char USBD_irx[];
extern unsigned int size_USBD_irx;

extern unsigned char USBHDFSD_irx[];
extern unsigned int size_USBHDFSD_irx;

extern unsigned char USBHDFSDFSV_irx[];
extern unsigned int size_USBHDFSDFSV_irx;

extern unsigned char SYSMAN_irx[];
extern unsigned int size_SYSMAN_irx;

extern unsigned char IOPRP_img[];
extern unsigned int size_IOPRP_img;

extern void *_gp;
static unsigned char ConsoleRegionData[13];

static int LoadEROMDRV(void)
{
    char eromdrv[] = "rom1:EROMDRV?";
    int result;

    //Handle region-specific DVD Player of newer consoles.
    if (OSDGetDVDPlayerRegion(&eromdrv[12]) == 0 || eromdrv[12] != '\0')
    {
        eromdrv[12] = '\0'; //Replace '?' with a NULL.
    }

    return SifLoadModuleEncrypted(eromdrv, 0, NULL);
}

#define SYSTEM_INIT_THREAD_STACK_SIZE 0x800

struct SystemInitParams
{
    struct SystemInformation *SystemInformation;
    int InitCompleteSema;
};

static void SystemInitThread(struct SystemInitParams *SystemInitParams)
{
    int id, ret;
    GetRomName(SystemInitParams->SystemInformation->mainboard.romver);

    id = SifExecModuleBuffer(MCSERV_irx, size_MCSERV_irx, 0, NULL, &ret);
    DEBUG_PRINTF("MCSERV id:%d ret:%d\n", id, ret);
    id = SifExecModuleBuffer(PADMAN_irx, size_PADMAN_irx, 0, NULL, &ret);
    DEBUG_PRINTF("PADMAN id:%d ret:%d\n", id, ret);

    id = SifExecModuleBuffer(POWEROFF_irx, size_POWEROFF_irx, 0, NULL, &ret);
    DEBUG_PRINTF("POWEROFF id:%d ret:%d\n", id, ret);
    id = SifExecModuleBuffer(PS2DEV9_irx, size_PS2DEV9_irx, 0, NULL, &ret);
    DEBUG_PRINTF("DEV9 id:%d ret:%d\n", id, ret);

    SifLoadModule("rom0:ADDDRV", 0, NULL);
    SifLoadModule("rom0:ADDROM2", 0, NULL);

    //Initialize PlayStation Driver (PS1DRV)
    PS1DRVInit();

    //Initialize ROM DVD player.
    //It is normal for this to fail on consoles that have no DVD ROM chip (i.e. DEX or the SCPH-10000/SCPH-15000).
    DVDPlayerInit();

    LoadEROMDRV();

    /* Must be loaded last, after all devices have been initialized. */
    id = SifExecModuleBuffer(SYSMAN_irx, size_SYSMAN_irx, 0, NULL, &ret);
    DEBUG_PRINTF("SYSMAN id:%d ret:%d\n", id, ret);

    SysmanInit();

    GetPeripheralInformation(SystemInitParams->SystemInformation);

    SignalSema(SystemInitParams->InitCompleteSema);
    ExitDeleteThread();
}

int VBlankStartSema;

static int VBlankStartHandler(int cause)
{
    ee_sema_t sema;
    iReferSemaStatus(VBlankStartSema, &sema);
    if (sema.count < sema.max_count)
        iSignalSema(VBlankStartSema);

    return 0;
}

extern int UsbReadyStatus;

static void usb_callback(void *packet, void *common)
{
    UsbReadyStatus = (((SifCmdHeader_t *)packet)->opt == USBMASS_DEV_EV_CONN) ? 1 : 0;
}

int main(int argc, char *argv[])
{
    int id, ret;
    static SifCmdHandlerData_t SifCmdbuffer;
    static struct SystemInformation SystemInformation;
    void *SysInitThreadStack;
    ee_sema_t ThreadSema;
    int SystemInitSema;
    unsigned int FrameNum;
    struct SystemInitParams InitThreadParams;

    //	chdir("mass:/PS2Ident/");
    if (argc < 1 || GetBootDeviceID() == BOOT_DEVICE_UNKNOWN)
    {
        Exit(-1);
    }

    SifInitRpc(0);
    while (!SifIopRebootBuffer(IOPRP_img, size_IOPRP_img)) {};

    memset(&SystemInformation, 0, sizeof(SystemInformation));

    /* Go gather some information from the EE's peripherals while the IOP reset. */
    GetEEInformation(&SystemInformation);

    InitCRC16LookupTable();

    ThreadSema.init_count = 0;
    ThreadSema.max_count  = 1;
    ThreadSema.attr = ThreadSema.option = 0;
    InitThreadParams.InitCompleteSema = SystemInitSema = CreateSema(&ThreadSema);
    InitThreadParams.SystemInformation                 = &SystemInformation;

    SysInitThreadStack                                 = memalign(64, SYSTEM_INIT_THREAD_STACK_SIZE);

    ThreadSema.init_count                              = 0;
    ThreadSema.max_count                               = 1;
    ThreadSema.attr = ThreadSema.option = 0;
    VBlankStartSema                     = CreateSema(&ThreadSema);

    AddIntcHandler(kINTC_VBLANK_START, &VBlankStartHandler, 0);
    EnableIntc(kINTC_VBLANK_START);

    while (!SifIopSync()) {};

    SifInitRpc(0);
    SifInitIopHeap();
    SifLoadFileInit();
    fioInit();

    sbv_patch_enable_lmb();
    sbv_patch_fileio();

    id = SifExecModuleBuffer(SIO2MAN_irx, size_SIO2MAN_irx, 0, NULL, &ret);
    DEBUG_PRINTF("SIO2MAN id:%d ret:%d\n", id, ret);
    id = SifExecModuleBuffer(MCMAN_irx, size_MCMAN_irx, 0, NULL, &ret);
    DEBUG_PRINTF("MCMAN id:%d ret:%d\n", id, ret);

    SifSetCmdBuffer(&SifCmdbuffer, 1);
    SifAddCmdHandler(0, &usb_callback, NULL);

    id = SifExecModuleBuffer(USBD_irx, size_USBD_irx, 0, NULL, &ret);
    DEBUG_PRINTF("USBD id:%d ret:%d\n", id, ret);
    id = SifExecModuleBuffer(USBHDFSD_irx, size_USBHDFSD_irx, 0, NULL, &ret);
    DEBUG_PRINTF("USBHDFSD id:%d ret:%d\n", id, ret);
    id = SifExecModuleBuffer(USBHDFSDFSV_irx, size_USBHDFSDFSV_irx, 0, NULL, &ret);
    DEBUG_PRINTF("USBHDFSDFSV id:%d ret:%d\n", id, ret);
    
    id = SifLoadStartModule("rom0:CDVDFSV", 0, NULL, &ret);
    DEBUG_PRINTF("rom0:CDVDFSV id:%d ret:%d\n", id, ret);

    sceCdInit(SCECdINoD);
    cdInitAdd();

    //Initialize system paths.
    OSDInitSystemPaths();

    //Initialize ROM version (must be done first).
    OSDInitROMVER();

    if (InitializeUI(0) != 0)
    {
        SifExitRpc();
        Exit(-1);
    }

    DEBUG_PRINTF("Loading database.\n");

    PS2IDBMS_LoadDatabase("PS2Ident.db");

    DEBUG_PRINTF("Initializing hardware...");

    SysCreateThread(&SystemInitThread, SysInitThreadStack, SYSTEM_INIT_THREAD_STACK_SIZE, &InitThreadParams, 0x2);

    FrameNum = 0;
    while (PollSema(SystemInitSema) != SystemInitSema)
    {
        RedrawLoadingScreen(FrameNum);
        FrameNum++;
    }
    DeleteSema(SystemInitSema);
    free(SysInitThreadStack);

    SifLoadFileExit();
    SifExitIopHeap();

    DEBUG_PRINTF("System init: Initializing RPCs.\n");

    PadInitPads();
    mcInit(MC_TYPE_XMC);

    DEBUG_PRINTF("done!\nEntering main menu.\n");

    MainMenu(&SystemInformation);

    PadDeinitPads();

    DisableIntc(kINTC_VBLANK_START);
    RemoveIntcHandler(kINTC_VBLANK_START, 0);
    DeleteSema(VBlankStartSema);
    SifRemoveCmdHandler(0);

    DeinitializeUI();

    PS2IDBMS_UnloadDatabase();

    sceCdInit(SCECdEXIT);
    fioExit();
    SysmanDeinit();
    SifExitRpc();

    return 0;
}

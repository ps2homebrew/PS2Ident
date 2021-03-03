#Enable to build support for the TOOL's host interface
DSNET_HOST_SUPPORT = 0
#disable for PCSX2 support
DEBUG = 0
DISABLE_ILINK_DUMPING = 0

EE_BIN = PS2Ident_np.elf
EE_PACKED_BIN = PS2Ident.elf

#IOP modules
EE_IOP_OBJS = SIO2MAN_irx.o MCMAN_irx.o MCSERV_irx.o PADMAN_irx.o POWEROFF_irx.o PS2DEV9_irx.o USBD_irx.o USBHDFSD_irx.o USBHDFSDFSV_irx.o SYSMAN_irx.o IOPRP_img.o

EE_GRAPHICS_OBJS = buttons.o devices.o background_img.o
EE_OBJS = main.o system.o UI.o menu.o ident.o dbms.o SYSMAN_rpc.o graphics.o font.o pad.o DeviceSupport.o crc16.o libcdvd_add.o OSDInit.o modelname.o dvdplayer.o ps1.o $(EE_IOP_OBJS) $(EE_GRAPHICS_OBJS)

EE_INCS := -I$(PS2SDK)/ee/include -I$(PS2SDK)/ports/include/freetype2 -I$(PS2SDK)/common/include -I. -I$(PS2SDK)/ports/include
EE_LDFLAGS :=  -L$(PS2SDK)/ee/lib -L$(PS2SDK)/ports/lib -s
EE_LIBS := -lgs -lpng -lz -lcdvd -lmc -lpadx -lpatches -liopreboot -lfreetype -lm
EE_GPVAL = -G383
EE_CFLAGS += -D_EE -O2 -mgpopt $(EE_GPVAL)

EE_TEMP_FILES = SIO2MAN_irx.c MCMAN_irx.c MCSERV_irx.c PADMAN_irx.c POWEROFF_irx.c PS2DEV9_irx.c USBD_irx.c USBHDFSD_irx.c USBHDFSDFSV_irx.c SYSMAN_irx.c buttons.c devices.c background_img.c IOPRP_img.c

ifeq ($(DSNET_HOST_SUPPORT),1)
	EE_CFLAGS += -DDSNET_HOST_SUPPORT=1
endif

ifeq ($(DEBUG),1)
	IOP_CFLAGS += -DDEBUG=1
endif

ifeq ($(DISABLE_ILINK_DUMPING),1)
	IOP_CFLAGS += -DDISABLE_ILINK_DUMPING
endif

%.o : %.c
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

%.o : %.S
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

%.o : %.s
	$(EE_AS) $(EE_ASFLAGS) $< -o $@

$(EE_BIN) : $(EE_OBJS)
	$(EE_CC) $(EE_CFLAGS) $(EE_LDFLAGS) -o $(EE_BIN) $(EE_OBJS) $(EE_LIBS)
	ps2-packer $(EE_BIN) $(EE_PACKED_BIN)

all:
	$(MAKE) $(EE_BIN)

clean:
	make clean -C sysman
	make clean -C usbhdfsdfsv
	rm -f $(EE_BIN) $(EE_PACKED_BIN) $(EE_BIN_REL) $(EE_OBJS) $(EE_TEMP_FILES)

SIO2MAN_irx.c:
	bin2c $(PS2SDK)/iop/irx/freesio2.irx SIO2MAN_irx.c SIO2MAN_irx

MCMAN_irx.c:
	bin2c $(PS2SDK)/iop/irx/mcman.irx MCMAN_irx.c MCMAN_irx

MCSERV_irx.c:
	bin2c $(PS2SDK)/iop/irx/mcserv.irx MCSERV_irx.c MCSERV_irx

PADMAN_irx.c:
	bin2c $(PS2SDK)/iop/irx/freepad.irx PADMAN_irx.c PADMAN_irx

POWEROFF_irx.c:
	bin2c $(PS2SDK)/iop/irx/poweroff.irx POWEROFF_irx.c POWEROFF_irx

PS2DEV9_irx.c:
	bin2c $(PS2SDK)/iop/irx/ps2dev9.irx PS2DEV9_irx.c PS2DEV9_irx

USBD_irx.c:
	bin2c $(PS2SDK)/iop/irx/usbd.irx USBD_irx.c USBD_irx

USBHDFSD_irx.c:
	bin2c $(PS2SDK)/iop/irx/usbhdfsd.irx USBHDFSD_irx.c USBHDFSD_irx

USBHDFSDFSV_irx.c:
	$(MAKE) -C usbhdfsdfsv
	# bin2c irx/usbhdfsdfsv.irx USBHDFSDFSV_irx.c USBHDFSDFSV_irx
	bin2c usbhdfsdfsv/usbhdfsdfsv.irx USBHDFSDFSV_irx.c USBHDFSDFSV_irx

SYSMAN_irx.c:
	$(MAKE) -C sysman
	# $(BIN2C) irx/sysman.irx SYSMAN_irx.c SYSMAN_irx
	bin2c sysman/sysman.irx SYSMAN_irx.c SYSMAN_irx

background_img.c:
	bin2c resources/background.png background_img.c background

buttons.c:
	bin2c resources/buttons.png buttons.c buttons

devices.c:
	bin2c resources/devices.png devices.c devices

IOPRP_img.c:
	bin2c irx/ioprp.img IOPRP_img.c IOPRP_img

include $(PS2SDK)/samples/Makefile.pref

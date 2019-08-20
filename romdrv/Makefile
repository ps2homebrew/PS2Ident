#Enable to build PS2Ident customizations for ROMDRV
ROMDRV_EXPORT_DEVICES = 1

IOP_BIN  = romdrv.irx
IOP_OBJS = romdrv.o imports.o exports.o

IOP_CFLAGS += -Os

ifeq ($(ROMDRV_EXPORT_DEVICES),1)
	IOP_CFLAGS += -DROMDRV_EXPORT_DEVICES=1
endif

include $(PS2SDK)/Defs.make
include Rules.bin.make
include Rules.make

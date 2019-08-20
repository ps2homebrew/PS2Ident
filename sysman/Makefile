IOP_BIN  = sysman.irx
IOP_OBJS = main.o iLink.o dev9.o usb.o spu2.o rom.o imports.o

IOP_CFLAGS += -Wall -fno-builtin -G8192 -mgpopt
IOP_LDFLAGS += -s
IOP_INCS += -I$(PS2SDK)/iop/include -I$(PS2SDK)/common/include -I../romdrv/include

all: $(IOP_BIN)

clean:
	rm -f $(IOP_BIN) $(IOP_OBJS)

include $(PS2SDK)/Defs.make
include Rules.make

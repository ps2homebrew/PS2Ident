enum BootDeviceIDs{
	BOOT_DEVICE_UNKNOWN = -1,
	BOOT_DEVICE_MC0 = 0,
	BOOT_DEVICE_MC1,
	BOOT_DEVICE_CDROM,
	BOOT_DEVICE_MASS,
	BOOT_DEVICE_HOST,

	BOOT_DEVICE_COUNT,
};

int GetBootDeviceID(void);
int RemoveFolder(const char *path);
int SysCreateThread(void *function, void *stack, unsigned int StackSize, void *arg, int priority);

int GetHasDeviceSufficientSpace(const char *device, int unit, const struct RequiredFileSpaceStat *RequiredSpaceStats, unsigned int NumFileEntries);
int HasMcUnitSufficientSpace(int unit, const struct RequiredFileSpaceStat *RequiredSpaceStats, unsigned int NumFileEntries);
int HasMassUnitSufficientSpace(int unit, const struct RequiredFileSpaceStat *RequiredSpaceStats, unsigned int NumFileEntries);
int HasHostUnitSufficientSpace(int unit, const struct RequiredFileSpaceStat *RequiredSpaceStats, unsigned int NumFileEntries);

int StartDevicePollingThread(void);
int StopDevicePollingThread(void);
int GetIsDeviceUnitReady(const char *device, int unit);

// C-Friendly Prototypes
void LaunchUserAppThread(uint32_t *appaddr, uint32_t *runtimep);
void LaunchUserApp(uint32_t *appaddr, uint32_t *runtimep);
void LaunchUserAppNoSP(uint32_t *appaddr, uint32_t *runtimep);
void LaunchUserAppUpdateNVIC(unsigned long *, unsigned long *runtime_p);


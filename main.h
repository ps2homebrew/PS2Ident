//#define DEBUG	1
#ifdef DEBUG
#ifdef EE_SIO
#include <sio.h>
#define DEBUG_PRINTF(args...) sio_printf(args)
#else
#define DEBUG_PRINTF(args...) printf(args)
#endif

#else
#define DEBUG_PRINTF(args...)
#endif

#ifdef COH
#define PS2IDENT_VERSION "0.850 - COH"
#else
#define PS2IDENT_VERSION "0.850"
#endif


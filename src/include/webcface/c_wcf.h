#pragma once
#include "common/def.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void* wcfClient;

WEBCFACE_DLL wcfClient wcfInit(const char *name,
                           const char *host = "127.0.0.1",
                           int port = WEBCFACE_DEFAULT_PORT);
WEBCFACE_DLL int wcfIsInstance(wcfClient wcli);
WEBCFACE_DLL int wcfClose(wcfClient wcli);
WEBCFACE_DLL int wcfStart(wcfClient wcli);
WEBCFACE_DLL int wcfSync(wcfClient wcli);

WEBCFACE_DLL int wcfValueSet(wcfClient wcli, const char *field, double value);
WEBCFACE_DLL int wcfValueSetVecD(wcfClient wcli, const char *field, double* value, int size);
// WEBCFACE_DLL int wcfValueSetVecI(void *wcli, const char *field, int* value, int size);



#ifdef __cplusplus
}
#endif

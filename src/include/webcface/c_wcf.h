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
WEBCFACE_DLL int wcfValueSetVecD(wcfClient wcli, const char *field, const double *values, int size);
// WEBCFACE_DLL int wcfValueSetVecI(void *wcli, const char *field, const int *values, int size);

typedef struct wcfMultiVal{
    int as_int;
    double as_double;
    const char *as_str;
} wcfMultiVal;

WEBCFACE_DLL const wcfMultiVal *wcfFuncRun(wcfClient wcli,
                                     const char *member,
                                     const char *field,
                                     wcfMultiVal *args,
                                     int arg_size);

WEBCFACE_DLL const char *wcfFuncRunS(wcfClient wcli,
                                     const char *member,
                                     const char *field,
                                     const char **args,
                                     int arg_size);

#ifdef __cplusplus
}
#endif

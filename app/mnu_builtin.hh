#ifndef __CLI_BUILT_IN_H__
#define __CLI_BUILT_IN_H__

#include "stackmenu.h"

#ifdef __cplusplus
#include <clientkit/nugu_client.hh>

extern "C" {
int builtin_init(NuguClientKit::NuguClient::CapabilityBuilder* builder);
void builtin_deinit(void);
StackmenuItem* builtin_get_stackmenu(void);
}

#else

StackmenuItem* builtin_get_stackmenu(void);

#endif

#endif
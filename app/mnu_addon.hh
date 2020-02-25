#ifndef __CLI_ADD_ON_H__
#define __CLI_ADD_ON_H__

#include "stackmenu.h"

#ifdef __cplusplus
#include <clientkit/nugu_client.hh>

extern "C" {
int addon_init(NuguClientKit::NuguClient::CapabilityBuilder* builder);
void addon_deinit(void);
StackmenuItem* addon_get_stackmenu(void);
}

#else
StackmenuItem* addon_get_stackmenu(void);

#endif

#endif
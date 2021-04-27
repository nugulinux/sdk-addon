#ifndef __CLI_MNU_ALARM_H__
#define __CLI_MNU_ALARM_H__

#include "stackmenu.h"

#ifdef __cplusplus

extern "C" {
StackmenuItem* alarm_get_stackmenu(void);
}

#else
StackmenuItem* alarm_get_stackmenu(void);

#endif

#endif

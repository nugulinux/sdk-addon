#ifndef __CLI_SDK_CONTROL_H__
#define __CLI_SDK_CONTROL_H__

#ifdef __cplusplus
extern "C" {
#endif

int sdk_init(void);
int sdk_deinit(void);
int sdk_is_initialized(void);

int sdk_connect(void);
int sdk_disconnect(void);
int sdk_is_connected(void);

#ifdef __cplusplus
}
#endif

#endif
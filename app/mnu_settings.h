#ifndef __CLI_SETTINGS_H__
#define __CLI_SETTINGS_H__

#include "stackmenu.h"

#ifdef __cplusplus
extern "C" {
#endif

#define AGENT_NAME_SYSTEM "System"
#define AGENT_NAME_ASR "ASR"
#define AGENT_NAME_TTS "TTS"
#define AGENT_NAME_TEXT "Text"
#define AGENT_NAME_AUDIOPLAYER "AudioPlayer"
#define AGENT_NAME_BATTERY "Battery"
#define AGENT_NAME_LOCATION "Location"
#define AGENT_NAME_DELEGATION "Delegation"
#define AGENT_NAME_EXTENSION "Extension"

void settings_init(void);

StackmenuItem *settings_get_stackmenu(void);

const char *settings_get_asr_model_path(void);

int settings_is_agent_enabled(const char *name);
char *settings_get_agent_list(void);

int settings_set_token(const char *token);
const char *settings_get_token(void);

int settings_is_log_enabled(void);

#ifdef __cplusplus
}
#endif

#endif
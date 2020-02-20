#ifndef __CLI_SETTINGS_H__
#define __CLI_SETTINGS_H__

#include "stackmenu.h"

#ifdef __cplusplus
extern "C" {
#endif

#define AGENT_NAME_SYSTEM "SystemAgent"
#define AGENT_NAME_ASR "ASRAgent"
#define AGENT_NAME_TTS "TTSAgent"
#define AGENT_NAME_TEXT "TextAgent"
#define AGENT_NAME_AUDIOPLAYER "AudioPlayerAgent"
#define AGENT_NAME_BATTERY "BatteryAgent"

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
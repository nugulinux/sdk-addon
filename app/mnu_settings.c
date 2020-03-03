#include <string.h>
#include <stdlib.h>

#include <base/nugu_log.h>

#include "mnu_settings.h"

static char data_token[MENU_DATA_SIZE];
static char data_log[MENU_DATA_SIZE] = "";
static char data_log_module[MENU_DATA_SIZE] = "";

static char data_system_agent[MENU_DATA_SIZE] = "1";
static char data_asr_agent[MENU_DATA_SIZE] = "1";
static char data_tts_agent[MENU_DATA_SIZE] = "1";
static char data_text_agent[MENU_DATA_SIZE] = "1";
static char data_audioplayer_agent[MENU_DATA_SIZE] = "1";

static char data_battery_agent[MENU_DATA_SIZE] = "1";
static char data_location_agent[MENU_DATA_SIZE] = "1";
static char data_delegation_agent[MENU_DATA_SIZE] = "1";
static char data_extension_agent[MENU_DATA_SIZE] = "1";

static char data_model_path[MENU_DATA_SIZE] = "/var/lib/nugu/model";

static int run_log_system_update(Stackmenu *mm, StackmenuItem *menu,
				 void *user_data)
{
	if (g_strcmp0(data_log, "stderr") == 0)
		nugu_log_set_system(NUGU_LOG_SYSTEM_STDERR);
	else if (g_strcmp0(data_log, "none") == 0)
		nugu_log_set_system(NUGU_LOG_SYSTEM_NONE);
	else if (g_strcmp0(data_log, "syslog") == 0)
		nugu_log_set_system(NUGU_LOG_SYSTEM_SYSLOG);
	else {
		memcpy(data_log, "stderr", 6);
		data_log[6] = '\0';

		nugu_log_set_system(NUGU_LOG_SYSTEM_STDERR);
		return -1;
	}

	return 0;
}

static int run_log_module_update(Stackmenu *mm, StackmenuItem *menu,
				 void *user_data)
{
	gchar **modules = NULL;
	gint count = 0;
	gint i;
	unsigned int bitset = 0;

	modules = g_strsplit(data_log_module, ",", -1);
	if (!modules)
		return -1;

	count = g_strv_length(modules);
	for (i = 0; i < count; i++) {
		if (!strncasecmp(modules[i], "all", 4)) {
			bitset = NUGU_LOG_MODULE_ALL;
			break;
		}

		if (!strncasecmp(modules[i], "default", 8))
			bitset = bitset | NUGU_LOG_MODULE_DEFAULT;
		if (!strncasecmp(modules[i], "network", 8))
			bitset = bitset | NUGU_LOG_MODULE_NETWORK;
		if (!strncasecmp(modules[i], "network_trace", 14))
			bitset = bitset | NUGU_LOG_MODULE_NETWORK_TRACE;
		if (!strncasecmp(modules[i], "protocol", 9))
			bitset = bitset | NUGU_LOG_MODULE_PROTOCOL;
		if (!strncasecmp(modules[i], "audio", 6))
			bitset = bitset | NUGU_LOG_MODULE_AUDIO;
	}

	g_strfreev(modules);

	nugu_log_set_modules(bitset);

	return 0;
}

static StackmenuItem menu_settings[] = {
	{ "*", " System settings" },
	{ "tok", "Token", NULL, NULL, data_token },
	{ "log", "SDK Log system", NULL, run_log_system_update, data_log },
	{ "_", "(none,stderr,syslog)" },
	{ "lm", "SDK Log modules", NULL, run_log_module_update,
	  data_log_module },
	{ "_", "(default,network,network_trace,protocol,audio,all)" },
	{ "-" },
	{ "*", " Enable / Disable capability agents - Built-in" },
	{ "1", AGENT_NAME_SYSTEM, NULL, NULL, data_system_agent },
	{ "2", AGENT_NAME_ASR, NULL, NULL, data_asr_agent },
	{ "mod", " - Set model file path", NULL, NULL, data_model_path },
	{ "3", AGENT_NAME_TTS, NULL, NULL, data_tts_agent },
	{ "4", AGENT_NAME_TEXT, NULL, NULL, data_text_agent },
	{ "5", AGENT_NAME_AUDIOPLAYER, NULL, NULL, data_audioplayer_agent },
	{ "-" },
	{ "*", " Enable / Disable capability agents - Add-on" },
	{ "6", AGENT_NAME_BATTERY, NULL, NULL, data_battery_agent },
	{ "7", AGENT_NAME_LOCATION, NULL, NULL, data_location_agent },
	{ "8", AGENT_NAME_DELEGATION, NULL, NULL, data_delegation_agent },
	{ "9", AGENT_NAME_EXTENSION, NULL, NULL, data_extension_agent },
	NULL
};

StackmenuItem *settings_get_stackmenu(void)
{
	return menu_settings;
}

const char *settings_get_asr_model_path(void)
{
	return data_model_path;
}

int settings_is_agent_enabled(const char *name)
{
	StackmenuItem *item;

	item = stackmenu_item_find_by_title(menu_settings, name);
	if (!item)
		return 0;

	if (item->data == NULL)
		return 0;

	if (item->data[0] != '1')
		return 0;

	return 1;
}

char *settings_get_agent_list(void)
{
	int i;
	int len = stackmenu_item_count(menu_settings);
	char buf[4096];
	char *pos;

	pos = buf;
	for (i = 0; i < len; i++) {
		if (menu_settings[i].data == NULL)
			continue;

		if (menu_settings[i].data[0] != '1')
			continue;

		memcpy(pos, menu_settings[i].title,
		       strlen(menu_settings[i].title));
		pos += strlen(menu_settings[i].title);
		*pos = ',';
		pos++;
	}

	if (pos == buf)
		return NULL;

	if (*(pos - 1) == ',')
		pos--;
	*pos = '\0';

	return strdup(buf);
}

int settings_set_token(const char *token)
{
	size_t len;

	if (!token)
		return -1;

	len = strlen(token);
	if (len == 0)
		return -1;

	if (len >= sizeof(data_token))
		len = sizeof(data_token) - 1;

	memcpy(data_token, token, len);
	data_token[len] = '\0';

	return 0;
}

const char *settings_get_token(void)
{
	if (data_token[0] == '\0')
		return NULL;

	return data_token;
}

int settings_is_log_enabled(void)
{
	if (nugu_log_get_system() == NUGU_LOG_SYSTEM_NONE)
		return 0;

	return 1;
}

void settings_init(void)
{
	unsigned int bitset;
	char *pos;

	settings_set_token(getenv("NUGU_TOKEN"));

	switch (nugu_log_get_system()) {
	case NUGU_LOG_SYSTEM_NONE:
		memcpy(data_log, "none", 4);
		data_log[4] = '\0';
		break;
	case NUGU_LOG_SYSTEM_STDERR:
		memcpy(data_log, "stderr", 6);
		data_log[6] = '\0';
		break;
	case NUGU_LOG_SYSTEM_SYSLOG:
		memcpy(data_log, "syslog", 6);
		data_log[6] = '\0';
		break;
	default:
		break;
	}

	bitset = nugu_log_get_modules();

	if (bitset == NUGU_LOG_MODULE_ALL) {
		memcpy(data_log_module, "all", 3);
		data_log_module[3] = '\0';
		return;
	}

	pos = data_log_module;
	if (bitset & NUGU_LOG_MODULE_DEFAULT) {
		memcpy(pos, "default", 7);
		pos += 7;

		*pos = ',';
		pos++;
	}

	if (bitset & NUGU_LOG_MODULE_NETWORK) {
		memcpy(pos, "network", 7);
		pos += 7;

		*pos = ',';
		pos++;
	}

	if (bitset & NUGU_LOG_MODULE_NETWORK_TRACE) {
		memcpy(pos, "network_trace", 13);
		pos += 13;

		*pos = ',';
		pos++;
	}

	if (bitset & NUGU_LOG_MODULE_PROTOCOL) {
		memcpy(pos, "protocol", 8);
		pos += 8;

		*pos = ',';
		pos++;
	}

	if (bitset & NUGU_LOG_MODULE_AUDIO) {
		memcpy(pos, "audio", 5);
		pos += 5;
	}

	if (pos == data_log_module)
		return;

	if (*(pos - 1) == ',')
		pos--;
	*pos = '\0';
}

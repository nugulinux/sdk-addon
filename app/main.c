#include <stdio.h>
#include <stdlib.h>

#include <glib.h>
#include <gst/gst.h>
#include <gst/pbutils/pbutils.h>
#include <base/nugu_log.h>

#include "stackmenu.h"
#include "mnu_settings.h"
#include "mnu_builtin.hh"
#include "mnu_addon.hh"
#include "mnu_alarm.hh"
#include "sdk_control.h"

static int run_status(Stackmenu *mm, StackmenuItem *menu, void *user_data)
{
	char *list;

	printf("  - Token: ");
	if (settings_get_token())
		printf("%.*s...\n", 45, settings_get_token());
	else
		printf(NUGU_ANSI_COLOR_RED "empty\n" NUGU_ANSI_COLOR_NORMAL);

	printf("  - Agent: ");
	list = settings_get_agent_list();
	if (list) {
		printf(NUGU_ANSI_COLOR_LIGHTBLUE "%s\n" NUGU_ANSI_COLOR_NORMAL,
		       list);
		free(list);
	} else {
		printf(NUGU_ANSI_COLOR_RED "empty\n" NUGU_ANSI_COLOR_NORMAL);
	}

	printf("  - SDK: ");
	if (sdk_is_initialized())
		printf(NUGU_ANSI_COLOR_LIGHTBLUE
		       "initialized\n" NUGU_ANSI_COLOR_NORMAL);
	else
		printf(NUGU_ANSI_COLOR_RED
		       "not ready\n" NUGU_ANSI_COLOR_NORMAL);

	printf("  - SDK Connection: ");
	if (sdk_is_connected())
		printf(NUGU_ANSI_COLOR_LIGHTBLUE
		       "connected\n" NUGU_ANSI_COLOR_NORMAL);
	else
		printf(NUGU_ANSI_COLOR_RED
		       "disconnected\n" NUGU_ANSI_COLOR_NORMAL);

	printf("  - SDK log: ");
	if (settings_is_log_enabled())
		printf(NUGU_ANSI_COLOR_LIGHTBLUE "on\n" NUGU_ANSI_COLOR_NORMAL);
	else
		printf(NUGU_ANSI_COLOR_RED "off\n" NUGU_ANSI_COLOR_NORMAL);

	return 0;
}

static int run_sdk_init(Stackmenu *mm, StackmenuItem *menu, void *user_data)
{
	return sdk_init();
}

static int run_sdk_deinit(Stackmenu *mm, StackmenuItem *menu, void *user_data)
{
	return sdk_deinit();
}

static int run_sdk_connect(Stackmenu *mm, StackmenuItem *menu, void *user_data)
{
	return sdk_connect();
}

static int run_sdk_disconnect(Stackmenu *mm, StackmenuItem *menu,
			      void *user_data)
{
	return sdk_disconnect();
}

static StackmenuItem menu_main[] = {
	{ "*", " Settings", NULL, run_status },
	{ "-" },
	{ "set", "Change settings" },
	{ "-" },
	{ "1", "SDK initialize", NULL, run_sdk_init },
	{ "2", "SDK de-initialize", NULL, run_sdk_deinit },
	{ "3", "SDK network connect", NULL, run_sdk_connect },
	{ "4", "SDK network disconnect", NULL, run_sdk_disconnect },
	{ "-" },
	{ "b", "Control built-in capability agents", NULL },
	{ "a", "Control add-on capability agents", NULL },
	{ "-" },
	{ "c", "Alerts agents test", NULL },
	NULL
};

int main(int argc, char *argv[])
{
	GMainLoop *loop;
	Stackmenu *mm;
	StackmenuItem *item;

	settings_init();

	item = stackmenu_item_find(menu_main, "set");
	if (item)
		item->sub_menu = settings_get_stackmenu();

	item = stackmenu_item_find(menu_main, "b");
	if (item)
		item->sub_menu = builtin_get_stackmenu();

	item = stackmenu_item_find(menu_main, "a");
	if (item)
		item->sub_menu = addon_get_stackmenu();

	item = stackmenu_item_find(menu_main, "c");
	if (item)
		item->sub_menu = alarm_get_stackmenu();

	loop = g_main_loop_new(NULL, FALSE);
	mm = stackmenu_new(menu_main, loop);
	stackmenu_run(mm);

	g_main_loop_run(loop);
	g_main_loop_unref(loop);

	if (sdk_is_connected())
		printf("exited without sdk disconnect\n");

	if (sdk_is_initialized())
		printf("exited without sdk de-initialization\n");

	return 0;
}

#include "battery_agent.hh"
#include "delegation_agent.hh"
#include "location_agent.hh"

#include "mnu_addon.hh"
#include "mnu_settings.h"

using namespace NuguClientKit;

static char data_battery_level[MENU_DATA_SIZE] = "1";
static char data_battery_charging[MENU_DATA_SIZE] = "1";
static char data_delegation_psid[MENU_DATA_SIZE] = "nugu.delegation.service";
static char data_delegation_data[MENU_DATA_SIZE] = "{ \"action\": \"test\" }";

static BatteryAgent* battery_agent;
static LocationAgent* location_agent;
static DelegationAgent* delegation_agent;

static int run_battery_level(Stackmenu* mm, StackmenuItem* menu, void* user_data)
{
    if (!battery_agent)
        return -1;

    printf("setBatteryLevel(%s)\n", data_battery_level);
    battery_agent->setBatteryLevel(strtol(data_battery_level, NULL, 10));

    return 0;
}

static int run_battery_charging(Stackmenu* mm, StackmenuItem* menu, void* user_data)
{
    if (!battery_agent)
        return -1;

    if (data_battery_charging[0] == '1') {
        printf("setCharging(true)\n");
        battery_agent->setCharging(true);
    } else if (data_battery_charging[0] == '0') {
        printf("setCharging(false)\n");
        battery_agent->setCharging(false);
    } else
        return -1;

    return 0;
}

static int run_delegation_request(Stackmenu* mm, StackmenuItem* menu, void* user_data)
{
    if (!delegation_agent)
        return -1;

    if (delegation_agent->request(data_delegation_psid, data_delegation_data) == false)
        return -1;

    return 0;
}

static StackmenuItem menu_addon[] = {
    { "*", " " AGENT_NAME_BATTERY },
    { "1", "setBatteryLevel", NULL, run_battery_level },
    { "1b", " - level", NULL, NULL, data_battery_level },
    { "2", "setCharging", NULL, run_battery_charging },
    { "2f", " - flag", NULL, NULL, data_battery_charging },
    { "-" },
    { "*", " " AGENT_NAME_LOCATION },
    { "-" },
    { "*", " " AGENT_NAME_DELEGATION },
    { "3", "request", NULL, run_delegation_request },
    { "-" },
    NULL
};

StackmenuItem* addon_get_stackmenu(void)
{
    return menu_addon;
}

int addon_init(NuguClientKit::NuguClient::CapabilityBuilder* builder)
{
    if (!builder)
        return -1;

    if (settings_is_agent_enabled(AGENT_NAME_BATTERY))
        battery_agent = new BatteryAgent();

    if (settings_is_agent_enabled(AGENT_NAME_LOCATION))
        location_agent = new LocationAgent();

    if (settings_is_agent_enabled(AGENT_NAME_DELEGATION))
        delegation_agent = new DelegationAgent();

    builder->add(battery_agent)
        ->add(location_agent)
        ->add(delegation_agent);

    return 0;
}

void addon_deinit(void)
{
    if (battery_agent) {
        delete battery_agent;
        battery_agent = nullptr;
    }

    if (location_agent) {
        delete location_agent;
        location_agent = nullptr;
    }

    if (delegation_agent) {
        delete delegation_agent;
        delegation_agent = nullptr;
    }
}

#include <stdio.h>

#include <glib.h>

#include <base/nugu_log.h>

#include <capability/capability_factory.hh>
#include <clientkit/nugu_client.hh>
#include <nugu.h>

#include "mnu_builtin.hh"
#include "mnu_addon.hh"
#include "mnu_settings.h"
#include "sdk_control.h"

using namespace NuguClientKit;

static NuguClient* nugu_client;
static INetworkManager* network_manager;
static INetworkManagerListener* network_listener;

static NetworkStatus network_status = NetworkStatus::DISCONNECTED;

class NetworkManagerListener : public INetworkManagerListener {
public:
    void onStatusChanged(NetworkStatus status)
    {
        network_status = status;

        switch (status) {
        case NetworkStatus::DISCONNECTED:
            printf(NUGU_ANSI_COLOR_LIGHTBLUE "Network disconnected.\n" NUGU_ANSI_COLOR_NORMAL);
            break;
        case NetworkStatus::CONNECTED:
            printf(NUGU_ANSI_COLOR_LIGHTBLUE "Network connected.\n" NUGU_ANSI_COLOR_NORMAL);
            break;
        case NetworkStatus::CONNECTING:
            printf(NUGU_ANSI_COLOR_LIGHTBLUE "Network connection in progress.\n" NUGU_ANSI_COLOR_NORMAL);
            break;
        default:
            break;
        }
    }

    void onError(NetworkError error)
    {
        switch (error) {
        case NetworkError::TOKEN_ERROR:
            printf(NUGU_ANSI_COLOR_RED "Network error [TOKEN_ERROR]\n" NUGU_ANSI_COLOR_NORMAL);
            break;
        case NetworkError::UNKNOWN:
            printf(NUGU_ANSI_COLOR_RED "Network error [UNKNOWN]\n" NUGU_ANSI_COLOR_NORMAL);
            break;
        }
    }
};

int sdk_init(void)
{
    if (nugu_client) {
        printf("already initialized\n");
        return -1;
    }

    nugu_client = new NuguClient();

    builtin_init(nugu_client->getCapabilityBuilder());
    addon_init(nugu_client->getCapabilityBuilder());

    nugu_client->getCapabilityBuilder()->construct();

    if (!nugu_client->initialize()) {
        printf("initialize() failed\n");
        return -1;
    }

    network_manager = nugu_client->getNetworkManager();

    network_listener = new NetworkManagerListener();
    network_manager->addListener(network_listener);

    printf("SDK initialized\n");

    return 0;
}

int sdk_deinit(void)
{
    if (network_listener) {
        if (network_manager)
            network_manager->removeListener(network_listener);

        delete network_listener;
        network_listener = nullptr;
    }

    if (nugu_client) {
        nugu_client->deInitialize();
        delete nugu_client;
        nugu_client = nullptr;
    }

    network_manager = nullptr;

    builtin_deinit();
    addon_deinit();

    printf("SDK de-initialized\n");

    return 0;
}

int sdk_is_initialized(void)
{
    if (nugu_client)
        return 1;

    return 0;
}

int sdk_connect(void)
{
    if (!network_manager) {
        printf("SDK not initialized\n");
        return -1;
    }

    if (settings_get_token() == NULL) {
        printf("token is empty\n");
        return -1;
    }

    network_manager->setToken(settings_get_token());

    if (!network_manager->connect()) {
        printf("network connection request - failed\n");
        return -1;
    }

    printf("network connection request - ok\n");

    return 0;
}

int sdk_disconnect(void)
{
    if (!network_manager) {
        printf("SDK not initialized\n");
        return -1;
    }

    if (!network_manager->disconnect()) {
        printf("network disconnection request - failed\n");
        return -1;
    }

    printf("network disconnection request - ok\n");

    return 0;
}

int sdk_is_connected(void)
{
    if (!nugu_client)
        return 0;

    if (!network_manager)
        return 0;

    if (network_status != NetworkStatus::CONNECTED)
        return 0;

    return 1;
}

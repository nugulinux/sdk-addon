#include <stdio.h>

#include <glib.h>

#include <base/nugu_log.h>
#include <capability/asr_interface.hh>
#include <capability/audio_player_interface.hh>
#include <capability/capability_factory.hh>
#include <capability/system_interface.hh>
#include <capability/text_interface.hh>
#include <capability/tts_interface.hh>
#include <clientkit/nugu_client.hh>
#include <nugu.h>

#include "battery_agent.hh"
#include "mnu_settings.h"
#include "sdk_control.h"

using namespace NuguClientKit;
using namespace NuguCapability;

static NuguClient* nugu_client;
static INetworkManager* network_manager;

static ISystemHandler* system_handler;
static IASRHandler* asr_handler;
static ITTSHandler* tts_handler;
static ITextHandler* text_handler;
static IAudioPlayerHandler* audioplayer_handler;
static BatteryAgent* battery_agent;

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

    if (settings_is_agent_enabled(AGENT_NAME_SYSTEM))
        system_handler = CapabilityFactory::makeCapability<SystemAgent, ISystemHandler>();

    if (settings_is_agent_enabled(AGENT_NAME_ASR)) {
        asr_handler = CapabilityFactory::makeCapability<ASRAgent, IASRHandler>();
        asr_handler->setAttribute(ASRAttribute {
            settings_get_asr_model_path(), "CLIENT", "PARTIAL" });
    }

    if (settings_is_agent_enabled(AGENT_NAME_TTS))
        tts_handler = CapabilityFactory::makeCapability<TTSAgent, ITTSHandler>();

    if (settings_is_agent_enabled(AGENT_NAME_TEXT))
        text_handler = CapabilityFactory::makeCapability<TextAgent, ITextHandler>();

    if (settings_is_agent_enabled(AGENT_NAME_AUDIOPLAYER))
        audioplayer_handler = CapabilityFactory::makeCapability<AudioPlayerAgent, IAudioPlayerHandler>();

    if (settings_is_agent_enabled(AGENT_NAME_BATTERY))
        battery_agent = new BatteryAgent();

    nugu_client = new NuguClient();
    nugu_client->getCapabilityBuilder()
        ->add(system_handler)
        ->add(asr_handler)
        ->add(tts_handler)
        ->add(text_handler)
        ->add(audioplayer_handler)
        ->add(battery_agent)
        ->construct();

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

    if (system_handler) {
        delete system_handler;
        system_handler = nullptr;
    }

    if (asr_handler) {
        delete asr_handler;
        asr_handler = nullptr;
    }

    if (tts_handler) {
        delete tts_handler;
        tts_handler = nullptr;
    }

    if (text_handler) {
        delete text_handler;
        text_handler = nullptr;
    }

    if (audioplayer_handler) {
        delete audioplayer_handler;
        audioplayer_handler = nullptr;
    }

    if (battery_agent) {
        delete battery_agent;
        battery_agent = nullptr;
    }

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

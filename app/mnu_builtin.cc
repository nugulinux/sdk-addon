#include <capability/capability_factory.hh>

#include <capability/asr_interface.hh>
#include <capability/audio_player_interface.hh>
#include <capability/system_interface.hh>
#include <capability/text_interface.hh>
#include <capability/tts_interface.hh>

#include "mnu_builtin.hh"
#include "mnu_settings.h"

using namespace NuguClientKit;
using namespace NuguCapability;

static char data_tts_volume[MENU_DATA_SIZE] = "50";
static char data_tts_text[MENU_DATA_SIZE] = "안녕하세요.";
static char data_tts_psid[MENU_DATA_SIZE] = "";
static char data_text_text[MENU_DATA_SIZE] = "며칠이야";
static char data_audio_seek[MENU_DATA_SIZE] = "10";
static char data_audio_favorite[MENU_DATA_SIZE] = "1";
static char data_audio_repeat[MENU_DATA_SIZE] = "one";
static char data_audio_shuffle[MENU_DATA_SIZE] = "1";
static char data_audio_volume[MENU_DATA_SIZE] = "50";
static char data_audio_mute[MENU_DATA_SIZE] = "1";

static ISystemHandler* system_handler;
static IASRHandler* asr_handler;
static ITTSHandler* tts_handler;
static ITextHandler* text_handler;
static IAudioPlayerHandler* audioplayer_handler;

static int run_system_sync(Stackmenu* mm, StackmenuItem* menu, void* user_data)
{
    if (!system_handler)
        return -1;

    system_handler->synchronizeState();

    return 0;
}

static int run_system_disconnect(Stackmenu* mm, StackmenuItem* menu, void* user_data)
{
    if (!system_handler)
        return -1;

    system_handler->disconnect();

    return 0;
}

static int run_system_useractivity(Stackmenu* mm, StackmenuItem* menu, void* user_data)
{
    if (!system_handler)
        return -1;

    system_handler->updateUserActivity();

    return 0;
}

static int run_asr_start_recognition(Stackmenu* mm, StackmenuItem* menu, void* user_data)
{
    if (!asr_handler)
        return -1;

    if (asr_handler->startRecognition() == false)
        return -1;

    return 0;
}

static int run_asr_stop_recognition(Stackmenu* mm, StackmenuItem* menu, void* user_data)
{
    if (!asr_handler)
        return -1;

    asr_handler->stopRecognition();

    return 0;
}

static int run_tts_stop(Stackmenu* mm, StackmenuItem* menu, void* user_data)
{
    if (!tts_handler)
        return -1;

    tts_handler->stopTTS();

    return 0;
}

static int run_tts_request(Stackmenu* mm, StackmenuItem* menu, void* user_data)
{
    if (!tts_handler)
        return -1;

    tts_handler->requestTTS(data_tts_text, data_tts_psid);

    return 0;
}

static int run_tts_volume(Stackmenu* mm, StackmenuItem* menu, void* user_data)
{
    if (!tts_handler)
        return -1;

    if (tts_handler->setVolume(atoi(data_tts_volume)) == false)
        return -1;

    return 0;
}

static int run_text_request(Stackmenu* mm, StackmenuItem* menu, void* user_data)
{
    if (!text_handler)
        return -1;

    if (text_handler->requestTextInput(data_text_text) == false)
        return -1;

    return 0;
}

static int run_audio_play(Stackmenu* mm, StackmenuItem* menu, void* user_data)
{
    if (!audioplayer_handler)
        return -1;

    audioplayer_handler->play();

    return 0;
}

static int run_audio_stop(Stackmenu* mm, StackmenuItem* menu, void* user_data)
{
    if (!audioplayer_handler)
        return -1;

    audioplayer_handler->stop();

    return 0;
}

static int run_audio_next(Stackmenu* mm, StackmenuItem* menu, void* user_data)
{
    if (!audioplayer_handler)
        return -1;

    audioplayer_handler->next();

    return 0;
}

static int run_audio_prev(Stackmenu* mm, StackmenuItem* menu, void* user_data)
{
    if (!audioplayer_handler)
        return -1;

    audioplayer_handler->prev();

    return 0;
}

static int run_audio_pause(Stackmenu* mm, StackmenuItem* menu, void* user_data)
{
    if (!audioplayer_handler)
        return -1;

    audioplayer_handler->pause();

    return 0;
}

static int run_audio_resume(Stackmenu* mm, StackmenuItem* menu, void* user_data)
{
    if (!audioplayer_handler)
        return -1;

    audioplayer_handler->resume();

    return 0;
}

static int run_audio_seek(Stackmenu* mm, StackmenuItem* menu, void* user_data)
{
    if (!audioplayer_handler)
        return -1;

    audioplayer_handler->seek(atoi(data_audio_seek));

    return 0;
}

static int run_audio_favorite(Stackmenu* mm, StackmenuItem* menu, void* user_data)
{
    if (!audioplayer_handler)
        return -1;

    if (data_audio_favorite[0] == '1')
        audioplayer_handler->setFavorite(true);
    else if (data_audio_favorite[0] == '0')
        audioplayer_handler->setFavorite(false);
    else
        return -1;

    return 0;
}

static int run_audio_repeat(Stackmenu* mm, StackmenuItem* menu, void* user_data)
{
    if (!audioplayer_handler)
        return -1;

    if (g_strcmp0(data_audio_repeat, "none") == 0)
        audioplayer_handler->setRepeat(RepeatType::NONE);
    else if (g_strcmp0(data_audio_repeat, "one") == 0)
        audioplayer_handler->setRepeat(RepeatType::ONE);
    else if (g_strcmp0(data_audio_repeat, "all") == 0)
        audioplayer_handler->setRepeat(RepeatType::ALL);
    else
        return -1;

    return 0;
}

static int run_audio_shuffle(Stackmenu* mm, StackmenuItem* menu, void* user_data)
{
    if (!audioplayer_handler)
        return -1;

    if (data_audio_shuffle[0] == '1')
        audioplayer_handler->setShuffle(true);
    else if (data_audio_shuffle[0] == '0')
        audioplayer_handler->setShuffle(false);
    else
        return -1;

    return 0;
}

static int run_audio_volume(Stackmenu* mm, StackmenuItem* menu, void* user_data)
{
    if (!audioplayer_handler)
        return -1;

    audioplayer_handler->setVolume(atoi(data_audio_volume));

    return 0;
}

static int run_audio_mute(Stackmenu* mm, StackmenuItem* menu, void* user_data)
{
    if (!audioplayer_handler)
        return -1;

    if (data_audio_mute[0] == '1')
        audioplayer_handler->setMute(true);
    else if (data_audio_mute[0] == '0')
        audioplayer_handler->setMute(false);
    else
        return -1;

    return 0;
}

static StackmenuItem menu_builtin_tts[] = {
    { "1", "stopTTS", NULL, run_tts_stop },
    { "-" },
    { "2", "requestTTS", NULL, run_tts_request },
    { "t", " - text", NULL, NULL, data_tts_text },
    { "p", " - play_service_id", NULL, NULL, data_tts_psid },
    { "-" },
    { "3", "setVolume", NULL, run_tts_volume },
    { "v", " - volume", NULL, NULL, data_tts_volume },
    NULL
};

static StackmenuItem menu_builtin_audio[] = {
    { "1", "play", NULL, run_audio_play },
    { "2", "stop", NULL, run_audio_stop },
    { "-" },
    { "3", "next", NULL, run_audio_next },
    { "4", "prev", NULL, run_audio_prev },
    { "-" },
    { "5", "pause", NULL, run_audio_pause },
    { "6", "resume", NULL, run_audio_resume },
    { "-" },
    { "7", "seek", NULL, run_audio_seek },
    { "7s", " - secs", NULL, NULL, data_audio_seek },
    { "8", "setFavorite", NULL, run_audio_favorite },
    { "8f", " - flag", NULL, NULL, data_audio_favorite },
    { "9", "setRepeat (none/one/all)", NULL, run_audio_repeat },
    { "9t", " - type", NULL, NULL, data_audio_repeat },
    { "10", "setShuffle", NULL, run_audio_shuffle },
    { "10f", " - flag", NULL, NULL, data_audio_shuffle },
    { "11", "setVolume", NULL, run_audio_volume },
    { "11v", " - volume", NULL, NULL, data_audio_volume },
    { "12", "setMute", NULL, run_audio_mute },
    { "12f", " - flag", NULL, NULL, data_audio_mute },
    NULL
};

static StackmenuItem menu_builtin[] = {
    { "*", " " AGENT_NAME_SYSTEM },
    { "1", "synchronizeState", NULL, run_system_sync },
    { "2", "disconnect", NULL, run_system_disconnect },
    { "3", "updateUserActivity", NULL, run_system_useractivity },
    { "-" },
    { "*", " " AGENT_NAME_ASR },
    { "4", "startRecognition", NULL, run_asr_start_recognition },
    { "5", "stopRecognition", NULL, run_asr_stop_recognition },
    { "-" },
    { "*", " " AGENT_NAME_TEXT },
    { "6", "requestTextInput", NULL, run_text_request },
    { "6t", " - text", NULL, NULL, data_text_text },
    { "-" },
    { "7", AGENT_NAME_TTS, menu_builtin_tts },
    { "8", AGENT_NAME_AUDIOPLAYER, menu_builtin_audio },
    NULL
};

StackmenuItem* builtin_get_stackmenu(void)
{
    return menu_builtin;
}

int builtin_init(NuguClientKit::NuguClient::CapabilityBuilder* builder)
{
    if (!builder)
        return -1;

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

    builder->add(system_handler)
        ->add(asr_handler)
        ->add(tts_handler)
        ->add(text_handler)
        ->add(audioplayer_handler);

    return 0;
}

void builtin_deinit(void)
{
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
}

/*
 * Copyright (c) 2019 SK Telecom Co., Ltd. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <base/nugu_log.h>
#include <string.h>

#include "alerts_audio_player.hh"

namespace NuguCapability {

static const char* CAPABILITY_NAME = "AudioPlayer";
static const char* CAPABILITY_VERSION = "1.2";

AlertsAudioPlayer::AlertsAudioPlayer()
    : Capability(CAPABILITY_NAME, CAPABILITY_VERSION)
    , cur_player(nullptr)
    , media_player(nullptr)
    , tts_player(nullptr)
    , speak_dir(nullptr)
    , is_tts_activate(false)
    , cur_aplayer_state(AudioPlayerState::IDLE)
    , prev_aplayer_state(AudioPlayerState::IDLE)
    , is_paused(false)
    , is_steal_focus(false)
    , ps_id("")
    , report_delay_time(-1)
    , report_interval_time(-1)
    , cur_token("")
    , pre_ref_dialog_id("")
    , is_finished(false)
    , cur_ndir(nullptr)
{
}

AlertsAudioPlayer::~AlertsAudioPlayer()
{
}

void AlertsAudioPlayer::initialize()
{
    if (initialized) {
        nugu_info("It's already initialized.");
        return;
    }

    media_player = core_container->createMediaPlayer();
    media_player->addListener(this);

    tts_player = core_container->createTTSPlayer();
    tts_player->addListener(this);

    cur_player = media_player;

    initialized = true;
}

void AlertsAudioPlayer::deInitialize()
{
    aplayer_listeners.clear();

    if (media_player) {
        media_player->removeListener(this);
        delete media_player;
        media_player = nullptr;
    }

    if (tts_player) {
        tts_player->removeListener(this);
        delete tts_player;
        tts_player = nullptr;
    }

    cur_player = nullptr;

    initialized = false;
}

void AlertsAudioPlayer::directiveDataCallback(NuguDirective* ndir, int seq, void* userdata)
{
    getAttachmentData(ndir, userdata);
}

void AlertsAudioPlayer::getAttachmentData(NuguDirective* ndir, void* userdata)
{
    AlertsAudioPlayer* agent = static_cast<AlertsAudioPlayer*>(userdata);
    unsigned char* buf;
    size_t length = 0;

    buf = nugu_directive_get_data(ndir, &length);
    if (buf) {
        agent->tts_player->write_audio((const char*)buf, length);
        free(buf);
    }

    if (nugu_directive_is_data_end(ndir)) {
        agent->tts_player->write_done();
        // agent->destroyDirective(ndir);
        agent->speak_dir = nullptr;
    }
}

std::string AlertsAudioPlayer::play()
{
    return "";
}

std::string AlertsAudioPlayer::stop(bool direct_access)
{
    return "";
}

std::string AlertsAudioPlayer::next()
{
    return "";
}

std::string AlertsAudioPlayer::prev()
{
    return "";
}

std::string AlertsAudioPlayer::pause(bool direct_access)
{
    return "";
}

std::string AlertsAudioPlayer::resume(bool direct_access)
{
    return "";
}

void AlertsAudioPlayer::seek(int msec)
{
    if (cur_player)
        cur_player->seek(msec * 1000);
}

std::string AlertsAudioPlayer::requestFavoriteCommand(bool favorite)
{
    return "";
}

std::string AlertsAudioPlayer::requestRepeatCommand(RepeatType repeat)
{
    return "";
}

std::string AlertsAudioPlayer::requestShuffleCommand(bool shuffle)
{
    return "";
}

bool AlertsAudioPlayer::setVolume(int volume)
{
    nugu_dbg("set media player's volume: %d", volume);
    if (!cur_player)
        return false;

    if (!cur_player->setVolume(volume))
        return false;

    nugu_dbg("media player's volume(%d) changed..", volume);
    return true;
}

bool AlertsAudioPlayer::setMute(bool mute)
{
    nugu_dbg("set media player's mute: %d", mute);
    if (!cur_player)
        return false;

    if (cur_player->setMute(mute))
        return false;

    nugu_dbg("media player's mute(%d) changed..", mute);
    return true;
}

void AlertsAudioPlayer::displayRendered(const std::string& id)
{
}

void AlertsAudioPlayer::displayCleared(const std::string& id)
{
}

void AlertsAudioPlayer::elementSelected(const std::string& id, const std::string& item_token)
{
}

void AlertsAudioPlayer::informControlResult(const std::string& id, ControlType type, ControlDirection direction)
{
}

void AlertsAudioPlayer::setListener(IDisplayListener* listener)
{
}

void AlertsAudioPlayer::removeListener(IDisplayListener* listener)
{
}

void AlertsAudioPlayer::stopRenderingTimer(const std::string& id)
{
}

void AlertsAudioPlayer::parsingDirective(const char* dname, const char* message)
{
    nugu_dbg("message: %s", message);

    if (!strcmp(dname, "Play"))
        parsingPlay(message);
    else
        nugu_warn("%s[%s] is not support %s directive", getName().c_str(), getVersion().c_str(), dname);
}

void AlertsAudioPlayer::updateInfoForContext(Json::Value& ctx)
{
    Json::Value aplayer;
    double offset = cur_player->position() * 1000;
    double duration = cur_player->duration() * 1000;

    aplayer["version"] = getVersion();
    aplayer["playerActivity"] = playerActivity(cur_aplayer_state);
    aplayer["offsetInMilliseconds"] = offset;
    if (cur_aplayer_state != AudioPlayerState::IDLE) {
        aplayer["token"] = cur_token;
        if (duration)
            aplayer["durationInMilliseconds"] = duration;
    }

    ctx[getName()] = aplayer;
}

bool AlertsAudioPlayer::receiveCommand(const std::string& from, const std::string& command, const std::string& param)
{
    std::string convert_command;
    convert_command.resize(command.size());
    std::transform(command.cbegin(), command.cend(), convert_command.begin(), ::tolower);

    if (!convert_command.compare("setvolume"))
        cur_player->setVolume(std::stoi(param));

    return true;
}

void AlertsAudioPlayer::setCapabilityListener(ICapabilityListener* listener)
{
    if (listener) {
        addListener(dynamic_cast<IAudioPlayerListener*>(listener));
    }
}

void AlertsAudioPlayer::addListener(IAudioPlayerListener* listener)
{
    auto iterator = std::find(aplayer_listeners.begin(), aplayer_listeners.end(), listener);

    if (iterator == aplayer_listeners.end())
        aplayer_listeners.emplace_back(listener);
}

void AlertsAudioPlayer::removeListener(IAudioPlayerListener* listener)
{
    auto iterator = std::find(aplayer_listeners.begin(), aplayer_listeners.end(), listener);

    if (iterator != aplayer_listeners.end())
        aplayer_listeners.erase(iterator);
}

void AlertsAudioPlayer::sendEventPlaybackStarted(EventResultCallback cb)
{
    sendEventCommon("PlaybackStarted", std::move(cb));
}

void AlertsAudioPlayer::sendEventPlaybackFinished(EventResultCallback cb)
{
    sendEventCommon("PlaybackFinished", std::move(cb));
}

void AlertsAudioPlayer::sendEventPlaybackStopped(EventResultCallback cb)
{
    sendEventCommon("PlaybackStopped", std::move(cb));
}

void AlertsAudioPlayer::sendEventPlaybackFailed(PlaybackError err, const std::string& reason, EventResultCallback cb)
{
    std::string ename = "PlaybackFailed";
    std::string payload = "";
    Json::StyledWriter writer;
    Json::Value root;
    long offset = cur_player->position() * 1000;

    if (offset < 0 || cur_token.size() == 0 || ps_id.size() == 0) {
        nugu_error("there is something wrong [%s]", ename.c_str());
        return;
    }

    root["token"] = cur_token;
    root["playServiceId"] = ps_id;
    root["offsetInMilliseconds"] = std::to_string(offset);
    root["error"]["type"] = playbackError(err);
    root["error"]["message"] = reason;
    root["currentPlaybackState"]["token"] = cur_token;
    root["currentPlaybackState"]["offsetInMilliseconds"] = std::to_string(offset);
    root["currentPlaybackState"]["playActivity"] = playerActivity(cur_aplayer_state);
    payload = writer.write(root);

    sendEvent(ename, getContextInfo(), payload, std::move(cb));
}

void AlertsAudioPlayer::sendEventProgressReportDelayElapsed(EventResultCallback cb)
{
    nugu_dbg("report_delay_time: %d, position: %d", report_delay_time / 1000, cur_player->position());
    sendEventCommon("ProgressReportDelayElapsed", std::move(cb));
}

void AlertsAudioPlayer::sendEventProgressReportIntervalElapsed(EventResultCallback cb)
{
    nugu_dbg("report_interval_time: %d, position: %d", report_interval_time / 1000, cur_player->position());
    sendEventCommon("ProgressReportIntervalElapsed", std::move(cb));
}

std::string AlertsAudioPlayer::sendEventByDisplayInterface(const std::string& command, EventResultCallback cb)
{
    return sendEventCommon(command, std::move(cb));
}

std::string AlertsAudioPlayer::sendEventCommon(const std::string& ename, EventResultCallback cb)
{
    std::string payload = "";
    Json::StyledWriter writer;
    Json::Value root;
    long offset = cur_player->position() * 1000;

    if (offset < 0 || cur_token.size() == 0 || ps_id.size() == 0) {
        nugu_error("there is something wrong [%s]", ename.c_str());
        return "";
    }

    root["token"] = cur_token;
    root["playServiceId"] = ps_id;
    root["offsetInMilliseconds"] = std::to_string(offset);
    payload = writer.write(root);

    return sendEvent(ename, getContextInfo(), payload, std::move(cb));
}

bool AlertsAudioPlayer::isContentCached(const std::string& key, std::string& playurl)
{
    std::string filepath;

    for (auto aplayer_listener : aplayer_listeners) {
        if (aplayer_listener->requestToGetCachedContent(key, filepath)) {
            playurl = filepath;
            return true;
        }
    }
    return false;
}

void AlertsAudioPlayer::parsingPlay(const char* message)
{
    Json::Value root;
    Json::Value audio_item;
    Json::Value stream;
    Json::Value report;
    Json::Reader reader;
    std::string source_type;
    std::string url;
    std::string cache_key;
    long offset;
    std::string token;
    std::string prev_token;
    std::string temp;
    std::string play_service_id;

    if (!reader.parse(message, root)) {
        nugu_error("parsing error");
        return;
    }

    audio_item = root["audioItem"];
    if (audio_item.empty()) {
        nugu_error("directive message syntex error");
        return;
    }
    play_service_id = root["playServiceId"].asString();
    cache_key = root["cacheKey"].asString();

    source_type = root["sourceType"].asString();
    stream = audio_item["stream"];
    if (stream.empty()) {
        nugu_error("directive message syntex error");
        return;
    }

    url = stream["url"].asString();
    offset = stream["offsetInMilliseconds"].asLargestInt();
    token = stream["token"].asString();
    prev_token = stream["expectedPreviousToken"].asString();

    if (token.size() == 0 || play_service_id.size() == 0) {
        nugu_error("There is no mandatory data in directive message");
        return;
    }

    if (source_type == "ATTACHMENT") {
        nugu_dbg("Receive Attachment media");
        destroy_directive_by_agent = true;
    } else {
        nugu_dbg("Receive Streaming media");
        if (url.size() == 0) {
            nugu_error("There is no mandatory data in directive message");
            return;
        }
    }

    if (cache_key.size()) {
        std::string filepath;
        if (isContentCached(cache_key, filepath)) {
            nugu_dbg("the content(key: %s) is cached in %s", cache_key.c_str(), filepath.c_str());
            url = filepath;
        } else {
            for (auto aplayer_listener : aplayer_listeners)
                aplayer_listener->requestContentCache(cache_key, url);
        }
    }

    report = stream["progressReport"];
    if (!report.empty()) {
        report_delay_time = report["progressReportDelayInMilliseconds"].asLargestInt();
        report_interval_time = report["progressReportIntervalInMilliseconds"].asLargestInt();
    }

    std::string playstackctl_ps_id = getPlayServiceIdInStackControl(root["playStackControl"]);

    if (!playstackctl_ps_id.empty()) {
        // playsync_manager->addContext(playstackctl_ps_id, getName());
    }

    is_finished = false;
    is_steal_focus = false;

    if (speak_dir) {
        nugu_directive_remove_data_callback(speak_dir);
        // destroyDirective(speak_dir);
        speak_dir = nullptr;
    }

    nugu_dbg("= token: %s", token.c_str());
    nugu_dbg("= url: %s", url.c_str());
    nugu_dbg("= offset: %d", offset);
    nugu_dbg("= report_delay_time: %d", report_delay_time);
    nugu_dbg("= report_interval_time: %d", report_interval_time);

    std::string dialog_id = nugu_directive_peek_dialog_id(getNuguDirective());
    std::string dname = nugu_directive_peek_name(getNuguDirective());
    // Different tokens mean different media play requests.
    if (token != cur_token) {

        if (pre_ref_dialog_id.size())
            setReferrerDialogRequestId(dname, pre_ref_dialog_id);

        if (!cur_player->stop()) {
            nugu_error("stop media failed");
            sendEventPlaybackFailed(PlaybackError::MEDIA_ERROR_INTERNAL_DEVICE_ERROR, "player can't stop");
        }

        std::string id = nugu_directive_peek_dialog_id(getNuguDirective());
        setReferrerDialogRequestId(dname, id);
        pre_ref_dialog_id = id;
    } else {
        setReferrerDialogRequestId(dname, pre_ref_dialog_id);
    }
    cur_dialog_id = dialog_id;
    cur_token = token;
    ps_id = play_service_id;

    if (destroy_directive_by_agent) {
        cur_player = dynamic_cast<IMediaPlayer*>(tts_player);
        is_tts_activate = true;
        speak_dir = getNuguDirective();
    } else {
        cur_player = media_player;
        is_tts_activate = false;
    }

    if (!cur_player->setSource(url)) {
        nugu_error("set source failed");
        sendEventPlaybackFailed(PlaybackError::MEDIA_ERROR_INTERNAL_DEVICE_ERROR, "can't set source");
        return;
    }
    if (offset >= 0 && !cur_player->seek(offset / 1000)) {
        nugu_error("seek media failed");
        sendEventPlaybackFailed(PlaybackError::MEDIA_ERROR_INTERNAL_DEVICE_ERROR, "can't seek");
        return;
    }
}

std::string AlertsAudioPlayer::playbackError(PlaybackError error)
{
    std::string err_str;

    switch (error) {
    case MEDIA_ERROR_INVALID_REQUEST:
        err_str = "MEDIA_ERROR_INVALID_REQUEST";
        break;
    case MEDIA_ERROR_SERVICE_UNAVAILABLE:
        err_str = "MEDIA_ERROR_SERVICE_UNAVAILABLE";
        break;
    case MEDIA_ERROR_INTERNAL_SERVER_ERROR:
        err_str = "MEDIA_ERROR_INTERNAL_SERVER_ERROR";
        break;
    case MEDIA_ERROR_INTERNAL_DEVICE_ERROR:
        err_str = "MEDIA_ERROR_INTERNAL_DEVICE_ERROR";
        break;
    default:
        err_str = "MEDIA_ERROR_UNKNOWN";
        break;
    }
    return err_str;
}

std::string AlertsAudioPlayer::playerActivity(AudioPlayerState state)
{
    std::string activity;

    switch (state) {
    case AudioPlayerState::IDLE:
        activity = "IDLE";
        break;
    case AudioPlayerState::PLAYING:
        activity = "PLAYING";
        break;
    case AudioPlayerState::PAUSED:
        activity = "PAUSED";
        break;
    case AudioPlayerState::STOPPED:
        activity = "STOPPED";
        break;
    case AudioPlayerState::FINISHED:
        activity = "FINISHED";
        break;
    }

    return activity;
}

void AlertsAudioPlayer::mediaStateChanged(MediaPlayerState state)
{
    switch (state) {
    case MediaPlayerState::IDLE:
        cur_aplayer_state = AudioPlayerState::IDLE;
        break;
    case MediaPlayerState::PREPARE:
    case MediaPlayerState::READY:
        prev_aplayer_state = AudioPlayerState::IDLE;
        return;
        break;
    case MediaPlayerState::PLAYING:
        cur_aplayer_state = AudioPlayerState::PLAYING;
        if (!is_steal_focus) {
            if (prev_aplayer_state != AudioPlayerState::PAUSED)
                sendEventPlaybackStarted();
        }
        is_steal_focus = false;
        break;
    case MediaPlayerState::PAUSED:
        cur_aplayer_state = AudioPlayerState::PAUSED;
        break;
    case MediaPlayerState::STOPPED:
        if (is_finished) {
            cur_aplayer_state = AudioPlayerState::FINISHED;
            sendEventPlaybackFinished();
        } else {
            cur_aplayer_state = AudioPlayerState::STOPPED;
            sendEventPlaybackStopped();
        }
        is_paused = false;
        is_steal_focus = false;
        break;
    default:
        break;
    }
    nugu_info("[audioplayer state] %s -> %s", playerActivity(prev_aplayer_state).c_str(), playerActivity(cur_aplayer_state).c_str());

    if (prev_aplayer_state == cur_aplayer_state)
        return;

    for (auto aplayer_listener : aplayer_listeners)
        aplayer_listener->mediaStateChanged(cur_aplayer_state, cur_dialog_id);

    if ((is_paused || is_steal_focus) && cur_aplayer_state == AudioPlayerState::PAUSED) {
        nugu_info("[audioplayer state] skip to save prev_aplayer_state");
        return;
    }

    prev_aplayer_state = cur_aplayer_state;
}

void AlertsAudioPlayer::mediaEventReport(MediaPlayerEvent event)
{
    switch (event) {
    case MediaPlayerEvent::INVALID_MEDIA_URL:
        nugu_warn("INVALID_MEDIA_URL");
        sendEventPlaybackFailed(PlaybackError::MEDIA_ERROR_INVALID_REQUEST, "media source is wrong");

        for (auto aplayer_listener : aplayer_listeners)
            aplayer_listener->mediaEventReport(AudioPlayerEvent::INVALID_URL, cur_dialog_id);
        break;
    case MediaPlayerEvent::LOADING_MEDIA_FAILED:
        nugu_warn("LOADING_MEDIA_FAILED");
        sendEventPlaybackFailed(PlaybackError::MEDIA_ERROR_SERVICE_UNAVAILABLE, "player can't load the media");

        for (auto aplayer_listener : aplayer_listeners)
            aplayer_listener->mediaEventReport(AudioPlayerEvent::LOAD_FAILED, cur_dialog_id);
        break;
    case MediaPlayerEvent::LOADING_MEDIA_SUCCESS:
        nugu_dbg("LOADING_MEDIA_SUCCESS");
        break;
    case MediaPlayerEvent::PLAYING_MEDIA_FINISHED:
        nugu_dbg("PLAYING_MEDIA_FINISHED");
        if (cur_player == media_player) {
            cur_player->setPosition(0);
            cur_player->play();
        } else if (cur_player == tts_player) {
            is_finished = true;
            mediaStateChanged(MediaPlayerState::STOPPED);
        }
        break;
    case MediaPlayerEvent::PLAYING_MEDIA_UNDERRUN:
        for (auto aplayer_listener : aplayer_listeners) {
            aplayer_listener->mediaEventReport(AudioPlayerEvent::UNDERRUN, cur_dialog_id);
        }
        break;
    default:
        break;
    }
}

void AlertsAudioPlayer::mediaChanged(const std::string& url)
{
}

void AlertsAudioPlayer::durationChanged(int duration)
{
    for (auto aplayer_listener : aplayer_listeners)
        aplayer_listener->durationChanged(duration);
}

void AlertsAudioPlayer::positionChanged(int position)
{
    if (report_delay_time > 0 && ((report_delay_time / 1000) == position))
        sendEventProgressReportDelayElapsed();

    if (report_interval_time > 0 && ((position % (int)(report_interval_time / 1000)) == 0))
        sendEventProgressReportIntervalElapsed();

    for (auto aplayer_listener : aplayer_listeners)
        aplayer_listener->positionChanged(position);
}

void AlertsAudioPlayer::volumeChanged(int volume)
{
}

void AlertsAudioPlayer::muteChanged(int mute)
{
}

void AlertsAudioPlayer::setNuguDirective(NuguDirective* ndir)
{
    nugu_info("set nugu directive => %p", ndir);
    cur_ndir = ndir;
}

NuguDirective* AlertsAudioPlayer::getNuguDirective()
{
    nugu_info("get nugu directive => %p", cur_ndir);
    return cur_ndir;
}

bool AlertsAudioPlayer::hasAttachment()
{
    return is_tts_activate;
}

IMediaPlayer* AlertsAudioPlayer::getPlayer()
{
    return cur_player;
}

bool AlertsAudioPlayer::playMedia()
{
    std::string type = hasAttachment() ? "attachment" : "streaming";

    cur_player = media_player;

    nugu_dbg("cur_aplayer_state[%s] => %d, player->state() => %s", type.c_str(), cur_aplayer_state, cur_player->stateString(cur_player->state()).c_str());

    if (!cur_player->play()) {
        nugu_error("play media(%s) failed", type.c_str());
        sendEventPlaybackFailed(PlaybackError::MEDIA_ERROR_INTERNAL_DEVICE_ERROR,
            "player can't play media");
        return false;
    }
    if (hasAttachment()) {
        if (nugu_directive_get_data_size(speak_dir) > 0)
            getAttachmentData(speak_dir, this);

        if (speak_dir)
            nugu_directive_set_data_callback(speak_dir, directiveDataCallback, this);
    }
    return true;
}

bool AlertsAudioPlayer::playTTS()
{
    unsigned char* buf;
    size_t length = 0;

    length = nugu_directive_get_data_size(cur_ndir);
    if (length <= 0) {
        nugu_error("no attachment data (size = %zd)", length);
        return false;
    }

    cur_player = tts_player;
    cur_player->play();

    buf = nugu_directive_get_data(cur_ndir, &length);
    nugu_info("write_audio %zd bytes", length);
    tts_player->write_audio((char*)buf, length);
    free(buf);

    tts_player->write_done();

    destroyDirective(cur_ndir);
    cur_ndir = NULL;

    return true;
}

bool AlertsAudioPlayer::isTTSPlayer()
{
    if (cur_player == tts_player)
        return true;

    return false;
}

} // NuguCapability

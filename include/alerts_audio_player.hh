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

#ifndef __NUGU_ALERTS_AUDIO_PLAYER_H__
#define __NUGU_ALERTS_AUDIO_PLAYER_H__

#include <capability/audio_player_interface.hh>
#include <clientkit/capability.hh>

namespace NuguCapability {

class AlertsAudioPlayer final : public NuguClientKit::Capability,
                                public NuguClientKit::IMediaPlayerListener,
                                public IAudioPlayerHandler {
public:
    enum PlaybackError {
        MEDIA_ERROR_UNKNOWN,
        MEDIA_ERROR_INVALID_REQUEST,
        MEDIA_ERROR_SERVICE_UNAVAILABLE,
        MEDIA_ERROR_INTERNAL_SERVER_ERROR,
        MEDIA_ERROR_INTERNAL_DEVICE_ERROR
    };

public:
    AlertsAudioPlayer();
    virtual ~AlertsAudioPlayer();

    void initialize() override;
    void deInitialize() override;

    void parsingDirective(const char* dname, const char* message) override;
    void updateInfoForContext(Json::Value& ctx) override;
    bool receiveCommand(const std::string& from, const std::string& command, const std::string& param) override;
    void setCapabilityListener(ICapabilityListener* clistener) override;

    static void directiveDataCallback(NuguDirective* ndir, int seq, void* userdata);
    static void getAttachmentData(NuguDirective* ndir, void* userdata);

    // implements IAudioPlayerHandler
    void addListener(IAudioPlayerListener* listener) override;
    void removeListener(IAudioPlayerListener* listener) override;
    std::string play() override;
    std::string stop(bool direct_access = false) override;
    std::string next() override;
    std::string prev() override;
    std::string pause(bool direct_access = false) override;
    std::string resume(bool direct_access = false) override;
    void seek(int msec) override;
    std::string requestFavoriteCommand(bool current_favorite) override;
    std::string requestRepeatCommand(RepeatType current_repeat) override;
    std::string requestShuffleCommand(bool current_shuffle) override;

    bool setVolume(int volume) override;
    bool setMute(bool mute) override;

    // implements IDisplayHandler
    void displayRendered(const std::string& id);
    void displayCleared(const std::string& id);
    void elementSelected(const std::string& id, const std::string& item_token, const std::string& postback = "");
    void informControlResult(const std::string& id, ControlType type, ControlDirection direction);
    void setListener(IDisplayListener* listener);
    void removeListener(IDisplayListener* listener);
    void stopRenderingTimer(const std::string& id);

    void sendEventPlaybackStarted(EventResultCallback cb = nullptr);
    void sendEventPlaybackFinished(EventResultCallback cb = nullptr);
    void sendEventPlaybackStopped(EventResultCallback cb = nullptr);
    void sendEventPlaybackFailed(PlaybackError err, const std::string& reason, EventResultCallback cb = nullptr);
    void sendEventProgressReportDelayElapsed(EventResultCallback cb = nullptr);
    void sendEventProgressReportIntervalElapsed(EventResultCallback cb = nullptr);
    std::string sendEventByDisplayInterface(const std::string& command, EventResultCallback cb = nullptr);

    void mediaStateChanged(MediaPlayerState state) override;
    void mediaEventReport(MediaPlayerEvent event) override;
    void mediaFinished();
    void mediaLoaded();

    void mediaChanged(const std::string& url) override;
    void durationChanged(int duration) override;
    void positionChanged(int position) override;
    void volumeChanged(int volume) override;
    void muteChanged(int mute) override;

    void setNuguDirective(NuguDirective* ndir);
    NuguDirective* getNuguDirective();
    bool hasAttachment();
    IMediaPlayer* getPlayer();
    bool playMedia();
    bool playTTS();
    bool isTTSPlayer();

    void setRepeat(bool repeat);

private:
    std::string sendEventCommon(const std::string& ename, EventResultCallback cb = nullptr);

    AudioPlayerState audioPlayerState();

    bool isContentCached(const std::string& key, std::string& playurl);
    void parsingPlay(const char* message);
    void parsingPause(const char* message);
    void parsingStop(const char* message);

    std::string playbackError(PlaybackError error);
    std::string playerActivity(AudioPlayerState state);

    const unsigned int PAUSE_CONTEXT_HOLD_TIME = 60 * 10;

    IMediaPlayer* cur_player;
    IMediaPlayer* media_player;
    ITTSPlayer* tts_player;
    NuguDirective* speak_dir;
    bool is_tts_activate;

    AudioPlayerState cur_aplayer_state;
    AudioPlayerState prev_aplayer_state;
    bool is_paused;
    bool is_steal_focus;
    std::string ps_id;
    long report_delay_time;
    long report_interval_time;
    std::string cur_token;
    std::string pre_ref_dialog_id;
    std::string cur_dialog_id;
    bool is_finished;
    std::vector<IAudioPlayerListener*> aplayer_listeners;
    bool is_repeat;

    NuguDirective* cur_ndir;
    bool destroy_directive_by_agent = false;
};

} // NuguCapability

#endif

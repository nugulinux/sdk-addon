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

#ifndef __NUGU_ALERTS_AGENT_H__
#define __NUGU_ALERTS_AGENT_H__

#include <clientkit/capability.hh>

#include "alerts_audio_player.hh"
#include "base_audio_player_listener.hh"

#include <glib.h>
#include <json/json.h>

using namespace NuguClientKit;

typedef struct _AlertItem AlertItem;

class AlertsManager;

class IAlertsManagerListener {
public:
    virtual ~IAlertsManagerListener() = default;

    virtual void onTimeout(const std::string& token) = 0;
    virtual void onAssetRequireTimeout(const std::string& token) = 0;
    virtual void onDurationTimeout(const std::string& token) = 0;
};

class IAlertsListener : public ICapabilityListener {
public:
    virtual ~IAlertsListener() = default;

    virtual void onAlertStart(const std::string& token, const std::string& type, const std::string& resourceType) = 0;
    virtual void onAlertStop(const std::string& token, const std::string& type) = 0;

    /* Called for external playback when media playback is not possible inside AlertAgent */
    virtual void onFilePlayRequest(const std::string& token, const std::string& type, const std::string& resourceType) = 0;

    /* SetAlerts directive received from server */
    virtual void onSetAlert(const char* message, const std::string& type) = 0;

    /* DeleteAlerts directive received from server (support multiple tokens) */
    virtual void onDeleteAlerts(const char* message) = 0;

    virtual void onAlertAdd(const std::string& json) = 0;
    virtual void onAlertDelete(const std::string& token) = 0;
};

class AlertsAgent : public Capability,
                    public IFocusResourceListener,
                    public BaseAudioPlayerListener,
                    public IAlertsManagerListener {
public:
    AlertsAgent();
    virtual ~AlertsAgent();

    void initialize() override;
    void deInitialize() override;

    void setCapabilityListener(ICapabilityListener* clistener) override;
    void parsingDirective(const char* dname, const char* message) override;
    void updateInfoForContext(Json::Value& ctx) override;

    bool addAlert(const Json::Value& item);
    bool removeAlert(const std::string& token);

    Json::Value getAlertList();
    int getAlertCount();
    void resetAlerts();

    void stopSound(const std::string& reason, bool keep_playstack = false);
    bool isAlertPlaying();

    void setEnable(bool flag);
    bool isEnable();

    void unHoldAlarm();
    void holdAlarmUntilTextProcess();
    void unholdAlarmUntilTextProcess();

private:
    void releaseFocus();
    void playSound();
    void complete(AlertItem *item);

    /* Events */
    void sendEventCommon(const std::string& ename, const std::string& ps_id, const std::string& token, const std::string& error = "");
    void sendEventCommon(const std::string& ename, const std::string& ps_id, std::list<std::string> tokens);
    void sendEventSetAlertSucceeded(const std::string& ps_id, const std::string& token);
    void sendEventSetAlertFailed(const std::string& ps_id, const std::string& token, const std::string& reason = "ALERTS_SETALERT_INTERNAL");
    void sendEventDeleteAlertsSucceeded(const std::string& ps_id, const std::vector<std::string>& token_list);
    void sendEventDeleteAlertsFailed(const std::string& ps_id, const std::vector<std::string>& token_list);
    void sendEventSetSnoozeSucceeded(const std::string& ps_id, const std::string& token);
    void sendEventSetSnoozeFailed(const std::string& ps_id, const std::string& token);
    void sendEventAlertStarted(const std::string& ps_id, const std::string& token);
    void sendEventAlertFailed(const std::string& ps_id, const std::string& token, const std::string& error);
    void sendEventAlertIgnored(const std::string& ps_id, const std::vector<std::string>& token_list);
    void sendEventAlertStopped(const std::string& ps_id, const std::string& token);
    void sendEventAlertEnteredForeground(const std::string& ps_id, const std::string& token);
    void sendEventAlertEnteredBackground(const std::string& ps_id, const std::string& token);
    void sendEventAlertAssetRequired(const std::string& ps_id, const std::string& token);

    /* Directives */
    void parsingSetAlert(const char* message);
    void parsingDeleteAlerts(const char* message);
    void parsingDeliveryAlertAsset(const char* message);
    void parsingSetSnooze(const char* message);

    /* IFocusResourceListener */
    void onFocusChanged(FocusState state) override;

    /* BaseAudioPlayerListener */
    void mediaStateChanged(NuguCapability::AudioPlayerState state, const std::string& dialog_id) override;
    void mediaEventReport(NuguCapability::AudioPlayerEvent event, const std::string& dialog_id) override;
    void durationChanged(int duration) override;

    /* IAlertsManagerListener */
    void onTimeout(const std::string& token) override;
    void onAssetRequireTimeout(const std::string& token) override;
    void onDurationTimeout(const std::string& token) override;

    NuguCapability::AlertsAudioPlayer* audioplayer;
    std::string playstackctl_ps_id;
    FocusState focus_state;
    NuguDirective* directive_for_sync;

    IAlertsListener* alerts_listener = nullptr;

    bool hold_alarm_by_text;
    bool is_enable;

    AlertsManager* manager;
    AlertItem* current;
};

#endif /* __NUGU_ALERTS_AGENT_H__ */

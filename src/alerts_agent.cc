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

#include "alerts_agent.hh"
#include "alerts_manager.hh"

#include <base/nugu_log.h>
#include <glib.h>
#include <json/json.h>
#include <string.h>
#include <time.h>

#include <map>
#include <string>
#include <vector>

/**
 * Maximum number of ALARM type alerts.
 * Must have a value less than MAX_ALERTS
 */
#define MAX_ALARM 50

/**
 * Maximum number of alerts(including ALARM type).
 * MAX_ALARM + (1 TIMER + 1 SLEEP)
 */
#define MAX_ALERTS (MAX_ALARM + 2)

using namespace NuguCapability;

static const char* CAPABILITY_NAME = "Alerts";
static const char* CAPABILITY_VERSION = "1.1";

AlertsAgent::AlertsAgent()
    : Capability(CAPABILITY_NAME, CAPABILITY_VERSION)
    , audioplayer(nullptr)
    , playstackctl_ps_id("")
    , manager(new AlertsManager())
    , current(nullptr)
{
    directive_for_sync = nugu_directive_new("Alerts", "SetAlert",
        CAPABILITY_VERSION, "", "", "", "{}",
        "{ \"directives\": [\"Alerts.SetAlert\"] }");

    manager->setListener(this);
}

AlertsAgent::~AlertsAgent()
{
    nugu_directive_unref(directive_for_sync);
    delete manager;
}

void AlertsAgent::initialize()
{
    if (initialized) {
        nugu_warn("It's already initialized.");
        return;
    }

    Capability::initialize();

    addReferrerEvents("SetAlertSucceeded", "SetAlert");
    addReferrerEvents("SetAlertFailed", "SetAlert");
    addReferrerEvents("DeleteAlertsSucceeded", "DeleteAlerts");
    addReferrerEvents("DeleteAlertsFailed", "DeleteAlerts");
    addReferrerEvents("SetSnoozeSucceeded", "SetSnooze");
    addReferrerEvents("SetSnoozeFailed", "SetSnooze");
    addReferrerEvents("SetAlertAssetSucceeded", "DeliveryAlertAsset");
    addReferrerEvents("SetAlertAssetFailed", "DeliveryAlertAsset");

    audioplayer = new AlertsAudioPlayer();
    audioplayer->setNuguCoreContainer(core_container);
    audioplayer->initialize();
    audioplayer->addListener(this);

    playsync_manager->registerCapabilityForSync(CAPABILITY_NAME);

    initialized = true;
    is_enable = true;
    focus_state = FocusState::NONE;
}

void AlertsAgent::deInitialize()
{
    if (current)
        stopSound("deInitialize");

    if (audioplayer) {
        audioplayer->removeListener(this);
        audioplayer->deInitialize();
        delete audioplayer;
        audioplayer = nullptr;
    }

    initialized = false;
    is_enable = false;
}

void AlertsAgent::setCapabilityListener(ICapabilityListener* clistener)
{
    if (clistener)
        alerts_listener = dynamic_cast<IAlertsListener*>(clistener);
}

void AlertsAgent::releaseFocus()
{
    nugu_info("releaseFocus - " ALERTS_FOCUS_TYPE);
    focus_manager->releaseFocus(ALERTS_FOCUS_TYPE, CAPABILITY_NAME);
}

void AlertsAgent::onFocusChanged(FocusState state)
{
    nugu_info("Focus Changed(%s -> %s)",
        focus_manager->getStateString(focus_state).c_str(),
        focus_manager->getStateString(state).c_str());

    switch (state) {
    case FocusState::FOREGROUND:
        playSound();
        break;
    case FocusState::BACKGROUND:
        stopSound("focus-background");
        break;
    case FocusState::NONE: {
        IMediaPlayer* player = audioplayer->getPlayer();
        if (player != nullptr) {
            player->stop();
        }
    } break;
    }

    focus_state = state;
}

void AlertsAgent::parsingDirective(const char* dname, const char* message)
{
    if (!strcmp(dname, "SetAlert"))
        parsingSetAlert(message);
    else if (!strcmp(dname, "DeleteAlerts"))
        parsingDeleteAlerts(message);
    else if (!strcmp(dname, "DeliveryAlertAsset"))
        parsingDeliveryAlertAsset(message);
    else if (!strcmp(dname, "SetSnooze"))
        parsingSetSnooze(message);
    else {
        nugu_warn("%s[%s] is not support %s directive", getName().c_str(), getVersion().c_str(), dname);
    }
}

void AlertsAgent::updateInfoForContext(Json::Value& ctx)
{
    Json::Value alerts;

    alerts["version"] = getVersion();
    alerts["maxAlertCount"] = MAX_ALERTS;
    alerts["maxAlarmCount"] = MAX_ALARM;
    alerts["supportedTypes"][0] = "TIMER";
    alerts["supportedTypes"][1] = "ALARM";
    alerts["supportedTypes"][2] = "SLEEP";
    alerts["supportedTypes"][3] = "ACTION";
    alerts["supportedAlarmResourceTypes"][0] = "INTERNAL";
    alerts["supportedAlarmResourceTypes"][1] = "MUSIC";
    alerts["supportedAlarmResourceTypes"][2] = "TTS";
    alerts["internalAlarms"][0]["BASIC"] = "기본 알람음";

    Json::Value allAlerts = getAlertList();
    if (allAlerts.empty()) {
        alerts["allAlerts"] = Json::Value(Json::arrayValue);
    } else {
        alerts["allAlerts"] = allAlerts;
    }

    if (current != nullptr) {
        if (current->type == ALERT_TYPE_ALARM)
            alerts["activeAlarmToken"] = current->token;
    }

    ctx[getName()] = alerts;
}

void AlertsAgent::sendEventSetAlertSucceeded(const std::string& ps_id, const std::string& token)
{
    sendEventCommon("SetAlertSucceeded", ps_id, token);
}

void AlertsAgent::sendEventSetAlertFailed(const std::string& ps_id, const std::string& token, const std::string& reason)
{
    sendEventCommon("SetAlertFailed", ps_id, token, reason);
}

void AlertsAgent::sendEventDeleteAlertsSucceeded(const std::string& ps_id, const std::vector<std::string>& token_list)
{
    Json::Value root;
    Json::FastWriter writer;

    root["playServiceId"] = ps_id;
    for (const auto& token : token_list)
        root["tokens"].append(token);

    sendEvent("DeleteAlertsSucceeded", getContextInfo(), writer.write(root));
}

void AlertsAgent::sendEventDeleteAlertsFailed(const std::string& ps_id, const std::vector<std::string>& token_list)
{
    Json::Value root;
    Json::FastWriter writer;

    root["playServiceId"] = ps_id;
    for (const auto& token : token_list)
        root["tokens"].append(token);

    sendEvent("DeleteAlertsFailed", getContextInfo(), writer.write(root));
}

void AlertsAgent::sendEventSetSnoozeSucceeded(const std::string& ps_id, const std::string& token)
{
    sendEventCommon("SetSnoozeSucceeded", ps_id, token);
}

void AlertsAgent::sendEventSetSnoozeFailed(const std::string& ps_id, const std::string& token)
{
    sendEventCommon("SetSnoozeFailed", ps_id, token);
}

void AlertsAgent::sendEventAlertStarted(const std::string& ps_id, const std::string& token)
{
    sendEventCommon("AlertStarted", ps_id, token);
}

void AlertsAgent::sendEventAlertFailed(const std::string& ps_id, const std::string& token, const std::string& error)
{
    sendEventCommon("AlertFailed", ps_id, token, error);
}

void AlertsAgent::sendEventAlertIgnored(const std::string& ps_id, const std::vector<std::string>& token_list)
{
    Json::Value root;
    Json::FastWriter writer;

    root["playServiceId"] = ps_id;
    for (const auto& token : token_list)
        root["tokens"].append(token);

    sendEvent("AlertIgnored", getContextInfo(), writer.write(root));
}

void AlertsAgent::sendEventAlertStopped(const std::string& ps_id, const std::string& token)
{
    sendEventCommon("AlertStopped", ps_id, token);
}

void AlertsAgent::sendEventAlertEnteredForeground(const std::string& ps_id, const std::string& token)
{
    sendEventCommon("AlertEnteredForeground", ps_id, token);
}

void AlertsAgent::sendEventAlertEnteredBackground(const std::string& ps_id, const std::string& token)
{
    sendEventCommon("AlertEnteredBackground", ps_id, token);
}

void AlertsAgent::sendEventAlertAssetRequired(const std::string& ps_id, const std::string& token)
{
    sendEventCommon("AlertAssetRequired", ps_id, token);
}

void AlertsAgent::sendEventCommon(const std::string& ename, const std::string& ps_id, const std::string& token, const std::string& error)
{
    std::string payload = "";
    Json::Value root;
    Json::FastWriter writer;

    root["playServiceId"] = ps_id;
    root["token"] = token;
    if (!error.empty()) {
        root["errorCode"] = error;
    }
    payload = writer.write(root);

    sendEvent(ename, getContextInfo(), payload);
}

void AlertsAgent::parsingSetAlert(const char* message)
{
    Json::Value root;
    Json::Reader reader;
    std::string ps_id;
    std::string token;
    std::string type;
    std::string schedule_time;

    if (!reader.parse(message, root)) {
        nugu_error("parsing error");
        return;
    }

    ps_id = root["playServiceId"].asString();
    token = root["token"].asString();
    type = root["alertType"].asString();
    schedule_time = root["scheduledTime"].asString();

    if (ps_id.size() == 0 || token.size() == 0 || type.size() == 0
        || schedule_time.size() == 0 || root["activation"].empty()) {
        nugu_error("There is no mandatory data in directive message");
        return;
    }

    if ((type == "ALARM" || type == "ACTION") && root["assets"].empty()) {
        nugu_error("There is no mandatory data in directive message");
        sendEventSetAlertFailed(ps_id, token);
        return;
    }

    nugu_info("parsingSetAlert");

    if (current && current->token == token)
        stopSound("ringing-SetAlert");

    AlertItem* alert = manager->generateAlert(root);
    if (!alert) {
        nugu_error("internal error");
        sendEventSetAlertFailed(ps_id, token);
        return;
    }

    /* remove same token */
    removeAlert(token);
    if (alerts_listener)
        alerts_listener->onAlertDelete(token);

    if (manager->processDuplication(alert)) {
        delete alert;
        nugu_error("duplicated alerts");
        sendEventSetAlertFailed(ps_id, token, "ALERTS_SETALERT_DUPLICATE");
        return;
    }

    if (manager->addItem(alert) == false) {
        delete alert;
        nugu_error("setAlerts failed");
        sendEventSetAlertFailed(ps_id, token);
        return;
    }

    if (alerts_listener)
        alerts_listener->onAlertAdd(token);

    manager->scheduling();
    manager->dump();

    if (alerts_listener)
        alerts_listener->onSetAlert(message, type);

    // add to play context stack
    if (root.isMember("playStackControl")) {
        playstackctl_ps_id = getPlayServiceIdInStackControl(root["playStackControl"]);
    }

    sendEventSetAlertSucceeded(ps_id, token);
}

void AlertsAgent::parsingDeleteAlerts(const char* message)
{
    Json::Value root;
    Json::Reader reader;
    Json::Value tokens;
    std::string ps_id;
    std::vector<std::string> list_success;
    std::vector<std::string> list_failed;

    if (!reader.parse(message, root)) {
        nugu_error("parsing error");
        return;
    }

    ps_id = root["playServiceId"].asString();
    tokens = root["tokens"];

    if (ps_id.size() == 0 || tokens.empty()) {
        nugu_error("There is no mandatory data in directive message");
        return;
    }

    nugu_info("parsingDeleteAlerts");

    nugu_dbg("remove %d alarms", tokens.size());

    for (int i = 0; i < (int)tokens.size(); i++) {
        std::string token = tokens[i].asString();

        nugu_info("remove %d/%d: %s", i + 1, tokens.size(), token.c_str());

        if (current && current->token == token)
            stopSound("ringing-DeleteAlert");

        if (removeAlert(token) == true)
            list_success.push_back(token);
        else
            list_failed.push_back(token);

        if (alerts_listener)
            alerts_listener->onAlertDelete(token);
    }

    manager->dump();

    if (list_success.size() > 0)
        sendEventDeleteAlertsSucceeded(ps_id, list_success);

    if (list_failed.size() > 0)
        sendEventDeleteAlertsFailed(ps_id, list_failed);

    if (alerts_listener)
        alerts_listener->onDeleteAlerts(message);
}

void AlertsAgent::parsingDeliveryAlertAsset(const char* message)
{
    Json::Value root;
    Json::Reader reader;
    Json::StyledWriter writer;
    Json::Value asset_detail;
    std::string token;
    std::string ps_id;

    if (!reader.parse(message, root)) {
        nugu_error("parsing error");
        return;
    }

    ps_id = root["playServiceId"].asString();
    token = root["token"].asString();

    if (ps_id.size() == 0 || token.size() == 0 || root["assetDetails"].empty()) {
        nugu_error("There is no mandatory data in directive message");
        return;
    }

    nugu_info("parsingDeliveryAlertAsset");

    asset_detail = root["assetDetails"];

    for (int i = 0; i < (int)asset_detail.size(); i++) {
        Json::Value asset = asset_detail[i];
        Json::Value header = asset["header"];
        Json::Value payload = asset["payload"];
        std::string type;

        if (header.empty() || payload.empty()) {
            nugu_error("There is no header or payload");
            continue;
        }

        type = header["namespace"].asString() + "." + header["name"].asString();
        nugu_dbg("%s", type.c_str());

        if (type == "TTS.Attachment") {
            destroy_directive_by_agent = true;
            audioplayer->setNuguDirective(getNuguDirective());
        } else if (type == "AudioPlayer.Play") {
            audioplayer->setNuguDirective(getNuguDirective());
            audioplayer->parsingDirective(header["name"].asCString(), writer.write(payload).c_str());
        } else {
            nugu_warn("%s is not support", type.c_str());
            continue;
        }
    }
}

void AlertsAgent::parsingSetSnooze(const char* message)
{
    Json::Value root;
    Json::Reader reader;
    std::string token;
    std::string ps_id;
    int duration_sec;

    if (!reader.parse(message, root)) {
        nugu_error("parsing error");
        return;
    }

    ps_id = root["playServiceId"].asString();
    token = root["token"].asString();

    if (ps_id.size() == 0 || token.size() == 0 || root["durationInSec"].empty()) {
        nugu_error("There is no mandatory data in directive message");
        sendEventSetSnoozeFailed(ps_id, token);
        return;
    }

    duration_sec = root["durationInSec"].asInt();
    if (duration_sec <= 0) {
        nugu_error("There is no mandatory data in directive message");
        sendEventSetSnoozeFailed(ps_id, token);
        return;
    }

    AlertItem* item = manager->findItem(token);
    if (item == nullptr) {
        nugu_error("can't find the token");
        sendEventSetSnoozeFailed(ps_id, token);
        return;
    }

    manager->activate(item);
    item->snooze_secs = duration_sec;

    manager->scheduling();
    manager->dump();

    sendEventSetSnoozeSucceeded(ps_id, token);
}

/*******************************************************************************
 * implements BaseAudioPlayerListener
 ******************************************************************************/
void AlertsAgent::mediaStateChanged(NuguCapability::AudioPlayerState state, const std::string& dialog_id)
{
    if (state != AudioPlayerState::FINISHED)
        return;

    if (current == nullptr) {
        nugu_error("current is null");
        return;
    }

    if (audioplayer->isTTSPlayer()) {
        nugu_info("TTS play finished. play default alarm sound");

        if (current->duration_timer_src != 0)
            manager->removeTimeout(current->duration_timer_src);

        current->duration_timer_src = manager->addDurationTimeout(current->duration_secs, current->token);

        if (alerts_listener && current)
            alerts_listener->onFilePlayRequest(current->token, current->type_str, current->rsrc_type);
    } else {
        nugu_info("finished!! releasefocus");
        releaseFocus();
    }
}

void AlertsAgent::mediaEventReport(NuguCapability::AudioPlayerEvent event, const std::string& dialog_id)
{
    switch (event) {
    case AudioPlayerEvent::UNDERRUN:
        nugu_warn("AlertAudioPlayer Underrun");
        break;
    case AudioPlayerEvent::LOAD_FAILED:
        nugu_error("AlertAudioPlayer Load Failed");
        break;
    case AudioPlayerEvent::LOAD_DONE:
        nugu_info("AlertAudioPlayer Load Done");
        break;
    case AudioPlayerEvent::INVALID_URL:
        nugu_error("AlertAudioPlayer Invalid URL");
        break;
    default:
        break;
    }
}

void AlertsAgent::durationChanged(int duration)
{
    if (current == nullptr)
        return;

    nugu_dbg("media resource length: %d secs", duration);

    if (duration <= current->duration_secs)
        return;

    nugu_dbg("media resource length is longer than the alarm duration");

    if (current->duration_timer_src != 0)
        manager->removeTimeout(current->duration_timer_src);

    current->duration_timer_src = manager->addDurationTimeout(duration, current->token);
}

/* callback in thread context */
void AlertsAgent::onTimeout(const std::string& token)
{
    nugu_info("timeout! %s", token.c_str());

    if (is_enable == false) {
        nugu_info("AlertsAgent is disabled");
        return;
    }

    AlertItem* item = manager->findItem(token);
    if (!item) {
        nugu_error("can't find the item");
        return;
    }

    if (item->is_ignored) {
        nugu_info("ignore the alert %s", item->token.c_str());
        sendEventAlertIgnored(item->ps_id, { item->token });
        manager->deactivate(item);
        return;
    }

    if (current == nullptr) {
        current = item;

        if (focus_manager) {
            nugu_info("requestFocus");
            focus_manager->requestFocus(ALERTS_FOCUS_TYPE, CAPABILITY_NAME, this);
        }
    } else {
        /* Keep current focus */
        stopSound("another_alert");

        current = item;
        playSound();
    }
}

/* callback in thread context */
void AlertsAgent::onAssetRequireTimeout(const std::string& token)
{
    nugu_info("asset timeout! %s", token.c_str());

    AlertItem* item = manager->findItem(token);
    if (!item) {
        nugu_error("can't find the item");
        return;
    }

    if (item->rsrc_type == "MUSIC" || item->rsrc_type == "TTS")
        sendEventAlertAssetRequired(item->ps_id, item->token);
}

/* callback in thread context */
void AlertsAgent::onDurationTimeout(const std::string& token)
{
    nugu_info("duration timeout! %s", token.c_str());

    AlertItem* item = manager->findItem(token);
    if (!item) {
        nugu_error("can't find the item");
        return;
    }

    stopSound("duration-done");
}

bool AlertsAgent::addAlert(const Json::Value& item)
{
    return manager->add(item);
}

bool AlertsAgent::removeAlert(const std::string& token)
{
    if (manager->findItem(token) == nullptr)
        return false;

    return manager->removeItem(token);
}

Json::Value AlertsAgent::getAlertList()
{
    return manager->getAlertList();
}

bool AlertsAgent::isAlertPlaying()
{
    if (current)
        return true;

    return false;
}

void AlertsAgent::resetAlerts()
{
    if (current)
        stopSound("resetAlerts");

    current = nullptr;
    manager->reset();
}

int AlertsAgent::getAlertCount()
{
    return manager->getAlertCount();
}

void AlertsAgent::setEnable(bool flag)
{
    is_enable = flag;
}

bool AlertsAgent::isEnable()
{
    return is_enable;
}

void AlertsAgent::unHoldAlarm()
{
    nugu_info("unHoldAlarm execute");
    focus_manager->unholdFocus(ALERTS_FOCUS_TYPE);
}

void AlertsAgent::holdAlarmUntilTextProcess()
{
    nugu_info("holdAlarmUntilTextProcess execute");
    hold_alarm_by_text = true;

    focus_manager->holdFocus(ALERTS_FOCUS_TYPE);
}

static gboolean _emit_in_idle(gpointer userdata)
{
    AlertsAgent* agent = static_cast<AlertsAgent*>(userdata);

    if (agent)
        agent->unHoldAlarm();

    return FALSE;
}

void AlertsAgent::unholdAlarmUntilTextProcess()
{
    nugu_info("unholdAlarmUntilTextProcess execute");

    if (hold_alarm_by_text && alerts_listener)
        g_idle_add(_emit_in_idle, this);

    hold_alarm_by_text = false;
}

void AlertsAgent::playSound()
{
    if (current == nullptr) {
        nugu_error("can't find the current alert");
        return;
    }

    nugu_info("playSound type: %s", current->type_str.c_str());

    sendEventAlertStarted(current->ps_id, current->token);

    if (alerts_listener)
        alerts_listener->onAlertStart(current->token, current->type_str, current->rsrc_type);

    if (!playstackctl_ps_id.empty()) {
        playsync_manager->prepareSync(playstackctl_ps_id, directive_for_sync);
        playsync_manager->startSync(playstackctl_ps_id, getName());
    }

    if (current->type != ALERT_TYPE_SLEEP)
        current->duration_timer_src = manager->addDurationTimeout(current->duration_secs, current->token);

    if (current->type == ALERT_TYPE_TIMER) {
        if (alerts_listener)
            alerts_listener->onFilePlayRequest(current->token, current->type_str, current->rsrc_type);
    } else if (current->type == ALERT_TYPE_ALARM) {
        bool use_file = true;

        if (current->rsrc_type == "MUSIC") {
            if (audioplayer->playMedia())
                use_file = false;
        } else if (current->rsrc_type == "TTS") {
            if (audioplayer->playTTS())
                use_file = false;
        } else if (current->rsrc_type != "INTERNAL") {
            nugu_error("unknown resource type: %s", current->rsrc_type.c_str());
        }

        if (use_file && alerts_listener)
            alerts_listener->onFilePlayRequest(current->token, current->type_str, current->rsrc_type);
    } else if (current->type == ALERT_TYPE_SLEEP) {
        stopSound("playSound, SLEEP");
        playsync_manager->clear();
        focus_manager->stopAllFocus();
    } else {
        nugu_error("Unknown alarm type");
    }
}

void AlertsAgent::stopSound(const std::string& reason, bool isWakeup)
{
    nugu_info("reason: %s", reason.c_str());

    if (current == nullptr) {
        nugu_dbg("already stopped");
        return;
    }

    AlertItem* item = current;
    current = nullptr;

    sendEventAlertStopped(item->ps_id, item->token);

    if (alerts_listener)
        alerts_listener->onAlertStop(item->token, item->type_str);

    if (!playstackctl_ps_id.empty()) {
        //check whether the alarm is stopped by asr or pressing button
        if (isWakeup) {
            playsync_manager->postPoneRelease();
        }

        playsync_manager->releaseSync(playstackctl_ps_id, getName());
    }

    manager->done(item);

    if (item->type != ALERT_TYPE_SLEEP && reason != "another_alert")
        releaseFocus();

    if (item->type == ALERT_TYPE_ALARM) {
        if (item->is_repeat == false) {
            nugu_dbg("no repeat alert. deactivated");
            manager->deactivate(item);
        }
    } else {
        removeAlert(item->token);
    }

    manager->scheduling();
    manager->dump();
}

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

#include <string.h>

#include <base/nugu_log.h>

#include "alerts_agent.hh"

static const char* CAPABILITY_NAME = "Alerts";
static const char* CAPABILITY_VERSION = "1.1";

AlertsAgent::AlertsAgent()
    : Capability(CAPABILITY_NAME, CAPABILITY_VERSION)
{
}

AlertsAgent::~AlertsAgent()
{
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
    addReferrerEvents("SetAlertAssetSucceeded", "DeliveryAlertAsset");
    addReferrerEvents("SetAlertAssetFailed", "DeliveryAlertAsset");
    addReferrerEvents("SetSnoozeSucceeded", "SetSnooze");
    addReferrerEvents("SetSnoozeFailed", "SetSnooze");

    initialized = true;
}

void AlertsAgent::deInitialize()
{
    initialized = false;
}

void AlertsAgent::setContextInformation(const std::string& ctx)
{
    context_info = ctx;
}

void AlertsAgent::parsingDirective(const char* dname, const char* message)
{
    nugu_dbg("message: %s", message);

    if (!strcmp(dname, "SetAlert"))
        parsingSetAlert(message);
    else if (!strcmp(dname, "DeleteAlerts"))
        parsingDeleteAlerts(message);
    else if (!strcmp(dname, "DeliveryAlertAsset"))
        parsingDeliveryAlertAsset(message);
    else if (!strcmp(dname, "SetSnooze"))
        parsingSetSnooze(message);
    else
        nugu_warn("%s[%s] is not support %s directive", getName().c_str(), getVersion().c_str(), dname);
}

void AlertsAgent::setCapabilityListener(ICapabilityListener* clistener)
{
    if (clistener)
        alerts_listener = dynamic_cast<IAlertsListener*>(clistener);
}

void AlertsAgent::updateInfoForContext(Json::Value& ctx)
{
    if (!context_info.size())
        return;

    Json::Value root;
    Json::Reader reader;

    if (!reader.parse(context_info, root)) {
        nugu_error("parsing error");
        return;
    }
    root["version"] = getVersion();

    ctx[getName()] = root;
}

void AlertsAgent::sendEventSetAlertSucceeded(const std::string& ps_id, const std::string& token)
{
    sendEventCommon("SetAlertSucceeded", ps_id, token);
}

void AlertsAgent::sendEventSetAlertFailed(const std::string& ps_id, const std::string& token, const std::string& error)
{
    // error: ASSET_NOT_EXSIT, NETWORK_ERROR, UNKNOWN_ERROR
    sendEventCommon("SetAlertFailed", ps_id, token, error);
}

void AlertsAgent::sendEventDeleteAlertsSucceeded(const std::string& ps_id, std::list<std::string> tokens)
{
    sendEventCommon("DeleteAlertsSucceeded", ps_id, std::move(tokens));
}

void AlertsAgent::sendEventDeleteAlertsFailed(const std::string& ps_id, std::list<std::string> tokens)
{
    sendEventCommon("DeleteAlertsFailed", ps_id, std::move(tokens));
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

void AlertsAgent::sendEventAlertFailed(const std::string& ps_id, const std::string& token)
{
    sendEventCommon("AlertFailed", ps_id, token);
}

void AlertsAgent::sendEventAlertIgnored(const std::string& ps_id, const std::string& token)
{
    sendEventCommon("AlertIgnored", ps_id, token);
}

void AlertsAgent::sendEventAlertStopped(const std::string& ps_id, const std::string& token)
{
    sendEventCommon("AlertStopped", ps_id, token);
}

void AlertsAgent::sendEventAlertAssetRequired(const std::string& ps_id, const std::string& token)
{
    sendEventCommon("AlertAssetRequired", ps_id, token);
}

void AlertsAgent::sendEventAlertEnteredForeground(const std::string& ps_id, const std::string& token)
{
    sendEventCommon("AlertEnteredForeground", ps_id, token);
}

void AlertsAgent::sendEventAlertEnteredBackground(const std::string& ps_id, const std::string& token)
{
    sendEventCommon("AlertEnteredBackground", ps_id, token);
}

void AlertsAgent::sendEventCommon(const std::string& ename, const std::string& ps_id, const std::string& token, const std::string& error)
{
    std::string payload = "";
    Json::Value root;
    Json::StyledWriter writer;

    root["playServiceId"] = ps_id;
    root["token"] = token;
    if (!error.empty()) {
        root["errorCode"] = error;
    }
    payload = writer.write(root);

    sendEvent(ename, getContextInfo(), payload);
}

void AlertsAgent::sendEventCommon(const std::string& ename, const std::string& ps_id, std::list<std::string> tokens)
{
    std::string payload = "";
    Json::Value root;
    Json::StyledWriter writer;

    root["playServiceId"] = ps_id;
    for (const auto& token : tokens)
        root["tokens"].append(token);
    payload = writer.write(root);

    sendEvent(ename, getContextInfo(), payload);
}

void AlertsAgent::parsingSetAlert(const char* message)
{
    if (alerts_listener)
        alerts_listener->setAlert(message);
}

void AlertsAgent::parsingDeleteAlerts(const char* message)
{
    if (alerts_listener)
        alerts_listener->deleteAlerts(message);
}

void AlertsAgent::parsingDeliveryAlertAsset(const char* message)
{
    if (alerts_listener)
        alerts_listener->deliveryAlertAsset(message);
}

void AlertsAgent::parsingSetSnooze(const char* message)
{
    if (alerts_listener)
        alerts_listener->setSnooze(message);
}

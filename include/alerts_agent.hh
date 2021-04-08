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

using namespace NuguClientKit;

class IAlertsListener : public ICapabilityListener {
public:
    virtual ~IAlertsListener() = default;

    virtual void setAlert(const std::string& payload) = 0;
    virtual void deleteAlerts(const std::string& payload) = 0;
    virtual void deliveryAlertAsset(const std::string& payload) = 0;
    virtual void setSnooze(const std::string& payload) = 0;
};

class AlertsAgent : public Capability {
public:
    AlertsAgent();
    virtual ~AlertsAgent();

    void initialize() override;
    void deInitialize() override;

    void setContextInformation(const std::string& ctx);

    void setCapabilityListener(ICapabilityListener* clistener) override;
    void parsingDirective(const char* dname, const char* message) override;
    void updateInfoForContext(Json::Value& ctx) override;

    void sendEventSetAlertSucceeded(const std::string& ps_id, const std::string& token);
    void sendEventSetAlertFailed(const std::string& ps_id, const std::string& token, const std::string& error);
    void sendEventDeleteAlertsSucceeded(const std::string& ps_id, std::list<std::string> tokens);
    void sendEventDeleteAlertsFailed(const std::string& ps_id, std::list<std::string> tokens);
    void sendEventSetSnoozeSucceeded(const std::string& ps_id, const std::string& token);
    void sendEventSetSnoozeFailed(const std::string& ps_id, const std::string& token);
    void sendEventAlertStarted(const std::string& ps_id, const std::string& token);
    void sendEventAlertFailed(const std::string& ps_id, const std::string& token);
    void sendEventAlertIgnored(const std::string& ps_id, const std::string& token);
    void sendEventAlertStopped(const std::string& ps_id, const std::string& token);
    void sendEventAlertEnteredForeground(const std::string& ps_id, const std::string& token);
    void sendEventAlertEnteredBackground(const std::string& ps_id, const std::string& token);
    void sendEventAlertAssetRequired(const std::string& ps_id, const std::string& token);

private:
    void sendEventCommon(const std::string& ename, const std::string& ps_id,
        const std::string& token, const std::string& error = "");
    void sendEventCommon(const std::string& ename, const std::string& ps_id,
        std::list<std::string> tokens);

    void parsingSetAlert(const char* message);
    void parsingDeleteAlerts(const char* message);
    void parsingDeliveryAlertAsset(const char* message);
    void parsingSetSnooze(const char* message);

    IAlertsListener* alerts_listener = nullptr;
    std::string context_info;
};

#endif /* __NUGU_ALERTS_AGENT_H__ */

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

#ifndef __NUGU_DELEGATION_AGENT_H__
#define __NUGU_DELEGATION_AGENT_H__

#include <clientkit/capability.hh>

using namespace NuguClientKit;

class IDelegationListener : public ICapabilityListener {
public:
    virtual ~IDelegationListener() = default;

    virtual void delegate(const std::string& app_id, const std::string& ps_id, const std::string& data) = 0;
    virtual bool requestContext(std::string& ps_id, std::string& data) = 0;
};

class DelegationAgent final : public Capability {
public:
    DelegationAgent();
    virtual ~DelegationAgent() = default;

    void setCapabilityListener(ICapabilityListener* clistener) override;
    void updateInfoForContext(Json::Value& ctx) override;
    void parsingDirective(const char* dname, const char* message) override;

    bool request(const std::string& ps_id, const std::string& data);

private:
    void parsingDelegate(const char* message);

    bool sendEventRequest(const std::string& ps_id, const std::string& data, EventResultCallback cb = nullptr);

    IDelegationListener* delegation_listener = nullptr;
};

#endif /* __NUGU_DELEGATION_AGENT_H__ */

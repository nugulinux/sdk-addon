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

#include "delegation_agent.hh"

static const char* CAPABILITY_NAME = "Delegation";
static const char* CAPABILITY_VERSION = "1.1";

DelegationAgent::DelegationAgent()
    : Capability(CAPABILITY_NAME, CAPABILITY_VERSION)
{
}

void DelegationAgent::setCapabilityListener(ICapabilityListener* clistener)
{
    if (clistener)
        delegation_listener = dynamic_cast<IDelegationListener*>(clistener);
}

void DelegationAgent::updateInfoForContext(Json::Value& ctx)
{
    Json::Value delegation;

    delegation["version"] = getVersion();

    if (delegation_listener) {
        std::string ps_id;
        std::string data;

        if (delegation_listener->requestContext(ps_id, data)) {
            Json::Value root;
            Json::Reader reader;

            if (!reader.parse(data, root)) {
                nugu_error("The required parameters are not set");
                return;
            }

            if (ps_id.size()) {
                delegation["playServiceId"] = ps_id;
                delegation["data"] = root;
            }
        }
    }

    ctx[getName()] = delegation;
}

void DelegationAgent::parsingDirective(const char* dname, const char* message)
{
    nugu_dbg("message: %s", message);

    if (!strcmp(dname, "Delegate"))
        parsingDelegate(message);
    else
        nugu_warn("%s[%s] is not support %s directive", getName().c_str(), getVersion().c_str(), dname);
}

void DelegationAgent::parsingDelegate(const char* message)
{
    Json::Value root;
    Json::Reader reader;

    if (!reader.parse(message, root)) {
        nugu_error("parsing error");
        return;
    }

    std::string nugu_id = root["appId"].asString();
    std::string ps_id = root["playServiceId"].asString();
    Json::Value data = root["data"];

    if (nugu_id.size() == 0 || ps_id.size() == 0 || data.isNull()) {
        nugu_error("The Manatory data are insufficient to process");
        return;
    }

    if (delegation_listener) {
        Json::StyledWriter writer;
        delegation_listener->delegate(nugu_id, ps_id, writer.write(data));
    }
}

bool DelegationAgent::request(const std::string& ps_id, const std::string& data)
{
    return sendEventRequest(ps_id, data);
}

bool DelegationAgent::sendEventRequest(const std::string& ps_id, const std::string& data, EventResultCallback cb)
{
    std::string ename = "Request";
    std::string payload;

    Json::Value root;
    Json::Value container;
    Json::Reader reader;
    Json::StyledWriter writer;

    if (!reader.parse(data, root)) {
        nugu_error("parsing error");
        return false;
    }

    container["playServiceId"] = ps_id;
    container["data"] = root;

    payload = writer.write(container);

    sendEvent(ename, getContextInfo(), payload, std::move(cb));
    return true;
}

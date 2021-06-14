/*
 * Copyright (c) 2021 SK Telecom Co., Ltd. All rights reserved.
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

#include "nugu_auto_agent.hh"

static const char* CAPABILITY_NAME = "NuguAuto";
static const char* CAPABILITY_VERSION = "1.0";

NuguAutoAgent::NuguAutoAgent()
    : Capability(CAPABILITY_NAME, CAPABILITY_VERSION)
{
    permission_map[PermissionType::CallHistory] = PermissionStatus::NotSupported;
    permission_map[PermissionType::PhoneBook] = PermissionStatus::NotSupported;
    permission_map[PermissionType::Location] = PermissionStatus::NotSupported;
    permission_map[PermissionType::Microphone] = PermissionStatus::NotSupported;
    permission_map[PermissionType::Call] = PermissionStatus::NotSupported;
    permission_map[PermissionType::Message] = PermissionStatus::NotSupported;
}

void NuguAutoAgent::setPermission(PermissionType type, PermissionStatus status)
{
    permission_map[type] = status;
}

std::string NuguAutoAgent::getPermissionTypeString(PermissionType type)
{
    std::string type_str;

    switch (type) {
    case PermissionType::CallHistory:
        type_str = "CallHistory";
        break;
    case PermissionType::PhoneBook:
        type_str = "PhoneBook";
        break;
    case PermissionType::Location:
        type_str = "Location";
        break;
    case PermissionType::Microphone:
        type_str = "Microphone";
        break;
    case PermissionType::Call:
        type_str = "Call";
        break;
    case PermissionType::Message:
        type_str = "Message";
        break;
    }

    return type_str;
}

std::string NuguAutoAgent::getPermissionStatusString(PermissionStatus status)
{
    std::string status_str;

    switch (status) {
    case PermissionStatus::Granted:
        status_str = "Granted";
        break;
    case PermissionStatus::Denied:
        status_str = "Denied";
        break;
    case PermissionStatus::NotSupported:
        status_str = "NotSupported";
        break;
    }

    return status_str;
}

void NuguAutoAgent::setCapabilityListener(ICapabilityListener* clistener)
{
    if (clistener)
        nugu_auto_listener = dynamic_cast<INuguAutoListener*>(clistener);
}

void NuguAutoAgent::updateInfoForContext(Json::Value& ctx)
{
    Json::Value nugu_auto;
    Json::Value permissions;

    nugu_auto["version"] = getVersion();
    nugu_auto["foregroundAppCategory"] = "ASSISTANT";

    int i = 0;
    for (auto iter = permission_map.begin(); iter != permission_map.end(); iter++, i++) {
        permissions[i]["name"] = getPermissionTypeString(iter->first);
        permissions[i]["status"] = getPermissionStatusString(iter->second);
    }

    nugu_auto["applicationPermissions"] = permissions;
    nugu_auto["mediaState"] = "NONE";

    ctx[getName()] = nugu_auto;
}

void NuguAutoAgent::parsingDirective(const char* dname, const char* message)
{
    nugu_dbg("message: %s", message);

    if (!strcmp(dname, "AutoCommand"))
        parsingAutoCommand(message);
    else
        nugu_warn("%s[%s] is not support %s directive", getName().c_str(), getVersion().c_str(), dname);
}

void NuguAutoAgent::parsingAutoCommand(const char* message)
{
    Json::Value root;
    Json::Value data;
    Json::Reader reader;
    Json::StyledWriter writer;
    std::string ps_id;

    if (!reader.parse(message, root)) {
        nugu_error("parsing error");
        return;
    }

    ps_id = root["playServiceId"].asString();
    data = root["data"];

    if (ps_id.empty() || data.isNull()) {
        nugu_error("There is no mandatory data in directive message");
        return;
    }

    sendEventAutoEvent(ps_id, writer.write(data));
}

void NuguAutoAgent::sendEventAutoEvent(const std::string& ps_id, const std::string& data, EventResultCallback cb)
{
    std::string ename = "AutoEvent";
    std::string payload;

    Json::Value root;
    Json::Value container;
    Json::Reader reader;
    Json::StyledWriter writer;

    if (!reader.parse(data, root)) {
        nugu_error("parsing error");
        return;
    }

    container["playServiceId"] = ps_id;
    container["data"] = root;

    payload = writer.write(container);

    sendEvent(ename, getContextInfo(), payload, std::move(cb));
}

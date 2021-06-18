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
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or` implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __NUGU_AUTO_AGENT_H__
#define __NUGU_AUTO_AGENT_H__

#include <clientkit/capability.hh>
#include <map>

using namespace NuguClientKit;

enum class PermissionType {
    CallHistory,
    PhoneBook,
    Location,
    Microphone,
    Call,
    Message
};

enum class PermissionStatus {
    Granted,
    Denied,
    NotSupported
};

enum class OSMediaSession {
    PLAYING,
    STOPPED,
    NONE
};

class INuguAutoListener : public ICapabilityListener {
public:
    virtual ~INuguAutoListener() = default;
};

class NuguAutoAgent final : public Capability {
public:
    NuguAutoAgent();
    virtual ~NuguAutoAgent() = default;

    void setMediaSession(OSMediaSession media_session);
    std::string getMediaSessionString(OSMediaSession media_session);

    void setPermission(PermissionType type, PermissionStatus status);
    std::string getPermissionTypeString(PermissionType type);
    std::string getPermissionStatusString(PermissionStatus status);

    void setCapabilityListener(ICapabilityListener* clistener) override;
    void updateInfoForContext(Json::Value& ctx) override;
    void parsingDirective(const char* dname, const char* message) override;

private:
    void parsingAutoCommand(const char* message);
    void sendEventAutoEvent(const std::string& ps_id, const std::string& data, EventResultCallback cb = nullptr);

    INuguAutoListener* nugu_auto_listener = nullptr;
    std::map<PermissionType, PermissionStatus> permission_map;
    OSMediaSession media_session;
};

#endif /* __NUGU_AUTO_AGENT_H__ */

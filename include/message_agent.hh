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

#ifndef __MESSAGE_AGENT_H__
#define __MESSAGE_AGENT_H__

#include <clientkit/capability.hh>

using namespace NuguClientKit;

class IMessageListener : public ICapabilityListener {
public:
    virtual ~IMessageListener() = default;
};

class MessageAgent final : public Capability {
public:
    MessageAgent();
    virtual ~MessageAgent() = default;

    void setCapabilityListener(ICapabilityListener* clistener) override;
    void updateInfoForContext(Json::Value& ctx) override;

private:
    IMessageListener* message_listener = nullptr;
};

#endif /* __MESSAGE_AGENT_H__ */

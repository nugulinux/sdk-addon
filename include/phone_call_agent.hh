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

#ifndef __PHONE_CALL_AGENT_H__
#define __PHONE_CALL_AGENT_H__

#include <clientkit/capability.hh>

using namespace NuguClientKit;

class IPhoneCallListener : public ICapabilityListener {
public:
    virtual ~IPhoneCallListener() = default;
};

class PhoneCallAgent final : public Capability {
public:
    PhoneCallAgent();
    virtual ~PhoneCallAgent() = default;

    void setCapabilityListener(ICapabilityListener* clistener) override;
    void updateInfoForContext(Json::Value& ctx) override;

private:
    IPhoneCallListener* phonecall_listener = nullptr;
};

#endif /* __PHONE_CALL_AGENT_H__ */

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

#ifndef __NUGU_DEVICE_FEATURE_AGENT_H__
#define __NUGU_DEVICE_FEATURE_AGENT_H__

#include <clientkit/capability.hh>

using namespace NuguClientKit;

class IDeviceFeatureListener : public ICapabilityListener {
public:
    virtual ~IDeviceFeatureListener() = default;

    virtual void requestToGet(const std::string& data) = 0;
    virtual void requestToSet(const std::string& data) = 0;
    virtual void requestToUnSet(const std::string& data) = 0;
};

class DeviceFeatureAgent final : public Capability {
public:
    DeviceFeatureAgent();
    virtual ~DeviceFeatureAgent() = default;

    void initialize() override;
    void deInitialize() override;

    void setContextInformation(const std::string& ctx);

    void parsingDirective(const char* dname, const char* message) override;
    void updateInfoForContext(Json::Value& ctx) override;
    void setCapabilityListener(ICapabilityListener* clistener) override;

    void getSucceeded(const std::string& data);
    void setSucceeded(const std::string& data);
    void setFailed(const std::string& data);
    void unSetSucceeded(const std::string& data);
    void unSetFailed(const std::string& data);

private:
    void sendEventCommon(const std::string& ename, const std::string& data, EventResultCallback cb = nullptr);

    void parsingGet(const char* message);
    void parsingSet(const char* message);
    void parsingUnSet(const char* message);

    IDeviceFeatureListener* device_feature_listener;
    std::string context_info;
};

#endif /* __NUGU_DEVICE_FEATURE_AGENT_H__ */

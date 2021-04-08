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

#include "device_feature_agent.hh"

static const char* CAPABILITY_NAME = "DeviceFeature";
static const char* CAPABILITY_VERSION = "1.2";

DeviceFeatureAgent::DeviceFeatureAgent()
    : Capability(CAPABILITY_NAME, CAPABILITY_VERSION)
    , device_feature_listener(nullptr)
{
}

void DeviceFeatureAgent::initialize()
{
    if (initialized) {
        nugu_warn("It's already initialized.");
        return;
    }

    Capability::initialize();

    addReferrerEvents("GetSucceeded", "Get");
    addReferrerEvents("SetSucceeded", "Set");
    addReferrerEvents("SetFailed", "Set");
    addReferrerEvents("UnSetSucceeded", "UnSet");
    addReferrerEvents("UnSetFailed", "UnSet");

    initialized = true;
}

void DeviceFeatureAgent::deInitialize()
{
    initialized = false;
}

void DeviceFeatureAgent::setContextInformation(const std::string& ctx)
{
    context_info = ctx;
}

void DeviceFeatureAgent::parsingDirective(const char* dname, const char* message)
{
    nugu_dbg("message: %s", message);

    // directive name check
    if (!strcmp(dname, "Get"))
        parsingGet(message);
    else if (!strcmp(dname, "Set"))
        parsingSet(message);
    else if (!strcmp(dname, "UnSet"))
        parsingUnSet(message);
    else
        nugu_warn("%s[%s] is not support %s directive", getName().c_str(), getVersion().c_str(), dname);
}

void DeviceFeatureAgent::updateInfoForContext(Json::Value& ctx)
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

void DeviceFeatureAgent::getSucceeded(const std::string& data)
{
    sendEventCommon("GetSucceeded", data);
}

void DeviceFeatureAgent::setSucceeded(const std::string& data)
{
    sendEventCommon("SetSucceeded", data);
}

void DeviceFeatureAgent::setFailed(const std::string& data)
{
    sendEventCommon("SetFailed", data);
}

void DeviceFeatureAgent::unSetSucceeded(const std::string& data)
{
    sendEventCommon("UnSetSucceeded", data);
}

void DeviceFeatureAgent::unSetFailed(const std::string& data)
{
    sendEventCommon("UnSetFailed", data);
}

void DeviceFeatureAgent::setCapabilityListener(ICapabilityListener* clistener)
{
    if (clistener)
        device_feature_listener = dynamic_cast<IDeviceFeatureListener*>(clistener);
}

void DeviceFeatureAgent::sendEventCommon(const std::string& ename, const std::string& data, EventResultCallback cb)
{
    Json::Reader reader;
    Json::StyledWriter writer;
    Json::Value root;
    std::string payload = "";

    if (!reader.parse(data, root)) {
        nugu_error("parsing error");
        return;
    }

    payload = writer.write(root);

    sendEvent(ename, getContextInfo(), payload, std::move(cb));
}

void DeviceFeatureAgent::parsingGet(const char* message)
{
    if (device_feature_listener)
        device_feature_listener->requestToGet(message);
}

void DeviceFeatureAgent::parsingSet(const char* message)
{
    if (device_feature_listener)
        device_feature_listener->requestToSet(message);
}

void DeviceFeatureAgent::parsingUnSet(const char* message)
{
    if (device_feature_listener)
        device_feature_listener->requestToUnSet(message);
}

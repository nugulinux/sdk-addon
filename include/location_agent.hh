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
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or` implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __NUGU_LOCATION_AGENT_H__
#define __NUGU_LOCATION_AGENT_H__

#include <clientkit/capability.hh>

using namespace NuguClientKit;

typedef struct {
    std::string latitude;
    std::string longitude;
} LocationInfo;

class ILocationListener : public ICapabilityListener {
public:
    virtual ~ILocationListener() = default;

    virtual void requestContext(LocationInfo& location_info) = 0;
};

class LocationAgent final : public Capability {
public:
    LocationAgent();
    virtual ~LocationAgent() = default;

    void setCapabilityListener(ICapabilityListener* clistener) override;
    void updateInfoForContext(Json::Value& ctx) override;

private:
    ILocationListener* location_listener = nullptr;
};

#endif /* __NUGU_LOCATION_AGENT_H__ */

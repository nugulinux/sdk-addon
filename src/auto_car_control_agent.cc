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

#include "auto_car_control_agent.hh"

static const char* CAPABILITY_NAME = "AutoCarControlEAA";
static const char* CAPABILITY_VERSION = "1.0";

AutoCarControlAgent::AutoCarControlAgent()
    : Capability(CAPABILITY_NAME, CAPABILITY_VERSION)
{
}

void AutoCarControlAgent::setCapabilityListener(ICapabilityListener* clistener)
{
    if (clistener)
        auto_car_control_listener = dynamic_cast<IAutoCarControlListener*>(clistener);
}

void AutoCarControlAgent::updateInfoForContext(Json::Value& ctx)
{
    Json::Value auto_car_control;

    auto_car_control["version"] = getVersion();

    ctx[getName()] = auto_car_control;
}

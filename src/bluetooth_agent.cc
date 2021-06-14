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

#include "bluetooth_agent.hh"

static const char* CAPABILITY_NAME = "Bluetooth";
static const char* CAPABILITY_VERSION = "1.1";

BluetoothAgent::BluetoothAgent()
    : Capability(CAPABILITY_NAME, CAPABILITY_VERSION)
{
}

void BluetoothAgent::setCapabilityListener(ICapabilityListener* clistener)
{
    if (clistener)
        bluetooth_listener = dynamic_cast<IBluetoothListener*>(clistener);
}

void BluetoothAgent::updateInfoForContext(Json::Value& ctx)
{
    Json::Value bluetooth;
    Json::Value device;
    Json::Value profiles;
    Json::Value active_device;

    bluetooth["version"] = getVersion();

    device["name"] = "AGL_LINUX_DEV";
    device["status"] = "ON";
    profiles[0]["name"] = "HSP";
    profiles[0]["enabled"] = "TRUE";
    profiles[1]["name"] = "A2DP";
    profiles[1]["enabled"] = "TRUE";
    profiles[2]["name"] = "PBAP";
    profiles[2]["enabled"] = "TRUE";
    profiles[3]["name"] = "MAP";
    profiles[3]["enabled"] = "TRUE";
    profiles[4]["name"] = "PAN";
    profiles[4]["enabled"] = "FALSE";
    device["profiles"] = profiles;

    active_device["id"] = "-1";
    active_device["name"] = "AGL_LINUX_DEV_BT_DEVICE";
    active_device["streaming"] = "UNUSABLE";

    bluetooth["device"] = device;
    bluetooth["activeDevice"] = active_device;

    ctx[getName()] = bluetooth;
}

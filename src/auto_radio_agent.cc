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

#include "auto_radio_agent.hh"

static const char* CAPABILITY_NAME = "AutoRadio";
static const char* CAPABILITY_VERSION = "1.0";

AutoRadioAgent::AutoRadioAgent()
    : Capability(CAPABILITY_NAME, CAPABILITY_VERSION)
{
}

void AutoRadioAgent::setCapabilityListener(ICapabilityListener* clistener)
{
    if (clistener)
        auto_radio_listener = dynamic_cast<IAutoRadioListener*>(clistener);
}

void AutoRadioAgent::updateInfoForContext(Json::Value& ctx)
{
    Json::Value auto_radio;

    auto_radio["version"] = getVersion();

    auto_radio["playerActivity"] = "STOPPED";
    auto_radio["channel"];
    auto_radio["recentlyPlayedChannel"]["type"] = "";
    auto_radio["recentlyPlayedChannel"]["value"] = "";
    auto_radio["supportFrequencyTypes"][0] = "FM";
#if 0
      "AutoRadio":{
         "version":"1.0",
         "playerActivity":"STOPPED",
         "channel":null,
         "recentlyPlayedChannel":{
            "type":"",
            "value":""
         },
         "supportFrequencyTypes":[
            "FM"
         ]
      }
#endif

    ctx[getName()] = auto_radio;
}

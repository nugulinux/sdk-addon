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

#include "nugu_tmap_agent.hh"

static const char* CAPABILITY_NAME = "NuguTmap";
static const char* CAPABILITY_VERSION = "1.0";

NuguTmapAgent::NuguTmapAgent()
    : Capability(CAPABILITY_NAME, CAPABILITY_VERSION)
{
}

void NuguTmapAgent::setCapabilityListener(ICapabilityListener* clistener)
{
    if (clistener)
        nugu_tmap_listener = dynamic_cast<INuguTmapListener*>(clistener);
}

void NuguTmapAgent::updateInfoForContext(Json::Value& ctx)
{
    Json::Value nugu_tmap;

    nugu_tmap["version"] = getVersion();

    ctx[getName()] = nugu_tmap;
}

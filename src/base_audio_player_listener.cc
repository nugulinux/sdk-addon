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

#include "base_audio_player_listener.hh"

/*******************************************************************************
 * implements IAudioPlayerListener
 ******************************************************************************/
void BaseAudioPlayerListener::mediaStateChanged(NuguCapability::AudioPlayerState state,
    const std::string& dialog_id)
{
}

void BaseAudioPlayerListener::mediaEventReport(NuguCapability::AudioPlayerEvent event,
    const std::string& dialog_id)
{
}

void BaseAudioPlayerListener::durationChanged(int duration)
{
}

void BaseAudioPlayerListener::positionChanged(int position)
{
}

void BaseAudioPlayerListener::favoriteChanged(bool favorite, const std::string& dialog_id)
{
}

void BaseAudioPlayerListener::shuffleChanged(bool shuffle, const std::string& dialog_id)
{
}

void BaseAudioPlayerListener::repeatChanged(NuguCapability::RepeatType repeat,
    const std::string& dialog_id)
{
}

void BaseAudioPlayerListener::requestContentCache(const std::string& key,
    const std::string& playurl)
{
}

bool BaseAudioPlayerListener::requestToGetCachedContent(const std::string& key,
    std::string& filepath)
{
    return false;
}

/*******************************************************************************
 * implements IAudioPlayerDisplayListener
 ******************************************************************************/
bool BaseAudioPlayerListener::requestLyricsPageAvailable(const std::string& id, bool& visible)
{
    return false;
}

bool BaseAudioPlayerListener::showLyrics(const std::string& id)
{
    return false;
}

bool BaseAudioPlayerListener::hideLyrics(const std::string& id)
{
    return false;
}

void BaseAudioPlayerListener::updateMetaData(const std::string& id, const std::string& json_payload)
{
}

/*******************************************************************************
 * implements IDisplayListener
 ******************************************************************************/
void BaseAudioPlayerListener::renderDisplay(const std::string& id, const std::string& type,
    const std::string& json_payload, const std::string& dialog_id)
{
}

bool BaseAudioPlayerListener::clearDisplay(const std::string& id,
    bool unconditionally, bool has_next)
{
    return false;
}

void BaseAudioPlayerListener::controlDisplay(const std::string& id,
    NuguCapability::ControlType type, NuguCapability::ControlDirection direction)
{
}

void BaseAudioPlayerListener::updateDisplay(const std::string& id, const std::string& json_payload)
{
}

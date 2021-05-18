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

#include "alerts_manager.hh"

#include <base/nugu_log.h>
#include <string.h>
#include <sys/eventfd.h>
#include <unistd.h>

#include <thread>

#ifndef G_SOURCE_FUNC
#define G_SOURCE_FUNC(f) ((GSourceFunc)(void (*)(void))(f))
#endif

struct timeout_data {
    AlertsManager* manager;
    std::string token;
    AlertItem* item;
};

static void dump_time_t(const char* prefix, time_t timestamp)
{
    struct tm local_tm;
    struct tm gmt_tm;

    localtime_r(&timestamp, &local_tm);
    gmtime_r(&timestamp, &gmt_tm);

    nugu_dbg(" %s(LOCAL) %04d-%02d-%02d %02d:%02d:%02d (%d)", prefix,
        local_tm.tm_year, local_tm.tm_mon, local_tm.tm_mday,
        local_tm.tm_hour, local_tm.tm_min, local_tm.tm_sec, timestamp);
    nugu_dbg(" %s( GMT ) %04d-%02d-%02d %02d:%02d:%02d (%d)", prefix,
        gmt_tm.tm_year, gmt_tm.tm_mon, gmt_tm.tm_mday, gmt_tm.tm_hour,
        gmt_tm.tm_min, gmt_tm.tm_sec, timestamp);
}

static void calculate_timeout(time_t now, AlertItem* item)
{
    nugu_dbg(" - available days: 0x%X", item->wday_bitset);

    if (item->is_repeat == false) {
        item->timeout_secs = item->local_secs - now;
        return;
    }

    struct tm now_tm;
    localtime_r(&now, &now_tm);

    time_t now_local_hms = (now_tm.tm_hour * 3600) + (now_tm.tm_min * 60) + now_tm.tm_sec;

    /* Find the nearest day of the week from today. */
    int min_day = 7;
    for (int i = 0, bit = 1; i <= 7; i++, bit = bit << 1) {
        if ((item->wday_bitset & bit) != bit)
            continue;

        /* Find the nearest day of the week. */
        int k = (i + 7 - now_tm.tm_wday) % 7;

        /* Matches today, but time has passed. (set to next week) */
        if (k == 0 && item->hms_local_secs < now_local_hms)
            k = 7;

        if (k < min_day)
            min_day = k;
    }

    nugu_dbg(" - candidate day: today + %d", min_day);
    int target_local_hms = item->hms_local_secs + (min_day * 86400);

    /* Get today time data using Y-M-D (without H:M:S) */
    struct tm time_data;
    memset(&time_data, 0, sizeof(struct tm));
    time_data.tm_year = now_tm.tm_year;
    time_data.tm_mon = now_tm.tm_mon;
    time_data.tm_mday = now_tm.tm_mday;

    /* Add H:M:S information to Y-M-D timestamp */
    item->timeout_secs = mktime(&time_data) + target_local_hms - now;
}

AlertsManager::AlertsManager()
    : listener(nullptr)
{
    quit_fd = eventfd(0, EFD_CLOEXEC);
    loop_ctx = g_main_context_new();

    day_map = {
        { "MON", DAY_MON },
        { "TUE", DAY_TUE },
        { "WED", DAY_WED },
        { "THU", DAY_THU },
        { "FRI", DAY_FRI },
        { "SAT", DAY_SAT },
        { "SUN", DAY_SUN },
    };

    std::thread t([&] {
        GMainLoop* loop;
        GIOChannel* channel;
        GSource* source;

        loop = g_main_loop_new(loop_ctx, FALSE);

        /* Create event-fd IO watch */
        channel = g_io_channel_unix_new(quit_fd);
        source = g_io_create_watch(channel, G_IO_IN);
        g_source_set_callback(source, G_SOURCE_FUNC(quit_fd_callback), loop, NULL);
        g_source_attach(source, loop_ctx);
        g_source_unref(source);
        g_io_channel_unref(channel);

        nugu_info("start loop");
        g_main_loop_run(loop);
        g_main_loop_unref(loop);
        nugu_info("exit loop");
    });

    int ret = pthread_setname_np(t.native_handle(), "alarm_timer");
    if (ret < 0)
        nugu_error("pthread_setname_np failed");

    t.detach();
}

AlertsManager::~AlertsManager()
{
    uint64_t ev = 1;

    int written = write(quit_fd, &ev, sizeof(ev));
    if (written != sizeof(ev)) {
        nugu_error("write failed");
    }

    for (auto const& iter : token_map) {
        nugu_dbg("delete %s", iter.first.c_str());
        AlertItem* item = iter.second;
        delete item;
    }

    token_map.clear();
}

void AlertsManager::setListener(IAlertsManagerListener* clistener)
{
    listener = clistener;
}

/* callback in thread context */
gboolean AlertsManager::quit_fd_callback(GIOChannel* channel, GIOCondition cond, gpointer userdata)
{
    nugu_info("quit alert GMainLoop!");
    g_main_loop_quit((GMainLoop*)userdata);
    return FALSE;
}

/* callback in thread context */
gboolean AlertsManager::timeout_callback(gpointer userdata)
{
    struct timeout_data* td = (struct timeout_data*)userdata;

    if (td->item)
        td->item->timer_src = 0;

    if (td->manager->listener)
        td->manager->listener->onTimeout(td->token);

    return FALSE;
}

/* callback in thread context */
gboolean AlertsManager::asset_timeout_callback(gpointer userdata)
{
    struct timeout_data* td = (struct timeout_data*)userdata;

    if (td->item)
        td->item->asset_timer_src = 0;

    if (td->manager->listener)
        td->manager->listener->onAssetRequireTimeout(td->token);

    return FALSE;
}

/* callback in thread context */
gboolean AlertsManager::duration_timeout_callback(gpointer userdata)
{
    struct timeout_data* td = (struct timeout_data*)userdata;

    if (td->item)
        td->item->duration_timer_src = 0;

    if (td->manager->listener)
        td->manager->listener->onDurationTimeout(td->token);

    return FALSE;
}

static void _timeout_destroy_notify(gpointer userdata)
{
    struct timeout_data* td = (struct timeout_data*)userdata;

    delete td;
}

guint AlertsManager::addTimeout(time_t secs, const std::string& token)
{
    struct timeout_data* td;

    td = new timeout_data;
    td->manager = this;
    td->token = token;
    td->item = findItem(token);

    GSource* source = g_timeout_source_new_seconds(secs);
    if (!source) {
        delete td;
        return 0;
    }

    g_source_set_callback(source, G_SOURCE_FUNC(timeout_callback), td, _timeout_destroy_notify);

    nugu_info("add timeout %zd secs (%s)", secs, token.c_str());

    guint src_id = g_source_attach(source, loop_ctx);
    g_source_unref(source);

    nugu_dbg(" - timer_src: %d", src_id);

    return src_id;
}

guint AlertsManager::addAssetTimeout(time_t secs, const std::string& token)
{
    struct timeout_data* td;

    td = new timeout_data;
    td->manager = this;
    td->token = token;
    td->item = findItem(token);

    GSource* source = g_timeout_source_new_seconds(secs);
    if (!source) {
        delete td;
        return 0;
    }

    g_source_set_callback(source, G_SOURCE_FUNC(asset_timeout_callback), td, _timeout_destroy_notify);

    nugu_info("add asset timeout %zd secs (%s)", secs, token.c_str());

    guint src_id = g_source_attach(source, loop_ctx);
    g_source_unref(source);

    nugu_dbg(" - asset_timer_src: %d", src_id);

    return src_id;
}

guint AlertsManager::addDurationTimeout(time_t secs, const std::string& token)
{
    struct timeout_data* td;

    td = new timeout_data;
    td->manager = this;
    td->token = token;
    td->item = findItem(token);

    GSource* source = g_timeout_source_new_seconds(secs);
    if (!source) {
        delete td;
        return 0;
    }

    g_source_set_callback(source, G_SOURCE_FUNC(duration_timeout_callback), td, _timeout_destroy_notify);

    nugu_info("add duration timeout %zd secs (%s)", secs, token.c_str());

    guint src_id = g_source_attach(source, loop_ctx);
    g_source_unref(source);

    nugu_dbg(" - duration_timer_src: %d", src_id);

    return src_id;
}

void AlertsManager::removeTimeout(guint timer_src)
{
    if (timer_src == 0)
        return;

    nugu_info("remove timeout(GSource) %d", timer_src);

    GSource* source = g_main_context_find_source_by_id(loop_ctx, timer_src);
    if (source)
        g_source_destroy(source);
}

AlertItem* AlertsManager::generateAlert(const Json::Value& json_item)
{
    Json::FastWriter writer;
    AlertItem* item;
    struct tm time_data;

    item = new AlertItem();
    item->timeout_secs = 0;
    item->json_str = writer.write(json_item);
    item->json = json_item;
    item->wday_bitset = DAY_NONE;
    item->wday_count = 1;
    item->is_ignored = false;
    item->token = json_item["token"].asString();
    item->scheduled_time = json_item["scheduledTime"].asString();
    item->is_activated = json_item["activation"].asBool();
    item->is_repeat = json_item.isMember("repeat");
    item->ps_id = json_item["playServiceId"].asString();
    item->rsrc_type = json_item["alarmResourceType"].asString();
    item->type_str = json_item["alertType"].asString();
    item->audioplayer = nullptr;
    clock_gettime(CLOCK_REALTIME, &item->creation_time);

    if (item->type_str == "ALARM")
        item->type = ALERT_TYPE_ALARM;
    else if (item->type_str == "TIMER")
        item->type = ALERT_TYPE_TIMER;
    else if (item->type_str == "SLEEP")
        item->type = ALERT_TYPE_SLEEP;
    else if (item->type_str == "ACTION")
        item->type = ALERT_TYPE_ACTION;

    if (json_item.isMember("assetRequiredInMilliseconds"))
        item->asset_secs = json_item["assetRequiredInMilliseconds"].asInt() / 1000;
    else
        item->asset_secs = 0;

    if (json_item.isMember("minDurationInSec"))
        item->duration_secs = json_item["minDurationInSec"].asInt();
    else
        item->duration_secs = DEFAULT_ALARM_DURATION_SEC;

    memset(&time_data, 0, sizeof(time_data));

    nugu_info("New alert - %s", item->token.c_str());
    nugu_dbg("- scheduled_time: %s", item->scheduled_time.c_str());
    nugu_dbg("- activation: %d / type: %s / rsrc: %s", item->is_activated,
        item->type_str.c_str(), item->rsrc_type.c_str());

    if (item->is_repeat) {
        std::string type = json_item["repeat"]["type"].asString();
        if (type == "DAILY") {
            /* Everyday */
            item->wday_bitset = DAY_ALL;
            item->wday_count = 7;
        } else {
            /* List of days of the week (save to bitset) */
            Json::Value days = json_item["repeat"]["daysOfWeek"];
            item->wday_count = days.size();
            for (int i = 0; i < item->wday_count; i++)
                item->wday_bitset = item->wday_bitset | day_map.at(days[i].asString());
        }

        /* Repeat alerts only have H:M:S information. (without Y-M-D) */
        /* NOLINTNEXTLINE */
        sscanf(item->scheduled_time.c_str(), "%d:%d:%d",
            &time_data.tm_hour, &time_data.tm_min,
            &time_data.tm_sec);
    } else {
        /* NOLINTNEXTLINE */
        sscanf(item->scheduled_time.c_str(), "%d-%d-%dT%d:%d:%d",
            &time_data.tm_year, &time_data.tm_mon,
            &time_data.tm_mday, &time_data.tm_hour,
            &time_data.tm_min, &time_data.tm_sec);

        /* year start from 1900 */
        time_data.tm_year -= 1900;

        /* month of year (0 - 11) */
        time_data.tm_mon -= 1;

        /* set specific time
         * Some device do not support the timelocal() API.
         * So, the KST(+9) time is manually calculated */
        item->local_secs = timegm(&time_data) - 9 * 3600;
        dump_time_t("- ", item->local_secs);

        localtime_r(&item->local_secs, &time_data);
        item->wday_bitset = (1 << time_data.tm_wday);
    }

    nugu_dbg("- Repeat days: 0x%X / count: %d", item->wday_bitset, item->wday_count);

    /* Local time Hour:Min:Sec to seconds */
    item->hms_local_secs = (time_data.tm_hour * 3600) + (time_data.tm_min * 60) + time_data.tm_sec;

    nugu_dbg("- (LOCAL) %02d:%02d:%02d (%d)", time_data.tm_hour,
        time_data.tm_min, time_data.tm_sec, item->hms_local_secs);

    return item;
}

static bool _compare_creation_time_func(AlertItem* a, AlertItem* b)
{
    if (a->creation_time.tv_sec != b->creation_time.tv_sec)
        return a->creation_time.tv_sec < b->creation_time.tv_sec;

    return a->creation_time.tv_nsec < b->creation_time.tv_nsec;
}

void AlertsManager::scheduling(time_t base_timestamp)
{
    struct tm now_tm;

    if (base_timestamp == 0)
        base_timestamp = time(NULL);

    localtime_r(&base_timestamp, &now_tm);

    nugu_info("Scheduling! base %zd", base_timestamp);
    dump_time_t("- NOW ", base_timestamp);

    std::vector<AlertItem*> sorted_list;
    for (auto const& iter : token_map)
        sorted_list.push_back(iter.second);

    /* sort alert item by creation order */
    std::sort(sorted_list.begin(), sorted_list.end(), _compare_creation_time_func);

    AlertItem* prev_item = nullptr;

    for (auto const& iter : sorted_list) {
        AlertItem* item = iter;
        time_t secs;

        nugu_dbg("token: %s", item->token.c_str());
        nugu_dbg("- activated: %d", item->is_activated);

        item->ignored = false;

        if (item->is_activated == false)
            continue;

        if (item->snooze_secs) {
            if (item->timer_src != 0) {
                nugu_dbg("- already calculated %d snooze secs (src=%d)",
                    item->snooze_secs, item->timer_src);
                prev_item = item;
                continue;
            }

            secs = item->snooze_secs;
            nugu_dbg("- use snooze %d secs", secs);
        } else {
            if (item->timeout_secs != 0) {
                nugu_dbg("- already calculated %d secs (src=%d)",
                    item->timeout_secs, item->timer_src);
                prev_item = item;
                continue;
            }

            calculate_timeout(base_timestamp, item);
            dump_time_t("- candidate ", item->timeout_secs + base_timestamp);

            /* Deactivate an alarm that has already timed out. (e.g. reboot) */
            if (item->timeout_secs < 0) {
                if (item->is_activated)
                    deactivate(item);

                continue;
            }

            secs = item->timeout_secs;
        }

        if (item->asset_secs > 0) {
            time_t asset_secs = secs - item->asset_secs;
            if (asset_secs < 0)
                asset_secs = 1;

            item->asset_timer_src = addAssetTimeout(asset_secs, item->token);
        }

        item->timer_src = addTimeout(secs, item->token);

        item->secs = base_timestamp + secs;

        if (prev_item) {
            if (item->secs == prev_item->secs) {
                nugu_info("- set ignored flag to %s", prev_item->token.c_str());
                prev_item->is_ignored = true;
            }
        }

        prev_item = item;
    }
}

bool AlertsManager::processDuplication(const AlertItem* target)
{
    std::vector<AlertItem*> deactivate_list;

    /* Duplicate registration is allowed for deactivated alarms. */
    if (target->is_activated == false)
        return false;

    /**
     * Alert repeat types
     * A) Everyday (Repeat 7 days)
     * B) Weekday (Repeat 5 days: Mon ~ Fri)
     * C) Weekend (Repeat 2 days: Sat and Sun)
     * D) Repeat only 1 day of the week
     * E) No repeat
     */
    for (auto const& iter : token_map) {
        if (target->token == iter.first)
            continue;

        AlertItem* existing = iter.second;

        /* Duplicate registration is allowed for deactivated alarms. */
        if (existing->is_activated == false)
            continue;

        /* Required time(H:M:S) is different */
        if (existing->hms_local_secs != target->hms_local_secs)
            continue;

        /* No duplicate days of the week */
        if ((existing->wday_bitset & target->wday_bitset) == 0)
            continue;

        /* Allow duplicated time alert for different type */
        if (target->type != existing->type)
            continue;

        if (existing->is_repeat) {
            /* Existing alert: A~D, New alert: E */
            if (target->is_repeat == false) {
                nugu_warn("duplicated - same time with reapeating alarm (%s)",
                    existing->token.c_str());
                return true;
            }

            /* Repeated days of the week are exactly the same. */
            if (existing->wday_bitset == target->wday_bitset) {
                nugu_warn("duplicated - repeat days 0x%02X (%s)",
                    existing->wday_bitset, existing->token.c_str());
                return true;
            }

            /* Existing: A~D, New: B~D */
            if (target->wday_count < existing->wday_count) {
                nugu_warn("repeat day(0x%02X) is smaller than 0x%02X (%s)",
                    target->wday_bitset, existing->wday_bitset, existing->token.c_str());
                return true;
            }

            /* The repeat date range of the new alert item is larger than
             * the existing alerts. The existing alerts should be deactivate */
            deactivate_list.push_back(existing);
        } else {
            /* Existing alert: E, New alert: E */
            if (target->is_repeat == false) {
                if (existing->local_secs != target->local_secs)
                    continue;

                nugu_warn("duplicated - exactly same time %s (%s)",
                    existing->scheduled_time.c_str(),
                    existing->token.c_str());
                return true;
            }

            /* Existing alert: E, New alert: A~D
             * The existing alarm should be deactivate */
            deactivate_list.push_back(existing);
        }
    }

    for (auto const& iter : deactivate_list)
        deactivate(iter);

    return false;
}

bool AlertsManager::addItem(AlertItem* item)
{
    if (!item)
        return false;

    if (findItem(item->token) != nullptr) {
        nugu_error("%s token already exist", item->token.c_str());
        return false;
    }

    if (item->type != ALERT_TYPE_ALARM) {
        /* Remove existing TIMER/SLEEP */
        for (auto const& iter : token_map) {
            AlertItem* existing = iter.second;
            if (existing->type != item->type)
                continue;

            removeItem(existing->token);
            break;
        }
    }

    token_map[item->token] = item;

    return true;
}

bool AlertsManager::add(const char* item)
{
    Json::Value root;
    Json::Reader reader;

    if (!reader.parse(item, root)) {
        nugu_error("parsing error");
        return false;
    }

    return add(root);
}

bool AlertsManager::add(const Json::Value& item)
{
    AlertItem* alert = generateAlert(item);
    if (!alert)
        return false;

    if (findItem(alert->token) != NULL) {
        nugu_error("failed! same token");
        delete alert;
        return false;
    }

    bool ret = processDuplication(alert);
    if (ret == true) {
        nugu_error("failed! not allowed");
        delete alert;
        return false;
    }

    if (addItem(alert) == false) {
        delete alert;
        return false;
    }

    scheduling();

    return true;
}

bool AlertsManager::removeItem(const std::string& token)
{
    AlertItem* item = findItem(token);
    if (!item)
        return false;

    nugu_info("remove %s", token.c_str());

    token_map.erase(token);

    if (item->is_activated)
        deactivate(item);

    if (item->audioplayer) {
        nugu_dbg("remove pending audioplayer");
        item->audioplayer->deInitialize();
        delete item->audioplayer;
        item->audioplayer = nullptr;
    }

    delete item;

    return true;
}

AlertItem* AlertsManager::findItem(const std::string& token)
{
    try {
        return token_map.at(token);
    } catch (const std::out_of_range& e) {
        return nullptr;
    }
}

void AlertsManager::reset()
{
    nugu_info("reset all alerts");

    for (auto const& iter : token_map) {
        nugu_dbg("delete %s", iter.first.c_str());
        AlertItem* item = iter.second;
        if (item->is_activated)
            deactivate(item);
        delete item;
    }

    token_map.clear();
}

void AlertsManager::dump()
{
    int length = token_map.size();
    int i = 1;

    nugu_dbg("----------");
    for (auto const& iter : token_map) {
        AlertItem* item = iter.second;

        nugu_info("[%d/%d] %s", i, length, item->token.c_str());
        nugu_dbg(" - %s", item->json_str.c_str());
        nugu_dbg(" - timer src: %d (%zd secs, snooze %d secs)",
            item->timer_src, item->timeout_secs, item->snooze_secs);
        i++;
    }
    nugu_dbg("----------");
}

void AlertsManager::done(AlertItem* item)
{
    if (!item)
        return;

    nugu_dbg("done %s", item->token.c_str());

    removeTimeout(item->timer_src);
    removeTimeout(item->asset_timer_src);
    removeTimeout(item->duration_timer_src);
    item->timer_src = 0;
    item->asset_timer_src = 0;
    item->duration_timer_src = 0;

    item->timeout_secs = 0;
    item->snooze_secs = 0;
}

void AlertsManager::activate(AlertItem* item)
{
    if (!item)
        return;

    done(item);

    nugu_info("activate %s", item->token.c_str());

    Json::FastWriter writer;

    item->is_activated = true;
    item->json["activation"] = true;
    item->json_str = writer.write(item->json);
    nugu_dbg("json: %s", item->json_str.c_str());
}

void AlertsManager::deactivate(AlertItem* item)
{
    if (!item)
        return;

    done(item);

    nugu_info("deactivate %s", item->token.c_str());

    Json::FastWriter writer;

    item->is_activated = false;
    item->json["activation"] = false;
    item->json_str = writer.write(item->json);
    nugu_dbg("json: %s", item->json_str.c_str());
}

size_t AlertsManager::getAlertCount()
{
    return token_map.size();
}

Json::Value AlertsManager::getAlertList(bool is_context)
{
    Json::Value result;
    Json::FastWriter writer;

    if (!token_map.empty()) {
        int index = 0;
        for (auto const& iter : token_map) {
            AlertItem* item = iter.second;

            if (is_context) {
                /* Skip the deactivated TIMER/SLEEP item to context list */
                if (item->type != ALERT_TYPE_ALARM
                    && item->is_activated == false)
                    continue;
            }

            result[index] = item->json;
            index++;
        }
    }

    return result;
}

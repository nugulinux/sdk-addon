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

#ifndef __ALERTS_MANAGER_H__
#define __ALERTS_MANAGER_H__

#include "alerts_agent.hh"

#include <glib.h>

#define DEFAULT_ALARM_DURATION_SEC 180

/**
 * supported repeat alerts
 *  - Everydat (DAY_ALL)
 *  - Weekday (DAY_WEEKDAY)
 *  - Weekend (DAY_WEEKEND)
 *  - One of the days of week (DAY_SUN ~ DAY_SAT)
 *  - No repeat (specific day)
 */
enum day {
    DAY_NONE = 0x0,
    DAY_SUN = 0x01,
    DAY_MON = 0x02,
    DAY_TUE = 0x04,
    DAY_WED = 0x08,
    DAY_THU = 0x10,
    DAY_FRI = 0x20,
    DAY_SAT = 0x40,
    DAY_WEEKDAY = (DAY_MON | DAY_TUE | DAY_WED | DAY_THU | DAY_FRI),
    DAY_WEEKEND = (DAY_SAT | DAY_SUN),
    DAY_ALL = 0x7F
};

enum alert_type {
    ALERT_TYPE_TIMER, /* support only 1 timer alert */
    ALERT_TYPE_ALARM,
    ALERT_TYPE_SLEEP, /* support only 1 sleep alert */
    ALERT_TYPE_ACTION
};

struct _AlertItem {
    std::string token;
    bool is_activated;
    bool is_repeat;
    std::string json_str; /* original json data */
    Json::Value json;
    std::string scheduled_time;
    std::string ps_id;
    std::string rsrc_type; /* resource type */
    int wday_bitset; /* day-of-week bitset(enum day) */
    int wday_count;
    enum alert_type type;
    std::string type_str;
    time_t hms_local_secs; /* H:M:S to seconds (local time) */
    time_t local_secs; /* Y-M-D H:M:S to seconds (local time) */
    time_t asset_secs; /* secs of assetRequiredInMilliseconds */
    time_t duration_secs;
    time_t snooze_secs;

    bool ignored; /* ignored by another alert item */

    time_t timeout_secs; /* Calculated timestamp to fire */

    guint timer_src; /* GSource id */
    guint asset_timer_src; /* GSource id */
    guint duration_timer_src; /* GSource id */
};

class AlertsManager {
public:
    AlertsManager();
    virtual ~AlertsManager();

    void setListener(IAlertsManagerListener* clistener);

    guint addTimeout(time_t secs, const std::string& token);
    guint addAssetTimeout(time_t secs, const std::string& token);
    guint addDurationTimeout(time_t secs, const std::string& token);
    void removeTimeout(guint timer_src);

    AlertItem* generateAlert(const Json::Value& item);
    bool processDuplication(const AlertItem* target);
    void scheduling(time_t base_timestamp = 0);

    bool addItem(AlertItem* item);
    bool removeItem(const std::string& token);
    AlertItem* findItem(const std::string& token);

    bool add(const char* item);
    bool add(const Json::Value& item);
    void reset();

    void done(AlertItem* item);

    void activate(AlertItem* item);
    void deactivate(AlertItem* item);

    void dump();

    size_t getAlertCount();
    Json::Value getAlertList();

private:
    static gboolean quit_fd_callback(GIOChannel* channel, GIOCondition cond, gpointer userdata);
    static gboolean timeout_callback(gpointer userdata);
    static gboolean asset_timeout_callback(gpointer userdata);
    static gboolean duration_timeout_callback(gpointer userdata);

    IAlertsManagerListener* listener;
    GMainContext* loop_ctx;
    int quit_fd;
    std::map<std::string, int> day_map;
    std::map<std::string, AlertItem*> token_map;
};

#endif

#include "mnu_alarm.hh"
#include "alerts_agent.hh"

#include <base/nugu_log.h>
#include <json/json.h>
#include <unistd.h>

#define DIR1                                         \
    "{"                                              \
    "    \"repeat\" : {"                             \
    "        \"daysOfWeek\" : [ \"THU\", \"SUN\" ]," \
    "        \"type\" : \"WEEKLY\""                  \
    "    },"                                         \
    "    \"scheduledTime\" : \"15:07:00\","          \
    "    \"token\" : \"1\""                          \
    "}"

#define DIR2                                         \
    "{"                                              \
    "    \"repeat\" : {"                             \
    "        \"daysOfWeek\" : [ \"THU\", \"WED\" ]," \
    "        \"type\" : \"WEEKLY\""                  \
    "    },"                                         \
    "    \"scheduledTime\" : \"23:07:00\","          \
    "    \"token\" : \"2\""                          \
    "}"

#define DIR3                                         \
    "{"                                              \
    "    \"repeat\" : {"                             \
    "        \"daysOfWeek\" : [ \"MON\", \"SUN\" ]," \
    "        \"type\" : \"WEEKLY\""                  \
    "    },"                                         \
    "    \"scheduledTime\" : \"15:07:00\","          \
    "    \"token\" : \"3\""                          \
    "}"

#define DIR4                                \
    "{"                                     \
    "    \"repeat\" : {"                    \
    "        \"type\" : \"DAILY\""          \
    "    },"                                \
    "    \"scheduledTime\" : \"23:50:00\"," \
    "    \"token\" : \"4\""                 \
    "}"

#define DIR5                                           \
    "{"                                                \
    "    \"scheduledTime\" : \"2021-04-30T15:07:00\"," \
    "    \"token\" : \"5\""                            \
    "}"

static int parse(const char* message, Json::Value& result)
{
    if (message == NULL)
        return -1;

    Json::Reader reader;
    if (!reader.parse(message, result)) {
        nugu_error("parsing error");
        return -1;
    }

    return 0;
}

static int run_test1(Stackmenu* mm, StackmenuItem* menu, void* user_data)
{
    Json::Value root;
    AlertsAgent agent;

    if (parse(DIR1, root) < 0)
        return -1;

    agent.addAlert(root);

    if (parse(DIR2, root) < 0)
        return -1;

    agent.addAlert(root);

    if (parse(DIR3, root) < 0)
        return -1;

    agent.addAlert(root);

    if (parse(DIR4, root) < 0)
        return -1;

    agent.addAlert(root);

    if (parse(DIR5, root) < 0)
        return -1;

    agent.addAlert(root);

    return 0;
}

#define DIR21                           \
    "{"                                 \
    "\"activation\" : true,"            \
    "\"repeat\" : {"                    \
    "  \"type\" : \"DAILY\""            \
    "},"                                \
    "\"scheduledTime\" : \"23:50:00\"," \
    "\"token\" : \"token-4\""           \
    "}"

#define DIR22                                        \
    "{"                                              \
    "\"activation\" : true,"                         \
    "\"alarmResourceType\" : \"INTERNAL\","          \
    "\"alertType\" : \"ALARM\","                     \
    "\"assets\" : ["                                 \
    "   {"                                           \
    "      \"resource\" : \"BASIC\","                \
    "      \"type\" : \"INTERNAL\""                  \
    "   }"                                           \
    "],"                                             \
    "\"minDurationInSec\" : 180,"                    \
    "\"playServiceId\" : \"nugu.builtin.alarm\","    \
    "\"playStackControl\" : {"                       \
    "   \"playServiceId\" : \"nugu.builtin.alarm\"," \
    "   \"type\" : \"PUSH\""                         \
    "},"                                             \
    "\"scheduledTime\" : \"2021-05-04T10:59:00\","   \
    "\"token\" : \"token-5-auto-deactivate\""        \
    "}"

static int run_test2(Stackmenu* mm, StackmenuItem* menu, void* user_data)
{
    Json::Value root;
    AlertsAgent agent;
    char buf[255];
    struct tm now_tm;
    time_t now;

    now = time(NULL);
    now += 3;

    localtime_r(&now, &now_tm);
    snprintf(buf, sizeof(buf), "%02d:%02d:%02d", now_tm.tm_hour,
        now_tm.tm_min, now_tm.tm_sec);

    if (parse(DIR21, root) < 0)
        return -1;

    root["scheduledTime"] = buf;
    agent.addAlert(root);

    if (parse(DIR22, root) < 0)
        return -1;

    agent.addAlert(root);

    sleep(5);

    return 0;
}

static StackmenuItem menu_alarm[] = {
    { "1", "test1", NULL, run_test1 },
    { "2", "test2", NULL, run_test2 },
    NULL
};

StackmenuItem* alarm_get_stackmenu(void)
{
    return menu_alarm;
}

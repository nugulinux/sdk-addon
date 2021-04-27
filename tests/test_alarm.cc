
#include <glib.h>

#include "alerts_agent.hh"
#include "alerts_manager.hh"

#define REPEAT_EVERY_DAY       \
    "\"repeat\" : {"           \
    "    \"type\" : \"DAILY\"" \
    "},"

#define REPEAT_WEEKDAY                                                      \
    "\"repeat\" : {"                                                        \
    "    \"daysOfWeek\" : [ \"MON\", \"TUE\", \"WED\", \"THU\", \"FRI\" ]," \
    "    \"type\" : \"WEEKLY\""                                             \
    "},"

#define REPEAT_WEEKEND                           \
    "\"repeat\" : {"                             \
    "    \"daysOfWeek\" : [ \"SAT\", \"SUN\" ]," \
    "    \"type\" : \"WEEKLY\""                  \
    "},"

#define REPEAT_FRIDAY                   \
    "\"repeat\" : {"                    \
    "    \"daysOfWeek\" : [ \"FRI\" ]," \
    "    \"type\" : \"WEEKLY\""         \
    "},"

#define DIR1_WEEKDAY                            \
    "{"                                         \
    "    \"activation\" : true," REPEAT_WEEKDAY \
    "    \"scheduledTime\" : \"15:07:05\","     \
    "    \"token\" : \"dir1-weekday\""          \
    "}"

#define DIR1_WEEKDAY_SAME                       \
    "{"                                         \
    "    \"activation\" : true," REPEAT_WEEKDAY \
    "    \"scheduledTime\" : \"15:07:05\","     \
    "    \"token\" : \"dir1-weekday-same\""     \
    "}"

#define DIR1_WEEKEND                            \
    "{"                                         \
    "    \"activation\" : true," REPEAT_WEEKEND \
    "    \"scheduledTime\" : \"15:07:05\","     \
    "    \"token\" : \"dir1-weekend\""          \
    "}"

#define DIR1_NO_REPEAT_TIME                            \
    "{"                                                \
    "    \"activation\" : true,"                       \
    "    \"scheduledTime\" : \"2021-04-30T15:07:05\"," \
    "    \"token\" : \"dir1-no-repeat\""               \
    "}"

#define DIR1_EVERYDAY                             \
    "{"                                           \
    "    \"activation\" : true," REPEAT_EVERY_DAY \
    "    \"scheduledTime\" : \"15:07:05\","       \
    "    \"token\" : \"dir1-everyday\""           \
    "}"

#define DIR1_FRIDAY_REPEAT                     \
    "{"                                        \
    "    \"activation\" : true," REPEAT_FRIDAY \
    "    \"scheduledTime\" : \"15:07:05\","    \
    "    \"token\" : \"dir1-fri-repeat\""      \
    "}"

static void test_simple1(void)
{
    AlertsManager manager;
    const AlertItem* item;

    /* add Weekday(Mon~Fri) repeat */
    g_assert(manager.add(DIR1_WEEKDAY) == true);

    /* add Weekend(Sat~Sun) repeat */
    g_assert(manager.add(DIR1_WEEKEND) == true);

    item = manager.findItem("dir1-weekday");
    g_assert(item != NULL);
    g_assert(item->is_activated == true);

    item = manager.findItem("dir1-weekend");
    g_assert(item != NULL);
    g_assert(item->is_activated == true);

    /* Test with reverse order */
    AlertsManager manager2;

    /* add Weekend(Sat~Sun) repeat */
    g_assert(manager2.add(DIR1_WEEKEND) == true);

    /* add Weekday(Mon~Fri) repeat */
    g_assert(manager2.add(DIR1_WEEKDAY) == true);

    item = manager2.findItem("dir1-weekday");
    g_assert(item != NULL);
    g_assert(item->is_activated == true);

    item = manager2.findItem("dir1-weekend");
    g_assert(item != NULL);
    g_assert(item->is_activated == true);
}

static void test_simple2(void)
{
    AlertsManager manager;
    const AlertItem* item;

    /* add Weekday(Mon~Fri) repeat */
    g_assert(manager.add(DIR1_WEEKDAY) == true);

    /* reject: add Fri repeat */
    g_assert(manager.add(DIR1_FRIDAY_REPEAT) == false);

    item = manager.findItem("dir1-weekday");
    g_assert(item != NULL);
    g_assert(item->is_activated == true);

    /* Test with reverse order */
    AlertsManager manager2;

    /* add Fri repeat */
    g_assert(manager2.add(DIR1_FRIDAY_REPEAT) == true);

    /* add Weekday(Mon~Fri) repeat */
    g_assert(manager2.add(DIR1_WEEKDAY) == true);

    /* Fri alert should be deactivated */
    item = manager2.findItem("dir1-fri-repeat");
    g_assert(item != NULL);
    g_assert(item->is_activated == false);

    item = manager2.findItem("dir1-weekday");
    g_assert(item != NULL);
    g_assert(item->is_activated == true);
}

static void test_simple3(void)
{
    AlertsManager manager;
    const AlertItem* item;

    /* add Weekday(Mon~Fri) repeat */
    g_assert(manager.add(DIR1_WEEKDAY) == true);

    /* add everyday repeat */
    g_assert(manager.add(DIR1_EVERYDAY) == true);

    /* Mon~Fri alert should be deactivated */
    item = manager.findItem("dir1-weekday");
    g_assert(item != NULL);
    g_assert(item->is_activated == false);

    item = manager.findItem("dir1-everyday");
    g_assert(item != NULL);
    g_assert(item->is_activated == true);

    /* Test with reverse order */
    AlertsManager manager2;

    /* add everyday repeat */
    g_assert(manager2.add(DIR1_EVERYDAY) == true);

    /* reject: add Weekday(Mon~Fri) repeat */
    g_assert(manager2.add(DIR1_WEEKDAY) == false);

    item = manager2.findItem("dir1-everyday");
    g_assert(item != NULL);
    g_assert(item->is_activated == true);
}

static void test_simple4(void)
{
    AlertsManager manager;
    const AlertItem* item;

    /* add no-repeat item */
    g_assert(manager.add(DIR1_NO_REPEAT_TIME) == true);

    /* add everyday repeat */
    g_assert(manager.add(DIR1_EVERYDAY) == true);

    /* No-repeat alert should be deactivated */
    item = manager.findItem("dir1-no-repeat");
    g_assert(item != NULL);
    g_assert(item->is_activated == false);

    item = manager.findItem("dir1-everyday");
    g_assert(item != NULL);
    g_assert(item->is_activated == true);

    /* Test with reverse order */
    AlertsManager manager2;

    /* add everyday repeat */
    g_assert(manager2.add(DIR1_EVERYDAY) == true);

    /* reject: add no-repeat item */
    g_assert(manager2.add(DIR1_NO_REPEAT_TIME) == false);

    item = manager.findItem("dir1-everyday");
    g_assert(item != NULL);
    g_assert(item->is_activated == true);
}

static void test_case1(void)
{
    AlertsManager manager;
    const AlertItem* item;

    /* add weekday repeat alert */
    g_assert(manager.add(DIR1_WEEKDAY) == true);

    /* same token */
    g_assert(manager.add(DIR1_WEEKDAY) == false);

    g_assert(manager.findItem("dir1-weekday-same") == NULL);

    item = manager.findItem("dir1-weekday");
    g_assert(item != NULL);
    g_assert(item->is_repeat == true);
    g_assert(item->wday_bitset == DAY_WEEKDAY);
    g_assert(item->hms_local_secs == 15 * 3600 + 7 * 60 + 5);
    g_assert(item->is_activated == true);

    /* reject: same time and same day-of-week repeat */
    g_assert(manager.add(DIR1_WEEKDAY_SAME) == false);

    /* allow: same time and different day-of-week repeat */
    g_assert(manager.add(DIR1_WEEKEND) == true);

    /* reject: same time and no-repeat */
    g_assert(manager.add(DIR1_NO_REPEAT_TIME) == false);
}

static void test_case2(void)
{
    AlertsManager manager;
    const AlertItem* item;

    /* add no-repeat alert */
    g_assert(manager.add(DIR1_NO_REPEAT_TIME) == true);

    item = manager.findItem("dir1-no-repeat");
    g_assert(item != NULL);
    g_assert(item->is_repeat == false);
    g_assert(item->wday_bitset == DAY_FRI);

    /* Alarms that have already timed out must be deactivated */
    g_assert(item->is_activated == false);

    /* allow: same time and repeat */
    g_assert(manager.add(DIR1_WEEKDAY_SAME) == true);

    /* previous item(DIR1_NO_REPEAT_TIME) should be deactivated */
    g_assert(item->is_activated == false);

    item = manager.findItem("dir1-weekday-same");
    g_assert(item != NULL);
    g_assert(item->is_activated == true);

    /* reject: same time and same repeat */
    g_assert(manager.add(DIR1_WEEKDAY) == false);

    /* allow: same time and some days of the week are overlapped */
    g_assert(manager.add(DIR1_EVERYDAY) == true);

    /* previous item(DIR1_SAME_REPEAT) should be deactivated */
    g_assert(item->is_activated == false);

    /* reject: same time and some days of the week are overlapped */
    g_assert(manager.add(DIR1_FRIDAY_REPEAT) == false);
}

int main(int argc, char* argv[])
{
#if !GLIB_CHECK_VERSION(2, 36, 0)
    g_type_init();
#endif

    g_test_init(&argc, &argv, (void*)NULL);
    g_log_set_always_fatal((GLogLevelFlags)G_LOG_FATAL_MASK);

    g_test_add_func("/alarm/simple1", test_simple1);
    g_test_add_func("/alarm/simple2", test_simple2);
    g_test_add_func("/alarm/simple3", test_simple3);
    g_test_add_func("/alarm/simple4", test_simple4);
    g_test_add_func("/alarm/case1", test_case1);
    g_test_add_func("/alarm/case2", test_case2);

    return g_test_run();
}

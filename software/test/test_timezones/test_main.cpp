#include <unity.h>
#include <cstring>
#include <set>
#include <string>
#include "Timezones.h"

void setUp(void) {}
void tearDown(void) {}

void test_timezone_table_not_empty(void) {
    TEST_ASSERT_TRUE(TIMEZONE_COUNT > 10);
}

void test_timezone_entries_valid(void) {
    for (size_t i = 0; i < TIMEZONE_COUNT; i++) {
        TEST_ASSERT_NOT_NULL(TIMEZONE_LIST[i].city);
        TEST_ASSERT_NOT_NULL(TIMEZONE_LIST[i].posix);
        TEST_ASSERT_TRUE(strlen(TIMEZONE_LIST[i].city) > 0);
        TEST_ASSERT_TRUE(strlen(TIMEZONE_LIST[i].posix) > 0);
        // POSIX TZ strings must not contain whitespace or quotes (stored in NVS / HTML <option value>)
        TEST_ASSERT_NULL(strchr(TIMEZONE_LIST[i].posix, ' '));
        TEST_ASSERT_NULL(strchr(TIMEZONE_LIST[i].posix, '"'));
        TEST_ASSERT_NULL(strchr(TIMEZONE_LIST[i].city, '"'));
    }
}

void test_timezone_city_labels_unique(void) {
    std::set<std::string> seen;
    for (size_t i = 0; i < TIMEZONE_COUNT; i++) {
        TEST_ASSERT_TRUE_MESSAGE(seen.insert(TIMEZONE_LIST[i].city).second, TIMEZONE_LIST[i].city);
    }
}

void test_timezone_contains_utc_and_default_region(void) {
    bool utc = false, bratislava = false;
    for (size_t i = 0; i < TIMEZONE_COUNT; i++) {
        if (strcmp(TIMEZONE_LIST[i].posix, "UTC0") == 0) utc = true;
        if (strstr(TIMEZONE_LIST[i].city, "Bratislava")) {
            bratislava = true;
            TEST_ASSERT_EQUAL_STRING("CET-1CEST,M3.5.0,M10.5.0/3", TIMEZONE_LIST[i].posix);
        }
    }
    TEST_ASSERT_TRUE(utc);
    TEST_ASSERT_TRUE(bratislava);
}

int main(int, char **) {
    UNITY_BEGIN();
    RUN_TEST(test_timezone_table_not_empty);
    RUN_TEST(test_timezone_entries_valid);
    RUN_TEST(test_timezone_city_labels_unique);
    RUN_TEST(test_timezone_contains_utc_and_default_region);
    return UNITY_END();
}

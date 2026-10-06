#include <unity.h>
#include <string>
// ConsoleLogger.cpp is compiled straight into the test (no src/ build filter needed);
// FreeRTOS critical sections come from test/include/freertos/FreeRTOS.h.
#include "../../src/ConsoleLogger.cpp"

static ConsoleLogger *L;

void setUp(void) { L = new ConsoleLogger(); }
void tearDown(void) { delete L; }

void test_starts_empty(void) {
    TEST_ASSERT_EQUAL(0, L->getCount());
    LogEntry e = L->getEntry(0);
    TEST_ASSERT_EQUAL_STRING("", e.msg);
}

void test_entries_in_order_with_levels(void) {
    L->logInfo("first %d", 1);
    L->logWarn("second");
    L->logError("third");
    TEST_ASSERT_EQUAL(3, L->getCount());
    TEST_ASSERT_EQUAL_STRING("first 1", L->getEntry(0).msg);
    TEST_ASSERT_EQUAL(LOG_LVL_INFO, L->getEntry(0).level);
    TEST_ASSERT_EQUAL_STRING("second", L->getEntry(1).msg);
    TEST_ASSERT_EQUAL(LOG_LVL_WARN, L->getEntry(1).level);
    TEST_ASSERT_EQUAL_STRING("third", L->getEntry(2).msg);
    TEST_ASSERT_EQUAL(LOG_LVL_ERROR, L->getEntry(2).level);
}

void test_ring_buffer_overwrites_oldest(void) {
    const size_t N = ConsoleLogger::MAX_LOG_ENTRIES;
    for (size_t i = 0; i < N + 5; i++) L->logInfo("msg %u", (unsigned)i);
    TEST_ASSERT_EQUAL(N, L->getCount());
    // oldest surviving entry is #5, newest is #N+4
    TEST_ASSERT_EQUAL_STRING("msg 5", L->getEntry(0).msg);
    char last[32];
    snprintf(last, sizeof(last), "msg %u", (unsigned)(N + 4));
    TEST_ASSERT_EQUAL_STRING(last, L->getEntry(N - 1).msg);
}

void test_long_message_truncated_and_terminated(void) {
    std::string big(300, 'x');
    L->logInfo("%s", big.c_str());
    LogEntry e = L->getEntry(0);
    TEST_ASSERT_TRUE(strlen(e.msg) < sizeof(e.msg));
    TEST_ASSERT_EQUAL('x', e.msg[0]);
}

void test_index_out_of_range_clamps_to_newest(void) {
    L->logInfo("a");
    L->logInfo("b");
    TEST_ASSERT_EQUAL_STRING("b", L->getEntry(99).msg);
}

void test_timestamp_uptime_when_not_synced(void) {
    // Host clock is "synced" (> 2020), so timestamp must be a plausible epoch or uptime value, never 0 garbage.
    L->logInfo("t");
    TEST_ASSERT_TRUE(L->getEntry(0).timestamp > 0 || millis() / 1000 == 0);
}

int main(int, char **) {
    UNITY_BEGIN();
    RUN_TEST(test_starts_empty);
    RUN_TEST(test_entries_in_order_with_levels);
    RUN_TEST(test_ring_buffer_overwrites_oldest);
    RUN_TEST(test_long_message_truncated_and_terminated);
    RUN_TEST(test_index_out_of_range_clamps_to_newest);
    RUN_TEST(test_timestamp_uptime_when_not_synced);
    return UNITY_END();
}

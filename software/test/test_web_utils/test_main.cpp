#include <unity.h>
#include "WebUtils.h"

void setUp(void) {}
void tearDown(void) {}

// ---- urlDecode -------------------------------------------------------------
void test_urldecode_plain(void) {
    TEST_ASSERT_EQUAL_STRING("hello", urlDecode("hello").c_str());
    TEST_ASSERT_EQUAL_STRING("", urlDecode("").c_str());
}

void test_urldecode_plus_and_percent(void) {
    TEST_ASSERT_EQUAL_STRING("a b", urlDecode("a+b").c_str());
    TEST_ASSERT_EQUAL_STRING("a b", urlDecode("a%20b").c_str());
    TEST_ASSERT_EQUAL_STRING("p@ss/w0rd!", urlDecode("p%40ss%2Fw0rd%21").c_str());
    TEST_ASSERT_EQUAL_STRING("p@ss", urlDecode("p%40ss").c_str());  // lowercase/uppercase hex
    TEST_ASSERT_EQUAL_STRING("\n", urlDecode("%0a").c_str());
    TEST_ASSERT_EQUAL_STRING("\n", urlDecode("%0A").c_str());
}

void test_urldecode_malformed_sequences(void) {
    // Malformed escapes are kept verbatim (no data loss, no crash)
    TEST_ASSERT_EQUAL_STRING("%zz", urlDecode("%zz").c_str());
    TEST_ASSERT_EQUAL_STRING("100%", urlDecode("100%").c_str());
    TEST_ASSERT_EQUAL_STRING("ab%4", urlDecode("ab%4").c_str());
}

// ---- encodeFormValue -------------------------------------------------------
void test_encode_unreserved_untouched(void) {
    TEST_ASSERT_EQUAL_STRING("AZaz09-_.~", encodeFormValue("AZaz09-_.~").c_str());
}

void test_encode_space_and_specials(void) {
    TEST_ASSERT_EQUAL_STRING("a+b", encodeFormValue("a b").c_str());
    TEST_ASSERT_EQUAL_STRING("%26%3D%25", encodeFormValue("&=%").c_str());
}

void test_encode_decode_roundtrip(void) {
    const char *samples[] = {"admin", "p@ss w0rd!", "a&b=c%d", "ESP-OOBM_01", "x+y"};
    for (const char *s : samples) {
        TEST_ASSERT_EQUAL_STRING(s, urlDecode(encodeFormValue(s)).c_str());
    }
}

// ---- extractFormArg --------------------------------------------------------
void test_form_urlencoded(void) {
    String body = "usr=admin&pwd=p%40ss+word&baud=115200";
    TEST_ASSERT_EQUAL_STRING("admin", extractFormArg(body, "usr").c_str());
    TEST_ASSERT_EQUAL_STRING("p@ss word", extractFormArg(body, "pwd").c_str());
    TEST_ASSERT_EQUAL_STRING("115200", extractFormArg(body, "baud").c_str());
}

void test_form_key_must_match_whole_name(void) {
    // "pwd" must not match inside "newpwd"
    String body = "newpwd=secret&pwd=real";
    TEST_ASSERT_EQUAL_STRING("real", extractFormArg(body, "pwd").c_str());
}

void test_form_missing_key_returns_empty(void) {
    TEST_ASSERT_EQUAL_STRING("", extractFormArg("a=1&b=2", "c").c_str());
    TEST_ASSERT_EQUAL_STRING("", extractFormArg("", "c").c_str());
}

void test_form_json_body(void) {
    String body = "{\"usr\": \"admin\", \"baud\": 9600}";
    TEST_ASSERT_EQUAL_STRING("admin", extractFormArg(body, "usr").c_str());
    TEST_ASSERT_EQUAL_STRING("9600", extractFormArg(body, "baud").c_str());
}

void test_form_multipart_body(void) {
    String body =
        "--X\r\nContent-Disposition: form-data; name=\"ssid\"\r\n\r\nMyNet\r\n"
        "--X\r\nContent-Disposition: form-data; name=\"pass\"\r\n\r\nsecret\r\n--X--\r\n";
    TEST_ASSERT_EQUAL_STRING("MyNet", extractFormArg(body, "ssid").c_str());
    TEST_ASSERT_EQUAL_STRING("secret", extractFormArg(body, "pass").c_str());
}

int main(int, char **) {
    UNITY_BEGIN();
    RUN_TEST(test_urldecode_plain);
    RUN_TEST(test_urldecode_plus_and_percent);
    RUN_TEST(test_urldecode_malformed_sequences);
    RUN_TEST(test_encode_unreserved_untouched);
    RUN_TEST(test_encode_space_and_specials);
    RUN_TEST(test_encode_decode_roundtrip);
    RUN_TEST(test_form_urlencoded);
    RUN_TEST(test_form_key_must_match_whole_name);
    RUN_TEST(test_form_missing_key_returns_empty);
    RUN_TEST(test_form_json_body);
    RUN_TEST(test_form_multipart_body);
    return UNITY_END();
}

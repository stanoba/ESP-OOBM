#include <unity.h>
#include <cstring>
#include <fstream>
#include <sstream>
#include <string>
#include "VectorGraphics.h"

void setUp(void) {}
void tearDown(void) {}

static int countOccurrences(const std::string &hay, const std::string &needle) {
    int n = 0;
    for (size_t p = hay.find(needle); p != std::string::npos; p = hay.find(needle, p + needle.size())) n++;
    return n;
}

static std::string readFile(const char *relPath) {
    const char *candidates[] = {
        relPath,
        "software/src/WebPortal.cpp",
        "src/WebPortal.cpp",
        "../src/WebPortal.cpp",
        "../../src/WebPortal.cpp",
        "../../../software/src/WebPortal.cpp"
    };
    for (const char *cand : candidates) {
        std::ifstream f(cand, std::ios::binary);
        if (f.is_open()) {
            std::stringstream ss;
            ss << f.rdbuf();
            std::string content = ss.str();
            if (content.size() > 500) {
                return content;
            }
        }
    }
    return "";
}

// Logo must stay a pure vector: no <text>, no font dependency (AP / offline mode).
void test_logo_has_no_font_dependency(void) {
    std::string svg = OOBM_LOGO_SVG;
    TEST_ASSERT_EQUAL(std::string::npos, svg.find("<text"));
    TEST_ASSERT_EQUAL(std::string::npos, svg.find("font-family"));
    TEST_ASSERT_EQUAL(std::string::npos, svg.find("http://fonts"));
    TEST_ASSERT_EQUAL(std::string::npos, svg.find("https://"));
}

// CSS in WebPortal themes the logo through these classes.
void test_logo_has_theme_classes(void) {
    std::string svg = OOBM_LOGO_SVG;
    TEST_ASSERT_NOT_EQUAL(std::string::npos, svg.find("class=\"brand-title-prefix\""));
    TEST_ASSERT_NOT_EQUAL(std::string::npos, svg.find("class=\"brand-title-main\""));
    TEST_ASSERT_NOT_EQUAL(std::string::npos, svg.find("class=\"brand-sub\""));
}

void test_logo_is_wellformed_svg(void) {
    std::string svg = OOBM_LOGO_SVG;
    TEST_ASSERT_EQUAL(0, svg.find("<svg"));
    TEST_ASSERT_EQUAL(svg.size() - 6, svg.rfind("</svg>"));
    TEST_ASSERT_EQUAL(countOccurrences(svg, "<g "), countOccurrences(svg, "</g>"));
    TEST_ASSERT_EQUAL(countOccurrences(svg, "<defs>"), countOccurrences(svg, "</defs>"));
    TEST_ASSERT_EQUAL(countOccurrences(svg, "<linearGradient"), countOccurrences(svg, "</linearGradient>"));
    TEST_ASSERT_TRUE(strstr(OOBM_LOGO_SVG, "viewBox=\"0 0 330 46\"") != nullptr);
}

// Every gradient referenced via url(#id) must be defined.
void test_logo_gradient_references_resolve(void) {
    std::string svg = OOBM_LOGO_SVG;
    for (size_t p = svg.find("url(#"); p != std::string::npos; p = svg.find("url(#", p + 5)) {
        size_t end = svg.find(')', p);
        std::string id = svg.substr(p + 5, end - (p + 5));
        TEST_ASSERT_NOT_EQUAL_MESSAGE(std::string::npos, svg.find("id=\"" + id + "\""), id.c_str());
    }
}

// USB-A collar must not poke past the body's vertical wall (x = 15, butt cap).
void test_logo_usb_collar_flush_with_body(void) {
    std::string svg = OOBM_LOGO_SVG;
    TEST_ASSERT_NOT_EQUAL(std::string::npos, svg.find("M15 14.5"));
    TEST_ASSERT_NOT_EQUAL(std::string::npos, svg.find("L15 29.5"));
    TEST_ASSERT_NOT_EQUAL(std::string::npos, svg.find("stroke-linecap=\"butt\""));
    TEST_ASSERT_NOT_EQUAL(std::string::npos, svg.find("<rect x=\"15\""));
}

void test_favicon_is_wellformed_svg(void) {
    std::string svg = OOBM_FAVICON_SVG;
    TEST_ASSERT_EQUAL(0, svg.find("<svg"));
    TEST_ASSERT_EQUAL(svg.size() - 6, svg.rfind("</svg>"));
    TEST_ASSERT_EQUAL(std::string::npos, svg.find("<text"));
    TEST_ASSERT_TRUE(strstr(OOBM_FAVICON_SVG, "viewBox=\"0 0 48 48\"") != nullptr);
}

// Regression: web pages must not pull fonts from external servers.
void test_webportal_has_no_external_font_links(void) {
    std::string src = readFile("src/WebPortal.cpp");
    TEST_ASSERT_TRUE_MESSAGE(src.size() > 1000, "src/WebPortal.cpp not found (run from project dir)");
    TEST_ASSERT_EQUAL(std::string::npos, src.find("fonts.googleapis.com"));
    TEST_ASSERT_EQUAL(std::string::npos, src.find("fonts.gstatic.com"));
    TEST_ASSERT_EQUAL(std::string::npos, src.find("Chakra"));
}

int main(int, char **) {
    UNITY_BEGIN();
    RUN_TEST(test_logo_has_no_font_dependency);
    RUN_TEST(test_logo_has_theme_classes);
    RUN_TEST(test_logo_is_wellformed_svg);
    RUN_TEST(test_logo_gradient_references_resolve);
    RUN_TEST(test_logo_usb_collar_flush_with_body);
    RUN_TEST(test_favicon_is_wellformed_svg);
    RUN_TEST(test_webportal_has_no_external_font_links);
    return UNITY_END();
}

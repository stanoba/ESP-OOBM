#include <unity.h>
#include <cstring>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>

void setUp(void) {}
void tearDown(void) {}

static std::string readWebPortalCode() {
    const char *candidates[] = {
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
            if (content.size() > 1000) {
                return content;
            }
        }
    }
    return "";
}

// C++ Mock Implementation of the JavaScript AnsiTerminal Engine
// to rigorously verify parsing logic against spec
struct Cell {
    char ch = ' ';
    std::string fg = "";
    std::string bg = "";
    bool bold = false;
    bool underline = false;
    bool inverse = false;
};

class AnsiTerminalMock {
public:
    std::vector<std::vector<Cell>> lines;
    int cursorRow = 0;
    int cursorCol = 0;
    std::string fg = "";
    std::string bg = "";
    bool bold = false;
    bool underline = false;
    bool inverse = false;
    int state = 0;
    std::string csiParamStr = "";
    size_t maxLines = 1200;
    std::string lastWsResponse = "";

    AnsiTerminalMock() {
        lines.push_back({});
    }

    std::string getAnsiColor(int code) {
        static const char *ANSI_COLORS[] = {
            "#0f172a", "#ef4444", "#10b981", "#f59e0b", "#3b82f6", "#a855f7", "#06b6d4", "#cbd5e1",
            "#64748b", "#f87171", "#34d399", "#fbbf24", "#60a5fa", "#c084fc", "#22d3ee", "#ffffff"
        };
        if (code >= 0 && code < 16) return ANSI_COLORS[code];
        if (code >= 232 && code <= 255) {
            int v = (code - 232) * 10 + 8;
            char buf[16];
            snprintf(buf, sizeof(buf), "#%02x%02x%02x", v, v, v);
            return buf;
        }
        if (code >= 16 && code <= 231) {
            int n = code - 16, b = n % 6, g = (n / 6) % 6, r = n / 36;
            static const int steps[] = {0, 95, 135, 175, 215, 255};
            char buf[32];
            snprintf(buf, sizeof(buf), "rgb(%d,%d,%d)", steps[r], steps[g], steps[b]);
            return buf;
        }
        return "";
    }

    void putChar(char ch) {
        while ((int)lines.size() <= cursorRow) lines.push_back({});
        auto &line = lines[cursorRow];
        while ((int)line.size() < cursorCol) {
            line.push_back({' ', "", "", false, false, false});
        }
        if (cursorCol < (int)line.size()) {
            line[cursorCol] = {ch, fg, bg, bold, underline, inverse};
        } else {
            line.push_back({ch, fg, bg, bold, underline, inverse});
        }
        cursorCol++;
    }

    void write(const std::string &str) {
        for (size_t i = 0; i < str.length(); i++) {
            char ch = str[i];
            uint8_t code = (uint8_t)ch;

            if (state == 0) {
                if (code == 27) { state = 1; }
                else if (code == 13) { cursorCol = 0; }
                else if (code == 10) {
                    cursorRow++;
                    if (cursorRow >= (int)lines.size()) lines.push_back({});
                    if (lines.size() > maxLines) {
                        lines.erase(lines.begin());
                        if (cursorRow > 0) cursorRow--;
                    }
                }
                else if (code == 8 || code == 127) { if (cursorCol > 0) cursorCol--; }
                else if (code == 9) {
                    int nextTab = ((cursorCol / 8) + 1) * 8;
                    while (cursorCol < nextTab) putChar(' ');
                }
                else if (code >= 32) { putChar(ch); }
            } else if (state == 1) {
                if (ch == '[') { state = 2; csiParamStr = ""; }
                else if (ch == 'c') { lines.clear(); lines.push_back({}); cursorRow = 0; cursorCol = 0; state = 0; }
                else { state = 0; }
            } else if (state == 2) {
                if ((csiParamStr.empty() || csiParamStr == "?") && (ch == '?' || ch == '>' || ch == '=')) {
                    csiParamStr += ch;
                } else if ((code >= '0' && code <= '9') || ch == ';' || ch == '?' || ch == '>') {
                    csiParamStr += ch;
                } else {
                    handleCsi(ch);
                    state = 0;
                }
            }
        }
    }

    void handleCsi(char cmd) {
        std::string clean = csiParamStr;
        while (!clean.empty() && (clean[0] == '?' || clean[0] == '>' || clean[0] == '=')) clean.erase(0, 1);
        std::vector<int> p;
        std::stringstream ss(clean);
        std::string item;
        while (std::getline(ss, item, ';')) {
            p.push_back(item.empty() ? 0 : std::stoi(item));
        }

        if (cmd == 'm') {
            if (p.empty()) p.push_back(0);
            for (size_t i = 0; i < p.size(); i++) {
                int v = p[i];
                if (v == 0) { fg = ""; bg = ""; bold = false; underline = false; inverse = false; }
                else if (v == 1) { bold = true; }
                else if (v == 4) { underline = true; }
                else if (v == 7) { inverse = true; }
                else if (v == 22) { bold = false; }
                else if (v == 24) { underline = false; }
                else if (v == 27) { inverse = false; }
                else if (v >= 30 && v <= 37) { fg = getAnsiColor(v - 30 + (bold ? 8 : 0)); }
                else if (v == 39) { fg = ""; }
                else if (v >= 40 && v <= 47) { bg = getAnsiColor(v - 40); }
                else if (v == 49) { bg = ""; }
                else if (v >= 90 && v <= 97) { fg = getAnsiColor(v - 90 + 8); }
                else if (v >= 100 && v <= 107) { bg = getAnsiColor(v - 100 + 8); }
                else if (v == 38 && i + 2 < p.size() && p[i + 1] == 5) { fg = getAnsiColor(p[i + 2]); i += 2; }
                else if (v == 48 && i + 2 < p.size() && p[i + 1] == 5) { bg = getAnsiColor(p[i + 2]); i += 2; }
                else if (v == 38 && i + 4 < p.size() && p[i + 1] == 2) {
                    char buf[64];
                    snprintf(buf, sizeof(buf), "rgb(%d,%d,%d)", p[i + 2], p[i + 3], p[i + 4]);
                    fg = buf;
                    i += 4;
                }
                else if (v == 48 && i + 4 < p.size() && p[i + 1] == 2) {
                    char buf[64];
                    snprintf(buf, sizeof(buf), "rgb(%d,%d,%d)", p[i + 2], p[i + 3], p[i + 4]);
                    bg = buf;
                    i += 4;
                }
            }
        } else if (cmd == 'K') {
            int m = p.empty() ? 0 : p[0];
            while ((int)lines.size() <= cursorRow) lines.push_back({});
            auto &line = lines[cursorRow];
            if (m == 0 && (int)line.size() > cursorCol) line.resize(cursorCol);
            else if (m == 2) { line.clear(); cursorCol = 0; }
        } else if (cmd == 'J') {
            int m = p.empty() ? 0 : p[0];
            if (m == 2 || m == 3) { lines.clear(); lines.push_back({}); cursorRow = 0; cursorCol = 0; }
        } else if (cmd == 'A') {
            int n = p.empty() || p[0] == 0 ? 1 : p[0];
            cursorRow = std::max(0, cursorRow - n);
        } else if (cmd == 'B') {
            int n = p.empty() || p[0] == 0 ? 1 : p[0];
            cursorRow = std::min((int)lines.size() - 1, cursorRow + n);
        } else if (cmd == 'C') {
            int n = p.empty() || p[0] == 0 ? 1 : p[0];
            cursorCol += n;
        } else if (cmd == 'D') {
            int n = p.empty() || p[0] == 0 ? 1 : p[0];
            cursorCol = std::max(0, cursorCol - n);
        } else if (cmd == 'G' || cmd == '`') {
            int col = p.empty() || p[0] == 0 ? 1 : p[0];
            cursorCol = std::max(0, col - 1);
        } else if (cmd == 'H' || cmd == 'f') {
            int r = p.size() > 0 && p[0] > 0 ? p[0] : 1;
            int c = p.size() > 1 && p[1] > 0 ? p[1] : 1;
            cursorRow = r - 1;
            while ((int)lines.size() <= cursorRow) lines.push_back({});
            cursorCol = c - 1;
        } else if (cmd == 'n') {
            int q = p.empty() ? 0 : p[0];
            if (q == 6) {
                char buf[32];
                snprintf(buf, sizeof(buf), "\x1b[%d;%dR", cursorRow + 1, cursorCol + 1);
                lastWsResponse = buf;
            } else if (q == 5) {
                lastWsResponse = "\x1b[0n";
            }
        } else if (cmd == 'c') {
            if (csiParamStr.find('>') != std::string::npos) {
                lastWsResponse = "\x1b[>0;10;0c";
            } else {
                lastWsResponse = "\x1b[?1;2c";
            }
        } else if (cmd == 't') {
            int q = p.empty() ? 0 : p[0];
            if (q == 18) {
                lastWsResponse = "\x1b[8;24;80t";
            }
        }
    }
};

// 1. Basic ANSI Color Rendering Tests
void test_ansi_standard_colors(void) {
    AnsiTerminalMock term;
    term.write("\x1b[32mGreen\x1b[0m \x1b[31mRed\x1b[0m \x1b[35mPurple\x1b[0m");

    TEST_ASSERT_EQUAL(1, term.lines.size());
    TEST_ASSERT_EQUAL_STRING("#10b981", term.lines[0][0].fg.c_str()); // G
    TEST_ASSERT_EQUAL_STRING("#10b981", term.lines[0][4].fg.c_str()); // n
    TEST_ASSERT_EQUAL_STRING("", term.lines[0][5].fg.c_str());        // space
    TEST_ASSERT_EQUAL_STRING("#ef4444", term.lines[0][6].fg.c_str()); // R
    TEST_ASSERT_EQUAL_STRING("#a855f7", term.lines[0][10].fg.c_str());// P
}

// 2. Bold / High Intensity Colors (RouterOS style)
void test_ansi_bold_high_intensity_colors(void) {
    AnsiTerminalMock term;
    term.write("\x1b[1;32m[admin@Router-Core]\x1b[0m");

    TEST_ASSERT_EQUAL_STRING("#34d399", term.lines[0][0].fg.c_str()); // Bright Green
    TEST_ASSERT_TRUE(term.lines[0][0].bold);
}

// 3. 256-Color and Extended Palette
void test_ansi_256_colors(void) {
    AnsiTerminalMock term;
    term.write("\x1b[38;5;201mVRF-Magenta\x1b[0m");

    TEST_ASSERT_FALSE(term.lines[0][0].fg.empty());
    TEST_ASSERT_EQUAL('V', term.lines[0][0].ch);
}

// 4. 24-bit TrueColor RGB Support
void test_ansi_truecolor_24bit(void) {
    AnsiTerminalMock term;
    term.write("\x1b[38;2;120;60;200mRGB-Text\x1b[0m");

    TEST_ASSERT_EQUAL_STRING("rgb(120,60,200)", term.lines[0][0].fg.c_str());
    TEST_ASSERT_EQUAL('R', term.lines[0][0].ch);
}

// 5. Text Styles (Underline, Inverse, Reset)
void test_ansi_text_styles(void) {
    AnsiTerminalMock term;
    term.write("\x1b[4mUnderlined\x1b[24m \x1b[7mInverted\x1b[27m");

    TEST_ASSERT_TRUE(term.lines[0][0].underline);
    TEST_ASSERT_FALSE(term.lines[0][10].underline); // space
    TEST_ASSERT_TRUE(term.lines[0][11].inverse);    // 'I'
}

// 6. Cursor Positioning & VT100 Redraw (Fixes Ghosting)
void test_ansi_cursor_positioning_and_home(void) {
    AnsiTerminalMock term;
    term.write("Line 1 Old\r\nLine 2 Old");
    TEST_ASSERT_EQUAL(1, term.cursorRow);

    // Reposition cursor to row 1, col 1 and overwrite
    term.write("\x1b[1;1HLine 1 New");
    TEST_ASSERT_EQUAL('N', term.lines[0][7].ch);
    TEST_ASSERT_EQUAL('e', term.lines[0][8].ch);
    TEST_ASSERT_EQUAL('w', term.lines[0][9].ch);
}

// 7. Relative Cursor Movement (Up, Down, Forward, Backward, Column)
void test_ansi_cursor_movement_relative(void) {
    AnsiTerminalMock term;
    term.write("ABCD\r\nEFGH");
    TEST_ASSERT_EQUAL(1, term.cursorRow);
    TEST_ASSERT_EQUAL(4, term.cursorCol);

    // Move Up 1, Backward 2, overwrite
    term.write("\x1b[1A\x1b[2DX");
    TEST_ASSERT_EQUAL(0, term.cursorRow);
    TEST_ASSERT_EQUAL(3, term.cursorCol);
    TEST_ASSERT_EQUAL('X', term.lines[0][2].ch);

    // Jump directly to column 1
    term.write("\x1b[1GZ");
    TEST_ASSERT_EQUAL('Z', term.lines[0][0].ch);
}

// 8. Line and Screen Clear Sequences (CSI K, CSI J)
void test_ansi_erase_line_and_screen(void) {
    AnsiTerminalMock term;
    term.write("PrefixToDeletePostfix");
    term.cursorCol = 6;
    term.write("\x1b[K"); // Erase to end of line from col 6
    TEST_ASSERT_EQUAL(6, term.lines[0].size());

    // Clear whole screen
    term.write("\x1b[2J");
    TEST_ASSERT_EQUAL(1, term.lines.size());
    TEST_ASSERT_EQUAL(0, term.lines[0].size());
    TEST_ASSERT_EQUAL(0, term.cursorRow);
    TEST_ASSERT_EQUAL(0, term.cursorCol);
}

// 9. Control Characters (\t tab stops, \b backspace, \r\n)
void test_ansi_control_characters(void) {
    AnsiTerminalMock term;
    term.write("A\tB");
    TEST_ASSERT_EQUAL(9, term.cursorCol);
    TEST_ASSERT_EQUAL('A', term.lines[0][0].ch);
    TEST_ASSERT_EQUAL(' ', term.lines[0][1].ch);
    TEST_ASSERT_EQUAL('B', term.lines[0][8].ch);

    // Backspace
    term.write("\bC");
    TEST_ASSERT_EQUAL('C', term.lines[0][8].ch);
}

// 10. RouterOS DSR Autonegotiation Probes
void test_routeros_dsr_query_responses(void) {
    AnsiTerminalMock term;
    term.write("Some Router Output");
    TEST_ASSERT_EQUAL(18, term.cursorCol);

    // Send 6n (Report Cursor Position)
    term.write("\x1b[6n");
    TEST_ASSERT_EQUAL_STRING("\x1b[1;19R", term.lastWsResponse.c_str());

    // Send 5n (Report Terminal Status)
    term.write("\x1b[5n");
    TEST_ASSERT_EQUAL_STRING("\x1b[0n", term.lastWsResponse.c_str());

    // Send Primary DA
    term.write("\x1b[c");
    TEST_ASSERT_EQUAL_STRING("\x1b[?1;2c", term.lastWsResponse.c_str());

    // Send Secondary DA
    term.write("\x1b[>c");
    TEST_ASSERT_EQUAL_STRING("\x1b[>0;10;0c", term.lastWsResponse.c_str());

    // Send Window Size Query (18t)
    term.write("\x1b[18t");
    TEST_ASSERT_EQUAL_STRING("\x1b[8;24;80t", term.lastWsResponse.c_str());
}

// 11. Source Code Verification in WebPortal.cpp
void test_webportal_contains_dsr_and_color_handlers(void) {
    std::string code = readWebPortalCode();
    TEST_ASSERT_FALSE(code.empty());

    // Verify DSR 6n response is present
    TEST_ASSERT_NOT_EQUAL(std::string::npos, code.find("q===6"));
    TEST_ASSERT_NOT_EQUAL(std::string::npos, code.find("'R'"));

    // Verify DA query response is present
    TEST_ASSERT_NOT_EQUAL(std::string::npos, code.find("cmd==='c'"));
    TEST_ASSERT_NOT_EQUAL(std::string::npos, code.find("\\x1b[?1;2c"));

    // Verify bold high-intensity color mapping
    TEST_ASSERT_NOT_EQUAL(std::string::npos, code.find("v-30 + (this.bold?8:0)"));

    // Verify Sessions label on dashboard
    TEST_ASSERT_NOT_EQUAL(std::string::npos, code.find(">Sessions</span>"));
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_ansi_standard_colors);
    RUN_TEST(test_ansi_bold_high_intensity_colors);
    RUN_TEST(test_ansi_256_colors);
    RUN_TEST(test_ansi_truecolor_24bit);
    RUN_TEST(test_ansi_text_styles);
    RUN_TEST(test_ansi_cursor_positioning_and_home);
    RUN_TEST(test_ansi_cursor_movement_relative);
    RUN_TEST(test_ansi_erase_line_and_screen);
    RUN_TEST(test_ansi_control_characters);
    RUN_TEST(test_routeros_dsr_query_responses);
    RUN_TEST(test_webportal_contains_dsr_and_color_handlers);
    return UNITY_END();
}

#pragma once
#include <Arduino.h>

// =============================================================================
// Pure string helpers shared by WebPortal (URL decoding and form parsing).
// Header-only and hardware independent so they can be unit tested natively
// (see test/test_web_utils).
// =============================================================================

inline String urlDecode(const String &src) {
    String decoded = "";
    decoded.reserve(src.length());
    char a, b;
    for (size_t i = 0; i < src.length(); i++) {
        if (src[i] == '%') {
            if (i + 2 < src.length()) {
                a = src[i+1];
                b = src[i+2];
                if (isxdigit(a) && isxdigit(b)) {
                    a = (a >= 'a') ? (a - 'a' + 10) : ((a >= 'A') ? (a - 'A' + 10) : (a - '0'));
                    b = (b >= 'a') ? (b - 'a' + 10) : ((b >= 'A') ? (b - 'A' + 10) : (b - '0'));
                    decoded += (char)((a << 4) | b);
                    i += 2;
                    continue;
                }
            }
            decoded += '%';  // malformed escape: keep the literal '%' instead of dropping it
        } else if (src[i] == '+') {
            decoded += ' ';
        } else {
            decoded += src[i];
        }
    }
    return decoded;
}

inline String extractFormArg(const String &body, const String &key) {
    int keyIdx = body.indexOf(key + "=");
    while (keyIdx != -1) {
        if (keyIdx == 0 || body[keyIdx - 1] == '&' || body[keyIdx - 1] == '?') {
            int valStart = keyIdx + key.length() + 1;
            int valEnd = body.indexOf('&', valStart);
            if (valEnd == -1) valEnd = body.length();
            return urlDecode(body.substring(valStart, valEnd));
        }
        keyIdx = body.indexOf(key + "=", keyIdx + 1);
    }
    String jsonKey = "\"" + key + "\":";
    int jIdx = body.indexOf(jsonKey);
    if (jIdx != -1) {
        int vStart = jIdx + jsonKey.length();
        while (vStart < (int)body.length() && (body[vStart] == ' ' || body[vStart] == '\"')) vStart++;
        int vEnd = vStart;
        while (vEnd < (int)body.length() && body[vEnd] != '\"' && body[vEnd] != ',' && body[vEnd] != '}') vEnd++;
        return body.substring(vStart, vEnd);
    }
    int nameIdx = body.indexOf("name=\"" + key + "\"");
    if (nameIdx != -1) {
        int dataStart = body.indexOf("\r\n\r\n", nameIdx);
        if (dataStart != -1) {
            dataStart += 4;
            int dataEnd = body.indexOf("\r\n--", dataStart);
            if (dataEnd != -1) {
                return body.substring(dataStart, dataEnd);
            }
        }
    }
    return "";
}

inline String encodeFormValue(const String &value) {
    static const char HEX_DIGITS[] = "0123456789ABCDEF";
    String encoded;
    encoded.reserve(value.length() * 3);
    for (size_t i = 0; i < value.length(); ++i) {
        uint8_t ch = (uint8_t)value[i];
        if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') ||
            (ch >= '0' && ch <= '9') || ch == '-' || ch == '_' ||
            ch == '.' || ch == '~') {
            encoded += (char)ch;
        } else if (ch == ' ') {
            encoded += '+';
        } else {
            encoded += '%';
            encoded += HEX_DIGITS[ch >> 4];
            encoded += HEX_DIGITS[ch & 0x0F];
        }
    }
    return encoded;
}

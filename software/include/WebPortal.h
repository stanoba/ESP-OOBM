#pragma once
#include <Arduino.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <Preferences.h>
#include "Config.h"
#include "SystemStats.h"

class ResponseWriter {
public:
    virtual ~ResponseWriter() {}
    virtual void setStatus(int code, const char *statusStr = "200 OK") = 0;
    virtual void setHeader(const char *name, const char *value) = 0;
    virtual void setContentType(const char *type) = 0;
    virtual void sendChunk(const char *buf, size_t len) = 0;
    virtual void sendChunk(const String &str) { sendChunk(str.c_str(), str.length()); }
    virtual void sendChunk_P(PGM_P buf) = 0;
    virtual void end() = 0;
};

class WebServerResponseWriter : public ResponseWriter {
public:
    WebServerResponseWriter(WebServer &server) : _server(server), _statusCode(200) {}
    void setStatus(int code, const char *statusStr = "200 OK") override {
        _statusCode = code;
    }
    void setHeader(const char *name, const char *value) override {
        _server.sendHeader(name, value);
    }
    void setContentType(const char *type) override {
        _server.setContentLength(CONTENT_LENGTH_UNKNOWN);
        _server.send(_statusCode, type, "");
    }
    void sendChunk(const char *buf, size_t len) override {
        _server.sendContent(String(buf, len));
    }
    void sendChunk(const String &str) override {
        _server.sendContent(str);
    }
    void sendChunk_P(PGM_P buf) override {
        _server.sendContent_P(buf);
    }
    void end() override {
        _server.sendContent("");
    }
private:
    WebServer &_server;
    int _statusCode;
};

class WebPortal {
public:
    WebPortal(WebServer &server, DNSServer &dnsServer, Preferences &prefs);
    void begin();
    void loop();

    void setAuthCredentials(bool enabled, const char *user, const char *pass);
    bool checkAuth();
    bool isAuthenticated();

    // HTTP Route Rendering
    void renderRoot(ResponseWriter &res);
    void renderTerminal(ResponseWriter &res);
    void renderSettings(ResponseWriter &res);
    void renderWifiPage(ResponseWriter &res);
    void renderUpdatePage(ResponseWriter &res);
    void renderMetrics(ResponseWriter &res);
    void renderLoginPage(ResponseWriter &res, const char *errMsg = nullptr);

    // Unified API Handlers
    void handleApiLogin(ResponseWriter &res, const String &u, const String &p);
    void handleApiStatus(ResponseWriter &res);
    void handleApiScan(ResponseWriter &res);
    void handleApiLogs(ResponseWriter &res);
    void handleApiSaveSettings(ResponseWriter &res, const String &body);
    void handleApiSaveWifi(ResponseWriter &res, const String &body);
    void handleApiRestart(ResponseWriter &res);
    void handleApiFactoryReset(ResponseWriter &res);
    void handleApiPlatform(ResponseWriter &res, const String &platform);

private:
    WebServer   &_server;
    DNSServer   &_dnsServer;
    Preferences &_prefs;

    bool _authRequired;
    char _authUser[32];
    char _authPass[32];
    String _sessionToken;
    bool _isApMode;
    bool _captiveEnabled;

    void updateSessionToken();
    void registerHttpRoutes();

    // Captive Portal Handlers
    void handleCaptivePortal();
    void handleNotFound();

    // HTML Component Generators (PROGMEM Streaming)
    void streamHeader(ResponseWriter &res, const char *activeTab, const char *title);
    void streamFooter(ResponseWriter &res);
};

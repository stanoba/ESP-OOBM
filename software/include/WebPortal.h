#pragma once
#include <Arduino.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <Preferences.h>
#include <esp_https_server.h>
#include "Config.h"
#include "SystemStats.h"
#include "TlsManager.h"

#define MAX_SSL_WS_CLIENTS 2

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
    WebServerResponseWriter(WebServer &server) : _server(server) {}
    void setStatus(int code, const char *statusStr = "200 OK") override {
        // WebServer status code is managed internally or via sendHeader/send
    }
    void setHeader(const char *name, const char *value) override {
        _server.sendHeader(name, value);
    }
    void setContentType(const char *type) override {
        _server.setContentLength(CONTENT_LENGTH_UNKNOWN);
        _server.send(200, type, "");
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
};

class HttpsResponseWriter : public ResponseWriter {
public:
    HttpsResponseWriter(httpd_req_t *req) : _req(req), _ended(false) {
        if (_req) {
            httpd_resp_set_hdr(_req, "Connection", "close");
        }
    }
    void setStatus(int code, const char *statusStr = "200 OK") override {
        httpd_resp_set_status(_req, statusStr);
    }
    void setHeader(const char *name, const char *value) override {
        httpd_resp_set_hdr(_req, name, value);
    }
    void setContentType(const char *type) override {
        httpd_resp_set_type(_req, type);
    }
    void sendChunk(const char *buf, size_t len) override {
        if (_ended || !_req || !buf || len == 0) return;
        const size_t maxSlice = 1024;
        size_t offset = 0;
        while (offset < len) {
            size_t toSend = (len - offset > maxSlice) ? maxSlice : (len - offset);
            esp_err_t err = httpd_resp_send_chunk(_req, buf + offset, toSend);
            if (err != ESP_OK) {
                _ended = true;
                return;
            }
            offset += toSend;
        }
    }
    void sendChunk(const String &str) override {
        sendChunk(str.c_str(), str.length());
    }
    void sendChunk_P(PGM_P buf) override {
        if (_ended || !_req || !buf) return;
        sendChunk(buf, strlen_P(buf));
    }
    void end() override {
        if (!_ended && _req) {
            httpd_resp_send_chunk(_req, NULL, 0);
            _ended = true;
        }
    }
private:
    httpd_req_t *_req;
    bool _ended;
};

class WebPortal {
public:
    WebPortal(WebServer &server, DNSServer &dnsServer, Preferences &prefs);
    void begin();
    void loop();

    void setAuthCredentials(bool enabled, const char *user, const char *pass);
    bool checkAuth();
    bool isAuthenticated();
    bool isRequestAuthenticated(httpd_req_t *req);

    // WebSocket Secure (WSS) Support
    void broadcastWs(const uint8_t *data, size_t len);
    void registerSslWsClient(int fd);
    void unregisterSslWsClient(int fd);
    void handleSslWsPayload(int fd, httpd_ws_type_t type, const uint8_t *payload, size_t length);
    uint16_t getActiveSslWsClients() const {
        uint16_t count = 0;
        for (int i = 0; i < MAX_SSL_WS_CLIENTS; i++) {
            if (_sslWsClients[i] > 0) count++;
        }
        return count;
    }

    // Dynamic HTTPS Server Lifecycle
    bool startHttpsServer();
    void stopHttpsServer();
    bool reloadTlsCertificates();

    // Unified Route Rendering (Dual HTTP / HTTPS)
    void renderRoot(ResponseWriter &res);
    void renderTerminal(ResponseWriter &res);
    void renderSettings(ResponseWriter &res);
    void renderWifiPage(ResponseWriter &res);
    void renderUpdatePage(ResponseWriter &res);
    void renderMetrics(ResponseWriter &res);
    void renderLoginPage(ResponseWriter &res, const char *errMsg = nullptr);
    void renderLogout(ResponseWriter &res);

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
    void handleApiTlsInfo(ResponseWriter &res);
    void handleApiTlsUpload(ResponseWriter &res, const String &certPem, const String &keyPem);
    void handleApiTlsReset(ResponseWriter &res);

private:
    WebServer   &_server;
    DNSServer   &_dnsServer;
    Preferences &_prefs;
    httpd_handle_t _httpsServer;

    bool _authRequired;
    char _authUser[32];
    char _authPass[32];
    String _sessionToken;
    bool _isApMode;
    bool _captiveEnabled;
    bool _httpsRedirect;

    int  _sslWsClients[MAX_SSL_WS_CLIENTS];
    bool _sslWsAuth[MAX_SSL_WS_CLIENTS];

    void updateSessionToken();
    void registerHttpRoutes();
    void registerHttpsRoutes();
    static void processSslWsTxWork(void *arg);

    // Captive Portal Handlers
    void handleCaptivePortal();
    void handleNotFound();

    // HTML Component Generators (PROGMEM Streaming)
    void streamHeader(ResponseWriter &res, const char *activeTab, const char *title);
    void streamFooter(ResponseWriter &res);
};

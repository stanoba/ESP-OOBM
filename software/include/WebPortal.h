#pragma once
#include <Arduino.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <Preferences.h>
#include "Config.h"
#include "SystemStats.h"

class WebPortal {
public:
    WebPortal(WebServer &server, DNSServer &dnsServer, Preferences &prefs);
    void begin();
    void loop();

    void setAuthCredentials(bool enabled, const char *user, const char *pass);
    bool checkAuth();
    bool isAuthenticated();

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

    // HTTP Route Handlers
    void handleRoot();
    void handleTerminal();
    void handleSettings();
    void handleWifiPage();
    void handleUpdatePage();
    void handleMetrics();
    void handleUpdateUpload();
    void handleUpdateFinish();
    void handleSyncNtp();
    void handleResetWifi();
    void handleLoginPage();
    void handleLogout();

    // AJAX API Handlers
    void handleApiLogin();
    void handleApiStatus();
    void handleApiScan();
    void handleApiLogs();
    void handleApiSaveSettings();
    void handleApiSaveWifi();
    void handleApiRestart();
    void handleApiFactoryReset();
    void handleApiPlatform();

    // Captive Portal Handlers
    void handleCaptivePortal();
    void handleNotFound();

    // HTML Component Generators (PROGMEM Streaming)
    void streamHeader(const char *activeTab, const char *title);
    void streamFooter();
};

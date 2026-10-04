#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <Preferences.h>
#include "Config.h"

enum TelnetAuthState : uint8_t {
    TELNET_AUTH_NONE = 0,
    TELNET_AUTH_PROMPT,
    TELNET_AUTH_OK
};

struct TelnetClientSession {
    WiFiClient client;
    TelnetAuthState authState;
    uint8_t failedAttempts;
    char passBuffer[32];
    uint8_t passLen;
};

class TelnetServer {
public:
    static const size_t MAX_TELNET_CLIENTS = 2;

    TelnetServer();
    void begin(Preferences &prefs);
    void loop();
    void broadcast(const uint8_t *data, size_t len);
    void stop();

    bool isEnabled() const { return _enabled; }
    uint16_t getPort() const { return _port; }
    bool isAuthRequired() const { return _authRequired; }

    void setEnabled(bool en) { _enabled = en; }
    void setPort(uint16_t port) { _port = port; }
    void setAuth(bool en, const char *pass);

private:
    bool     _enabled;
    uint16_t _port;
    bool     _authRequired;
    char     _password[32];

    WiFiServer _server;
    TelnetClientSession _sessions[MAX_TELNET_CLIENTS];

    void handleNewClient();
    void processClient(TelnetClientSession &sess);
    void sendTelnetNegotiation(WiFiClient &c);
    void closeSession(TelnetClientSession &sess);
};

extern TelnetServer telnetServer;

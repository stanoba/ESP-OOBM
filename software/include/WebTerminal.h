#pragma once
#include <Arduino.h>
#include <WebSocketsServer.h>
#include <Preferences.h>
#include "Config.h"

class WebTerminal {
public:
    WebTerminal();
    void begin(Preferences &prefs);
    void loop();
    void broadcast(const uint8_t *data, size_t len);

    void setAuth(bool en, const char *user, const char *pass);
    bool isAuthRequired() const { return _authRequired; }

private:
    WebSocketsServer _wsServer;
    bool _authRequired;
    char _authUser[32];
    char _authPass[32];
    bool _clientAuth[WEBSOCKETS_SERVER_CLIENT_MAX];

    void onWebSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length);
    static void staticWsEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length);
};

extern WebTerminal webTerminal;

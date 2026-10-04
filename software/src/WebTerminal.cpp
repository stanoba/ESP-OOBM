#include "WebTerminal.h"
#include "SerialBridge.h"
#include "ConsoleLogger.h"

WebTerminal webTerminal;
uint16_t g_activeWsClients = 0;

static WebTerminal *s_instance = nullptr;

WebTerminal::WebTerminal() 
    : _wsServer(WEBSOCKET_PORT),
      _authRequired(false) {
    _authUser[0] = '\0';
    _authPass[0] = '\0';
    memset(_clientAuth, 0, sizeof(_clientAuth));
    s_instance = this;
}

void WebTerminal::staticWsEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length) {
    if (s_instance) {
        s_instance->onWebSocketEvent(num, type, payload, length);
    }
}

void WebTerminal::begin(Preferences &prefs) {
    _authRequired = prefs.getBool(NVS_KEY_AUTH_EN, false);
    String user = prefs.getString(NVS_KEY_AUTH_USER, "admin");
    String pass = prefs.getString(NVS_KEY_AUTH_PASS, "admin");

    strncpy(_authUser, user.c_str(), sizeof(_authUser) - 1);
    _authUser[sizeof(_authUser) - 1] = '\0';
    strncpy(_authPass, pass.c_str(), sizeof(_authPass) - 1);
    _authPass[sizeof(_authPass) - 1] = '\0';

    _wsServer.begin();
    _wsServer.onEvent(staticWsEvent);

    logger.logInfo("WebSocket Terminal started on port %u (Auth: %s)", 
                   WEBSOCKET_PORT, _authRequired ? "Enabled" : "Disabled");
}

void WebTerminal::setAuth(bool en, const char *user, const char *pass) {
    _authRequired = en;
    if (user) {
        strncpy(_authUser, user, sizeof(_authUser) - 1);
        _authUser[sizeof(_authUser) - 1] = '\0';
    }
    if (pass) {
        strncpy(_authPass, pass, sizeof(_authPass) - 1);
        _authPass[sizeof(_authPass) - 1] = '\0';
    }
}

void WebTerminal::onWebSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length) {
    switch (type) {
        case WStype_CONNECTED: {
            IPAddress ip = _wsServer.remoteIP(num);
            logger.logInfo("WebSocket client #%u connected from %s", num, ip.toString().c_str());

            if (num < WEBSOCKETS_SERVER_CLIENT_MAX) {
                _clientAuth[num] = !_authRequired;
            }

            if (_authRequired) {
                // Request client auth token/credentials
                const char req[] = "\x1b[33m[ESP-OOBM: Authentication Required]\x1b[0m\r\n";
                _wsServer.sendBIN(num, (const uint8_t*)req, strlen(req));
            } else {
                if (serialBridge.getGreetingBanner()) {
                    const char banner[] = "\x1b[32m[ESP-OOBM: Connected to Serial Console]\x1b[0m\r\n";
                    _wsServer.sendBIN(num, (const uint8_t*)banner, strlen(banner));
                }
            }

            g_activeWsClients = _wsServer.connectedClients();
            break;
        }

        case WStype_DISCONNECTED: {
            logger.logInfo("WebSocket client #%u disconnected", num);
            if (num < WEBSOCKETS_SERVER_CLIENT_MAX) {
                _clientAuth[num] = false;
            }
            g_activeWsClients = _wsServer.connectedClients();
            break;
        }

        case WStype_TEXT:
        case WStype_BIN: {
            if (!payload || length == 0) break;

            if (num < WEBSOCKETS_SERVER_CLIENT_MAX && !_clientAuth[num]) {
                // Check if message is authentication token "AUTH:<user>:<pass>"
                if (length > 5 && memcmp(payload, "AUTH:", 5) == 0) {
                    char authBuf[96];
                    size_t copyLen = (length - 5 < sizeof(authBuf) - 1) ? (length - 5) : (sizeof(authBuf) - 1);
                    memcpy(authBuf, payload + 5, copyLen);
                    authBuf[copyLen] = '\0';

                    char *colon = strchr(authBuf, ':');
                    if (colon) {
                        *colon = '\0';
                        const char *u = authBuf;
                        const char *p = colon + 1;
                        if (strcmp(u, _authUser) == 0 && strcmp(p, _authPass) == 0) {
                            _clientAuth[num] = true;
                            const char okMsg[] = "\x1b[32m[Authenticated successfully. Connected to Serial Console.]\x1b[0m\r\n";
                            _wsServer.sendBIN(num, (const uint8_t*)okMsg, strlen(okMsg));
                            logger.logInfo("WebSocket client #%u authenticated successfully.", num);
                            return;
                        }
                    }
                }
                const char failMsg[] = "\x1b[31m[Authentication failed]\x1b[0m\r\n";
                _wsServer.sendBIN(num, (const uint8_t*)failMsg, strlen(failMsg));
                return;
            }

            // Client is authenticated: forward payload to Serial
            serialBridge.write(payload, length);
            if (serialBridge.getForceEcho()) {
                _wsServer.sendBIN(num, payload, length);
            }
            break;
        }

        case WStype_ERROR:
        case WStype_PING:
        case WStype_PONG:
        default:
            break;
    }
}

void WebTerminal::loop() {
    _wsServer.loop();
    g_activeWsClients = _wsServer.connectedClients();
}

void WebTerminal::broadcast(const uint8_t *data, size_t len) {
    if (!data || len == 0) return;

    for (uint8_t i = 0; i < WEBSOCKETS_SERVER_CLIENT_MAX; i++) {
        if (_clientAuth[i]) {
            _wsServer.sendBIN(i, data, len);
        }
    }
}

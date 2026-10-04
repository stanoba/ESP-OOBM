#include "TelnetServer.h"
#include "SerialBridge.h"
#include "ConsoleLogger.h"

TelnetServer telnetServer;
uint16_t g_activeTelnetClients = 0;

TelnetServer::TelnetServer() 
    : _enabled(true),
      _port(TELNET_PORT),
      _authRequired(DEFAULT_TELNET_AUTH),
      _server(TELNET_PORT) {
    strncpy(_password, DEFAULT_TELNET_PASS, sizeof(_password) - 1);
    _password[sizeof(_password) - 1] = '\0';
    for (size_t i = 0; i < MAX_TELNET_CLIENTS; i++) {
        _sessions[i].authState = TELNET_AUTH_NONE;
        _sessions[i].failedAttempts = 0;
        _sessions[i].passLen = 0;
        memset(_sessions[i].passBuffer, 0, sizeof(_sessions[i].passBuffer));
    }
}

void TelnetServer::begin(Preferences &prefs) {
    _enabled = prefs.getBool(NVS_KEY_TELNET_EN, true);
    _port = prefs.getUShort(NVS_KEY_TELNET_PORT, TELNET_PORT);
    _authRequired = prefs.getBool(NVS_KEY_TELNET_AUTH, DEFAULT_TELNET_AUTH);
    
    String pass = prefs.getString(NVS_KEY_TELNET_PASS, DEFAULT_TELNET_PASS);
    strncpy(_password, pass.c_str(), sizeof(_password) - 1);
    _password[sizeof(_password) - 1] = '\0';

    if (_enabled) {
        _server = WiFiServer(_port);
        _server.begin();
        logger.logInfo("Telnet server started on port %u (Auth: %s)", 
                       _port, _authRequired ? "Enabled" : "Disabled");
    }
}

void TelnetServer::setAuth(bool en, const char *pass) {
    _authRequired = en;
    if (pass) {
        strncpy(_password, pass, sizeof(_password) - 1);
        _password[sizeof(_password) - 1] = '\0';
    }
}

void TelnetServer::stop() {
    for (size_t i = 0; i < MAX_TELNET_CLIENTS; i++) {
        closeSession(_sessions[i]);
    }
    _server.stop();
    _enabled = false;
    g_activeTelnetClients = 0;
}

void TelnetServer::sendTelnetNegotiation(WiFiClient &c) {
    // Telnet IAC commands:
    // IAC WILL ECHO (255, 251, 1)
    // IAC WILL SUPPRESS_GO_AHEAD (255, 251, 3)
    const uint8_t telnetInit[] = {
        255, 251, 1,   // IAC WILL ECHO
        255, 251, 3,   // IAC WILL SUPPRESS_GO_AHEAD
        255, 253, 3    // IAC DO SUPPRESS_GO_AHEAD
    };
    c.write(telnetInit, sizeof(telnetInit));
}

void TelnetServer::handleNewClient() {
    if (!_server.hasClient()) return;

    WiFiClient newClient = _server.available();
    if (!newClient) return;

    // Find free session slot
    int slot = -1;
    for (size_t i = 0; i < MAX_TELNET_CLIENTS; i++) {
        if (!_sessions[i].client || !_sessions[i].client.connected()) {
            slot = (int)i;
            break;
        }
    }

    if (slot == -1) {
        // No slots available
        newClient.println("\r\n[ESP-OOBM: Maximum Telnet connections reached. Disconnecting.]\r\n");
        newClient.stop();
        logger.logWarn("Telnet connection rejected: maximum client limit reached.");
        return;
    }

    TelnetClientSession &sess = _sessions[slot];
    sess.client = newClient;
    sess.failedAttempts = 0;
    sess.passLen = 0;
    memset(sess.passBuffer, 0, sizeof(sess.passBuffer));

    sendTelnetNegotiation(sess.client);

    if (_authRequired && strlen(_password) > 0) {
        sess.authState = TELNET_AUTH_PROMPT;
        sess.client.print("\r\n=========================================\r\n");
        sess.client.print("  ESP-OOBM Console - Authentication\r\n");
        sess.client.print("=========================================\r\n");
        sess.client.print("Password: ");
        logger.logInfo("Telnet client connected from %s (Awaiting auth)", sess.client.remoteIP().toString().c_str());
    } else {
        sess.authState = TELNET_AUTH_OK;
        if (serialBridge.getGreetingBanner()) {
            sess.client.print("\r\n[ESP-OOBM: Connected to Serial Console]\r\n\r\n");
        }
        logger.logInfo("Telnet client connected from %s", sess.client.remoteIP().toString().c_str());
    }

    // Update active count
    uint16_t count = 0;
    for (size_t i = 0; i < MAX_TELNET_CLIENTS; i++) {
        if (_sessions[i].client && _sessions[i].client.connected()) count++;
    }
    g_activeTelnetClients = count;
}

void TelnetServer::closeSession(TelnetClientSession &sess) {
    if (sess.client) {
        sess.client.stop();
    }
    sess.authState = TELNET_AUTH_NONE;
    sess.failedAttempts = 0;
    sess.passLen = 0;
}

void TelnetServer::processClient(TelnetClientSession &sess) {
    if (!sess.client || !sess.client.connected()) {
        closeSession(sess);
        return;
    }

    while (sess.client.available() > 0) {
        int b = sess.client.read();
        if (b < 0) break;
        uint8_t byte = (uint8_t)b;

        // Skip Telnet IAC commands (0xFF ...)
        if (byte == 255) {
            if (sess.client.available() >= 2) {
                sess.client.read(); // command
                sess.client.read(); // option
            }
            continue;
        }

        if (sess.authState == TELNET_AUTH_PROMPT) {
            if (byte == '\r' || byte == '\n') {
                sess.passBuffer[sess.passLen] = '\0';
                if (strcmp(sess.passBuffer, _password) == 0) {
                    sess.authState = TELNET_AUTH_OK;
                    sess.client.println("\r\n[Authenticated. Welcome to Console.]\r\n");
                    logger.logInfo("Telnet client from %s authenticated successfully.", sess.client.remoteIP().toString().c_str());
                } else {
                    sess.failedAttempts++;
                    sess.passLen = 0;
                    if (sess.failedAttempts >= 3) {
                        sess.client.println("\r\n[Authentication failed: Maximum attempts exceeded. Disconnecting.]\r\n");
                        closeSession(sess);
                        logger.logWarn("Telnet client disconnected: 3 failed password attempts.");
                        return;
                    } else {
                        sess.client.print("\r\nInvalid password. Password: ");
                    }
                }
            } else if (byte == 8 || byte == 127) { // Backspace
                if (sess.passLen > 0) sess.passLen--;
            } else if (sess.passLen < sizeof(sess.passBuffer) - 1 && byte >= 32 && byte <= 126) {
                sess.passBuffer[sess.passLen++] = (char)byte;
            }
        } else if (sess.authState == TELNET_AUTH_OK) {
            // Forward character to serial
            serialBridge.write(byte);
            if (serialBridge.getForceEcho()) {
                sess.client.write(byte);
            }
        }
    }
}

void TelnetServer::loop() {
    if (!_enabled) return;

    handleNewClient();

    uint16_t active = 0;
    for (size_t i = 0; i < MAX_TELNET_CLIENTS; i++) {
        if (_sessions[i].client && _sessions[i].client.connected()) {
            processClient(_sessions[i]);
            if (_sessions[i].client && _sessions[i].client.connected()) active++;
        } else if (_sessions[i].authState != TELNET_AUTH_NONE) {
            closeSession(_sessions[i]);
        }
    }
    g_activeTelnetClients = active;
}

void TelnetServer::broadcast(const uint8_t *data, size_t len) {
    if (!_enabled || !data || len == 0) return;

    for (size_t i = 0; i < MAX_TELNET_CLIENTS; i++) {
        if (_sessions[i].client && _sessions[i].client.connected() && _sessions[i].authState == TELNET_AUTH_OK) {
            _sessions[i].client.write(data, len);
        }
    }
}

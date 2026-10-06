#include "WebPortal.h"
#include "VectorGraphics.h"
#include "WebUtils.h"
#include "Timezones.h"
#include "SerialBridge.h"
#include "TelnetServer.h"
#include "WebTerminal.h"
#include "PrometheusExporter.h"
#include "MndpDiscovery.h"
#include "ConsoleLogger.h"
#include <Update.h>
#include <esp_sntp.h>

#define WEBPORTAL_STRINGIFY_IMPL(value) #value
#define WEBPORTAL_STRINGIFY(value) WEBPORTAL_STRINGIFY_IMPL(value)

extern void triggerNtpSync();
extern time_t g_lastNtpSyncTimestamp;
extern bool g_ntpSynced;

// =============================================================================
// CSS Stylesheet in Flash ROM (PROGMEM) - Zero RAM Allocation
// =============================================================================
static const char COMMON_CSS[] PROGMEM =
R"rawliteral(
html{box-sizing:border-box;overflow-y:scroll;}
*,*::before,*::after{box-sizing:inherit;}
:root{--navy:#0284c7;--green:#10b981;--cyan:#00c0d8;--bg:#0b1120;--card:#151e32;--text:#e2e8f0;--muted:#94a3b8;--border:#22324d;--hover:#1e293b;--danger:#ef4444;--warn:#f59e0b;}
html.light{--navy:#0284c7;--green:#059669;--cyan:#0891b2;--bg:#f4f6fa;--card:#ffffff;--text:#0f172a;--muted:#64748b;--border:#e2e8f0;--hover:#e2e8f0;--danger:#dc2626;--warn:#d97706;}
body{font-family:-apple-system,BlinkMacSystemFont,'Segoe UI',Roboto,Helvetica,Arial,sans-serif;background:var(--bg);color:var(--text);margin:0;padding:0;line-height:1.5;}
.header-wrap{position:sticky;top:0;z-index:1000;width:100%;}
.top-accent{height:3px;width:100%;background:linear-gradient(90deg,var(--cyan),var(--navy),var(--green));}
.navbar{position:relative;background:var(--card);border-bottom:1px solid var(--border);box-shadow:0 2px 10px rgba(0,0,0,0.2);padding:8px 0;width:100%;}
.nav-inner{max-width:1200px;margin:0 auto;padding:0 18px;width:100%;display:flex;align-items:center;justify-content:space-between;position:relative;}
.brand{display:flex;align-items:center;gap:10px;text-decoration:none;flex-shrink:0;}
.brand svg{height:46px;width:auto;display:block;}
html.light .brand-title-main{fill:#091a36!important;}
html.light .brand-title-prefix{fill:#0284c7!important;stroke:#0284c7!important;}
html.light .brand-sub{fill:#009aa0!important;}
html:not(.light) .brand-title-main{fill:#f8fafc!important;}
html:not(.light) .brand-title-prefix{fill:#38bdf8!important;stroke:#38bdf8!important;}
html:not(.light) .brand-sub{fill:#2dd4bf!important;}
.nav-links{display:flex;gap:8px;align-items:center;}
.nav-link{padding:6px 14px;border-radius:6px;font-size:0.90rem;font-weight:600;text-decoration:none;color:var(--muted);border:1px solid transparent;transition:all 0.15s;}
.nav-link:hover{color:var(--text);background:var(--hover);}
.nav-link.active{background:var(--navy);color:#ffffff;border-color:var(--navy);box-shadow:0 2px 6px rgba(2,132,199,0.3);}
.badge-clock{font-family:Consolas,monospace;font-size:0.82rem;padding:4px 9px;border-radius:6px;background:var(--hover);border:1px solid var(--border);color:var(--muted);display:flex;align-items:center;gap:6px;}

.nav-actions{display:flex;align-items:center;gap:10px;flex-shrink:0;}
.theme-switch{display:inline-flex;align-items:center;background:var(--hover);border:1px solid var(--border);border-radius:20px;padding:2px;gap:2px;}
.theme-btn{background:transparent;border:none;border-radius:16px;padding:4px 7px;display:inline-flex;align-items:center;justify-content:center;color:var(--muted);cursor:pointer;transition:all 0.15s;}
.theme-btn:hover{color:var(--text);}
.theme-btn.active{background:var(--card);color:var(--text);box-shadow:0 1px 3px rgba(0,0,0,0.25);}

@media(min-width:961px){
  .nav-links{position:absolute;left:50%;transform:translateX(-50%);}
}
@media(max-width:960px){
  .header-wrap{position:static;}
  .navbar{padding:10px 0;}
  .nav-inner{flex-direction:column;align-items:center;gap:10px;}
  .nav-links{position:static;transform:none;}
  .nav-actions{margin-top:4px;}
}

.container{max-width:1200px;margin:22px auto;padding:0 18px;}
.grid-cards{display:grid;grid-template-columns:repeat(auto-fit,minmax(260px,1fr));gap:16px;margin-bottom:24px;}
.card, .table-card{background:var(--card);border:1px solid var(--border);border-radius:10px;padding:18px 20px;box-shadow:0 4px 12px rgba(0,0,0,0.06);position:relative;margin-bottom:20px;}
.card h3, .table-card h3{margin:0 0 12px 0;font-size:0.92rem;text-transform:uppercase;letter-spacing:0.06em;color:var(--muted);font-weight:700;display:flex;align-items:center;justify-content:space-between;}
.card .val{font-size:1.85rem;font-weight:700;font-family:Consolas,monospace;color:var(--text);line-height:1.2;}
.card .val.val-accent{color:var(--navy);}
.card .val.val-green{color:var(--green);}
.card .sub{font-size:0.84rem;color:var(--muted);margin-top:6px;font-weight:500;}
.stat-row{display:flex;justify-content:space-between;padding:6px 0;border-bottom:1px solid var(--border);font-size:0.88rem;}
.stat-row:last-child{border-bottom:none;}
.stat-label{color:var(--muted);font-weight:500;}
.stat-val{font-family:Consolas,monospace;font-weight:600;}

.badge{padding:3px 8px;border-radius:4px;font-size:0.75rem;font-weight:700;text-transform:uppercase;letter-spacing:0.04em;}
.badge-ok{background:#064e3b;color:#34d399;border:1px solid #059669;}
html.light .badge-ok{background:#dcfce7;color:#15803d;border:1px solid #86efac;}
.badge-warn{background:#451a03;color:#fbbf24;border:1px solid #d97706;}
html.light .badge-warn{background:#fef3c7;color:#b45309;border:1px solid #fde68a;}
.badge-danger{background:#450a0a;color:#f87171;border:1px solid #dc2626;}
html.light .badge-danger{background:#fee2e2;color:#b91c1c;border:1px solid #fca5a5;}

.btn{padding:8px 16px;border-radius:6px;font-size:0.88rem;font-weight:600;text-decoration:none;display:inline-flex;align-items:center;justify-content:center;gap:6px;border:none;cursor:pointer;transition:all 0.18s;white-space:nowrap;}
.btn-primary{background:var(--navy);color:#ffffff;}
.btn-primary:hover{background:#0369a1;color:#ffffff;transform:translateY(-1px);box-shadow:0 2px 6px rgba(2,132,199,0.35);}
.btn-green{background:var(--green);color:#ffffff;}
.btn-green:hover{background:#059669;color:#ffffff;transform:translateY(-1px);box-shadow:0 2px 6px rgba(16,185,129,0.35);}
.btn-danger{background:var(--danger);color:#ffffff;}
.btn-danger:hover{background:#b91c1c;color:#ffffff;transform:translateY(-1px);box-shadow:0 2px 6px rgba(239,68,68,0.35);}
.btn-outline{background:var(--card);color:var(--text);border:1px solid var(--border);box-shadow:0 1px 2px rgba(0,0,0,0.05);}
.btn-outline:hover{background:var(--hover);border-color:#38bdf8;color:#38bdf8;transform:translateY(-1px);}
html.light .btn-outline:hover{background:#e2e8f0;border-color:var(--navy);color:var(--navy);}
.btn-sm{padding:4px 10px;font-size:0.80rem;}

.form-grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(240px,1fr));gap:16px;}
.form-group{display:flex;flex-direction:column;gap:6px;min-width:0;}
.form-group label{font-size:0.84rem;font-weight:600;color:var(--muted);}
.form-control{width:100%;padding:8px 12px;border-radius:6px;border:1px solid var(--border);background:var(--bg);color:var(--text);font-size:0.88rem;outline:none;transition:border-color 0.15s;min-width:0;box-sizing:border-box;}
.form-control:focus{border-color:var(--navy);}
.switch-label{display:inline-flex;align-items:center;gap:10px;cursor:pointer;user-select:none;font-size:0.88rem;font-weight:500;}
.pwd-wrap{position:relative;display:flex;align-items:center;width:100%;box-sizing:border-box;}
.pwd-wrap .form-control{width:100%!important;flex:1 1 100%;padding-right:38px;}
.pwd-toggle{position:absolute;right:8px;top:50%;transform:translateY(-50%);background:none;border:none;padding:4px;cursor:pointer;color:var(--muted);display:inline-flex;align-items:center;justify-content:center;border-radius:4px;transition:color 0.15s;z-index:2;}
.pwd-toggle:hover{color:var(--text);}
.pwd-toggle:focus{outline:none;color:var(--navy);}

.term-container{background:#000000;border:1px solid var(--border);border-radius:8px;padding:12px;font-family:Consolas,Courier,monospace;font-size:0.92rem;color:#e2e8f0;height:calc(100vh - 280px);min-height:360px;overflow-y:auto;white-space:pre-wrap;word-break:break-all;outline:none;box-shadow:inset 0 2px 8px rgba(0,0,0,0.6);line-height:1.35;}
.term-cursor{display:inline-block;min-width:8px;background:#38bdf8;color:#000000!important;border-radius:1px;animation:term-blink 1s steps(2,start) infinite;}
@keyframes term-blink{0%,100%{opacity:1;}50%{opacity:0.15;}}
.term-toolbar{display:flex;gap:6px;flex-wrap:wrap;margin-top:10px;padding:8px;background:var(--card);border:1px solid var(--border);border-radius:8px;}
.term-key{padding:5px 9px;border-radius:4px;background:var(--hover);border:1px solid var(--border);color:var(--text);font-family:monospace;font-size:0.80rem;font-weight:600;cursor:pointer;user-select:none;transition:all 0.1s;}
.term-key:hover{background:#334155;}
.term-key:active{background:var(--navy);color:#ffffff;}

.log-box{background:#000000;border:1px solid var(--border);border-radius:8px;padding:12px;font-family:Consolas,monospace;font-size:0.84rem;height:320px;overflow-y:auto;color:#cbd5e1;white-space:pre-wrap;}
.log-info{color:#38bdf8;}
.log-warn{color:#f59e0b;}
.log-error{color:#ef4444;}
.toast{position:fixed;bottom:24px;right:24px;padding:12px 20px;border-radius:8px;background:var(--card);border:1px solid var(--border);box-shadow:0 6px 16px rgba(0,0,0,0.3);font-size:0.88rem;font-weight:600;display:none;z-index:9999;}
.progress-bar-wrap{height:8px;background:var(--hover);border-radius:4px;overflow:hidden;margin-top:12px;}
.progress-bar-fill{height:100%;width:0%;background:linear-gradient(90deg,var(--cyan),var(--green));transition:width 0.2s;}

.ap-banner{background:#fffbeb;border:1px solid #fef3c7;border-radius:8px;padding:10px 16px;margin-bottom:18px;display:flex;align-items:center;justify-content:space-between;gap:12px;font-size:0.86rem;color:#92400e;}
.ap-banner a{color:#b45309;font-weight:700;text-decoration:underline;}
html:not(.light) .ap-banner{background:#451a03;border-color:#78350f;color:#fde68a;}
html:not(.light) .ap-banner a{color:#fbbf24;}
)rawliteral";

// =============================================================================
// Shared JavaScript in Flash ROM (PROGMEM)
// =============================================================================
static const char COMMON_JS[] PROGMEM =
R"rawliteral(
function isDarkTheme(m){
  if(m === 'dark') return true;
  if(m === 'light') return false;
  var osDark = window.matchMedia && window.matchMedia('(prefers-color-scheme: dark)').matches;
  var hr = new Date().getHours() + new Date().getMinutes() / 60;
  return osDark || (hr >= 19 || hr < 7);
}
function setTheme(m){
  try{
    if(m === 'system') localStorage.removeItem('oobm_theme');
    else localStorage.setItem('oobm_theme', m);
  }catch(e){}
  applyThemeUI();
}
function applyThemeUI(){
  var m = 'system';
  try{ m = localStorage.getItem('oobm_theme') || 'system'; }catch(e){}
  var d = isDarkTheme(m);
  if(d) document.documentElement.classList.remove('light');
  else document.documentElement.classList.add('light');
  var bl = document.getElementById('themeBtnLight'), bd = document.getElementById('themeBtnDark'), bs = document.getElementById('themeBtnSystem');
  if(bl) bl.className = 'theme-btn' + (m === 'light' ? ' active' : '');
  if(bd) bd.className = 'theme-btn' + (m === 'dark' ? ' active' : '');
  if(bs) bs.className = 'theme-btn' + (m === 'system' ? ' active' : '');
}
if(window.matchMedia){
  try{
    window.matchMedia('(prefers-color-scheme: dark)').addEventListener('change', function(){
      if(!localStorage.getItem('oobm_theme')) applyThemeUI();
    });
  }catch(e){}
}
document.addEventListener('DOMContentLoaded', applyThemeUI);
(function(){ applyThemeUI(); })();

function showToast(msg, isErr){
  var t = document.getElementById('toast');
  if(!t){
    t = document.createElement('div');
    t.id = 'toast';
    t.className = 'toast';
    document.body.appendChild(t);
  }
  t.innerText = msg;
  t.style.borderColor = isErr ? 'var(--danger)' : 'var(--green)';
  t.style.color = isErr ? 'var(--danger)' : 'var(--green)';
  t.style.display = 'block';
  setTimeout(function(){ t.style.display = 'none'; }, 3500);
}

function togglePassword(btn){
  var wrap = btn.closest('.pwd-wrap');
  if(!wrap) return;
  var input = wrap.querySelector('input');
  if(!input) return;
  var open = btn.querySelector('.eye-open');
  var closed = btn.querySelector('.eye-closed');
  if(input.type === 'password'){
    input.type = 'text';
    if(open) open.style.display = 'none';
    if(closed) closed.style.display = 'block';
  } else {
    input.type = 'password';
    if(open) open.style.display = 'block';
    if(closed) closed.style.display = 'none';
  }
}
window.addEventListener('load', function(){
  try{
    var t = Math.round(performance.now());
    var el = document.getElementById('page_load_time');
    if(el) el.innerText = t + ' ms';
  }catch(e){}
});
)rawliteral";

static const char PWD_EYE_TOGGLE_HTML[] PROGMEM =
"<button type=\"button\" class=\"pwd-toggle\" onclick=\"togglePassword(this)\" title=\"Toggle password visibility\" tabindex=\"-1\">"
"<svg class=\"eye-open\" width=\"16\" height=\"16\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\"><path d=\"M1 12s4-8 11-8 11 8 11 8-4 8-11 8-11-8-11-8z\"/><circle cx=\"12\" cy=\"12\" r=\"3\"/></svg>"
"<svg class=\"eye-closed\" width=\"16\" height=\"16\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\" style=\"display:none;\"><path d=\"M17.94 17.94A10.07 10.07 0 0 1 12 20c-7 0-11-8-11-8a18.45 18.45 0 0 1 5.06-5.94M9.9 4.24A9.12 9.12 0 0 1 12 4c7 0 11 8 11 8a18.5 18.5 0 0 1-2.16 3.19m-6.72-1.07a3 3 0 1 1-4.24-4.24\"/><line x1=\"1\" y1=\"1\" x2=\"23\" y2=\"23\"/></svg>"
"</button>";

static String getRequestFormBody(WebServer &server) {
    if (server.hasArg("plain")) return server.arg("plain");

    // WebServer parses multipart FormData into named arguments instead of
    // exposing the raw multipart body as "plain". Serialize those arguments
    // so the shared form parser can handle both multipart and URL-encoded POSTs.
    String body;
    body.reserve(512);
    for (int i = 0; i < server.args(); ++i) {
        if (i > 0) body += '&';
        body += encodeFormValue(server.argName(i));
        body += '=';
        body += encodeFormValue(server.arg(i));
    }
    return body;
}

// =============================================================================
// Constructor & Initialization
// =============================================================================
WebPortal::WebPortal(WebServer &server, DNSServer &dnsServer, Preferences &prefs)
    : _server(server),
      _dnsServer(dnsServer),
      _prefs(prefs),
      _authRequired(DEFAULT_AUTH_ENABLED),
      _sessionToken(""),
      _isApMode(false),
      _captiveEnabled(true) {
    strncpy(_authUser, DEFAULT_AUTH_USER, sizeof(_authUser) - 1);
    strncpy(_authPass, DEFAULT_AUTH_PASS, sizeof(_authPass) - 1);
    updateSessionToken();
}

void WebPortal::updateSessionToken() {
    char buf[128];
    snprintf(buf, sizeof(buf), "%s:%s:esp-oobm-dongle-token", _authUser, _authPass);
    uint32_t hash1 = 5381, hash2 = 0;
    for (size_t i = 0; buf[i]; i++) {
        hash1 = ((hash1 << 5) + hash1) + (uint8_t)buf[i];
        hash2 = (hash2 * 31) + (uint8_t)buf[i];
    }
    char hexToken[33];
    snprintf(hexToken, sizeof(hexToken), "%08x%08x%08x%08x", hash1, hash2, hash1 ^ 0x5a5a5a5a, hash2 ^ 0xa5a5a5a5);
    _sessionToken = String(hexToken);
}

void WebPortal::setAuthCredentials(bool enabled, const char *user, const char *pass) {
    _authRequired = enabled;
    if (user) strncpy(_authUser, user, sizeof(_authUser) - 1);
    if (pass) strncpy(_authPass, pass, sizeof(_authPass) - 1);
    updateSessionToken();
}

bool WebPortal::isAuthenticated() {
    if (!_authRequired) return true;

    if (_server.hasHeader("Cookie")) {
        String cookie = _server.header("Cookie");
        if (cookie.indexOf("oobm_session=" + _sessionToken) != -1) {
            return true;
        }
    }

    if (_server.authenticate(_authUser, _authPass)) {
        return true;
    }

    return false;
}

bool WebPortal::checkAuth() {
    if (isAuthenticated()) return true;

    WebServerResponseWriter res(_server);
    renderLoginPage(res);
    return false;
}

// =============================================================================
// WebPortal Main Setup & Loop
// =============================================================================
void WebPortal::begin() {
    const char *headerKeys[] = {"Cookie", "Authorization", "Host"};
    _server.collectHeaders(headerKeys, 3);

    _authRequired = _prefs.getBool(NVS_KEY_AUTH_EN, DEFAULT_AUTH_ENABLED);
    String u = _prefs.getString(NVS_KEY_AUTH_USER, DEFAULT_AUTH_USER);
    String p = _prefs.getString(NVS_KEY_AUTH_PASS, DEFAULT_AUTH_PASS);
    strncpy(_authUser, u.c_str(), sizeof(_authUser) - 1);
    strncpy(_authPass, p.c_str(), sizeof(_authPass) - 1);
    updateSessionToken();

    _isApMode = (WiFi.getMode() & WIFI_MODE_AP);
    _captiveEnabled = _prefs.getBool(NVS_KEY_AP_CAPTIVE, true);

    if (_isApMode && _captiveEnabled) {
        _dnsServer.start(DNS_PORT, "*", IPAddress(AP_IP_ADDRESS));
    }

    // Register HTTP Port 80 Routes
    registerHttpRoutes();
    _server.begin();
    logger.logInfo("HTTP server started on port %u", HTTP_PORT);
}

void WebPortal::registerHttpRoutes() {
    // Captive Portal Detection Probes
    _server.on("/generate_204", HTTP_GET, [this]() { handleCaptivePortal(); });
    _server.on("/gen_204", HTTP_GET, [this]() { handleCaptivePortal(); });
    _server.on("/hotspot-detect.html", HTTP_GET, [this]() { handleCaptivePortal(); });
    _server.on("/canonical.html", HTTP_GET, [this]() { handleCaptivePortal(); });
    _server.on("/ncsi.txt", HTTP_GET, [this]() { handleCaptivePortal(); });
    _server.on("/connecttest.txt", HTTP_GET, [this]() { handleCaptivePortal(); });
    _server.on("/success.txt", HTTP_GET, [this]() { handleCaptivePortal(); });

    // HTML Page Routes on Port 80
    _server.on("/", HTTP_GET, [this]() {
        if (!checkAuth()) return;
        WebServerResponseWriter res(_server);
        renderRoot(res);
    });

    _server.on("/terminal", HTTP_GET, [this]() {
        if (!checkAuth()) return;
        WebServerResponseWriter res(_server);
        renderTerminal(res);
    });

    _server.on("/settings", HTTP_GET, [this]() {
        if (!checkAuth()) return;
        WebServerResponseWriter res(_server);
        renderSettings(res);
    });

    _server.on("/wifi", HTTP_GET, [this]() {
        if (!checkAuth()) return;
        WebServerResponseWriter res(_server);
        renderWifiPage(res);
    });

    _server.on("/update", HTTP_GET, [this]() {
        if (!checkAuth()) return;
        WebServerResponseWriter res(_server);
        renderUpdatePage(res);
    });

    _server.on("/metrics", HTTP_GET, [this]() {
        WebServerResponseWriter res(_server);
        renderMetrics(res);
    });

    _server.on("/login", HTTP_GET, [this]() {
        WebServerResponseWriter res(_server);
        renderLoginPage(res);
    });

    _server.on("/logout", HTTP_GET, [this]() {
        _server.sendHeader("Set-Cookie", "oobm_session=; Path=/; Expires=Thu, 01 Jan 1970 00:00:00 GMT");
        _server.sendHeader("Location", "/login");
        _server.send(302, "text/plain", "Logged out");
    });

    _server.on("/favicon.svg", HTTP_GET, [this]() {
        _server.sendHeader("Cache-Control", "public, max-age=604800");
        _server.send_P(200, "image/svg+xml", OOBM_FAVICON_SVG);
    });
    _server.on("/favicon.ico", HTTP_GET, [this]() {
        _server.sendHeader("Cache-Control", "public, max-age=604800");
        _server.send_P(200, "image/svg+xml", OOBM_FAVICON_SVG);
    });

    // AJAX API routes on Port 80
    _server.on("/api/login", HTTP_POST, [this]() {
        String u = _server.hasArg("usr") ? _server.arg("usr") : (_server.hasArg("user") ? _server.arg("user") : "");
        String p = _server.hasArg("pwd") ? _server.arg("pwd") : (_server.hasArg("pass") ? _server.arg("pass") : "");
        if (u.length() == 0 && _server.hasArg("plain")) {
            String body = _server.arg("plain");
            u = extractFormArg(body, "usr");
            if (u.length() == 0) u = extractFormArg(body, "user");
            p = extractFormArg(body, "pwd");
            if (p.length() == 0) p = extractFormArg(body, "pass");
        }
        WebServerResponseWriter res(_server);
        handleApiLogin(res, u, p);
    });
    _server.on("/api/status", HTTP_GET, [this]() {
        if (!isAuthenticated()) { _server.send(401, "application/json", "{\"error\":\"Unauthorized\"}"); return; }
        WebServerResponseWriter res(_server);
        handleApiStatus(res);
    });
    _server.on("/api/scan", HTTP_GET, [this]() {
        if (!isAuthenticated()) { _server.send(401, "application/json", "{\"error\":\"Unauthorized\"}"); return; }
        WebServerResponseWriter res(_server);
        handleApiScan(res);
    });
    _server.on("/api/logs", HTTP_GET, [this]() {
        if (!isAuthenticated()) { _server.send(401, "application/json", "{\"error\":\"Unauthorized\"}"); return; }
        WebServerResponseWriter res(_server);
        handleApiLogs(res);
    });
    _server.on("/api/settings/save", HTTP_POST, [this]() {
        if (!isAuthenticated()) { _server.send(401, "application/json", "{\"error\":\"Unauthorized\"}"); return; }
        WebServerResponseWriter res(_server);
        handleApiSaveSettings(res, getRequestFormBody(_server));
    });
    _server.on("/api/wifi/save", HTTP_POST, [this]() {
        if (!isAuthenticated()) { _server.send(401, "application/json", "{\"error\":\"Unauthorized\"}"); return; }
        WebServerResponseWriter res(_server);
        handleApiSaveWifi(res, getRequestFormBody(_server));
    });
    _server.on("/api/restart", HTTP_POST, [this]() {
        if (!isAuthenticated()) { _server.send(401, "application/json", "{\"error\":\"Unauthorized\"}"); return; }
        WebServerResponseWriter res(_server);
        handleApiRestart(res);
    });
    _server.on("/api/factory_reset", HTTP_POST, [this]() {
        if (!isAuthenticated()) { _server.send(401, "application/json", "{\"error\":\"Unauthorized\"}"); return; }
        WebServerResponseWriter res(_server);
        handleApiFactoryReset(res);
    });
    _server.on("/api/platform", HTTP_POST, [this]() {
        if (!isAuthenticated()) { _server.send(401, "application/json", "{\"error\":\"Unauthorized\"}"); return; }
        WebServerResponseWriter res(_server);
        handleApiPlatform(res, _server.arg("p"));
    });
    _server.on("/api/ntp/sync", HTTP_POST, [this]() {
        if (!isAuthenticated()) { _server.send(401, "application/json", "{\"error\":\"Unauthorized\"}"); return; }
        triggerNtpSync();
        _server.send(200, "application/json", "{\"success\":true}");
    });

    _server.onNotFound([this]() { handleNotFound(); });
}

void WebPortal::loop() {
    if ((WiFi.getMode() & WIFI_MODE_AP) && _captiveEnabled) {
        _dnsServer.processNextRequest();
    }
    _server.handleClient();
}

// =============================================================================
// HTML Helpers (PROGMEM Streaming)
// =============================================================================
void WebPortal::streamHeader(ResponseWriter &res, const char *activeTab, const char *title) {
    res.setContentType("text/html; charset=utf-8");

    time_t nowT = time(nullptr);
    char timeBuf[32];
    char clockTime[10] = "--:--:--";
    bool is24h = _prefs.getBool(NVS_KEY_TIME_FORMAT_24H, true);
    if (nowT > 1577836800) {
        struct tm ti;
        localtime_r(&nowT, &ti);
        strftime(clockTime, sizeof(clockTime), "%H:%M:%S", &ti);
        if (is24h) {
            strftime(timeBuf, sizeof(timeBuf), "%H:%M:%S", &ti);
        } else {
            strftime(timeBuf, sizeof(timeBuf), "%I:%M:%S %p", &ti);
        }
    } else {
        snprintf(timeBuf, sizeof(timeBuf), "--:--:--");
    }

    String hostname = getDeviceHostname(_prefs);

    res.sendChunk_P(PSTR("<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\">"
                         "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
                         "<title>"));
    res.sendChunk(title);
    res.sendChunk_P(PSTR(" - ESP-OOBM</title>"
                         "<link rel=\"icon\" type=\"image/svg+xml\" href=\"/favicon.svg\">"
                         "<link rel=\"icon\" type=\"image/x-icon\" href=\"/favicon.ico\">"
                         "<style>"));
    res.sendChunk_P(COMMON_CSS);
    res.sendChunk_P(PSTR("</style><script>"));
    res.sendChunk_P(COMMON_JS);
    res.sendChunk_P(PSTR("</script></head><body>"
                         "<div class=\"header-wrap\">"
                         "<div class=\"top-accent\"></div>"
                         "<nav class=\"navbar\"><div class=\"nav-inner\">"
                         "<a href=\"/\" class=\"brand\">"));
    res.sendChunk_P(OOBM_LOGO_SVG);
    res.sendChunk_P(PSTR("</a><div class=\"nav-links\">"));

    auto renderNavLink = [&](const char *tabId, const char *url, const char *label) {
        bool isActive = (strcmp(activeTab, tabId) == 0);
        res.sendChunk("<a href=\"");
        res.sendChunk(url);
        res.sendChunk("\" class=\"nav-link");
        if (isActive) res.sendChunk(" active");
        res.sendChunk("\">");
        res.sendChunk(label);
        res.sendChunk("</a>");
    };

    renderNavLink("dashboard", "/", "Dashboard");
    renderNavLink("terminal", "/terminal", "Terminal");
    renderNavLink("settings", "/settings", "Settings");

    res.sendChunk_P(PSTR("</div><div class=\"nav-actions\">"
                         "<div class=\"badge-clock\">"
                         "<svg width=\"14\" height=\"14\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\"><circle cx=\"12\" cy=\"12\" r=\"10\"/><polyline points=\"12 6 12 12 16 14\"/></svg>"
                         "<span id=\"header_clock\" data-time=\""));
    res.sendChunk(clockTime);
    res.sendChunk_P(PSTR("\" data-24h=\""));
    res.sendChunk(is24h ? "1" : "0");
    res.sendChunk_P(PSTR("\">"));
    res.sendChunk(timeBuf);
    res.sendChunk_P(PSTR("</span></div>"
                         "<div class=\"theme-switch\">"
                         "<button id=\"themeBtnLight\" class=\"theme-btn\" onclick=\"setTheme('light')\" title=\"Light Theme\">"
                         "<svg width=\"13\" height=\"13\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\"><circle cx=\"12\" cy=\"12\" r=\"4\"/><path d=\"M12 2v2M12 20v2M4.93 4.93l1.41 1.41M17.66 17.66l1.41 1.41M2 12h2M20 12h2M6.34 17.66l-1.41 1.41M19.07 4.93l-1.41 1.41\"/></svg>"
                         "</button>"
                         "<button id=\"themeBtnDark\" class=\"theme-btn\" onclick=\"setTheme('dark')\" title=\"Dark Theme\">"
                         "<svg width=\"13\" height=\"13\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\"><path d=\"M21 12.79A9 9 0 1 1 11.21 3 7 7 0 0 0 21 12.79z\"/></svg>"
                         "</button>"
                         "<button id=\"themeBtnSystem\" class=\"theme-btn\" onclick=\"setTheme('system')\" title=\"Follow System Theme\">"
                         "<svg width=\"13\" height=\"13\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\"><rect x=\"2\" y=\"3\" width=\"20\" height=\"14\" rx=\"2\"/><line x1=\"8\" y1=\"21\" x2=\"16\" y2=\"21\"/><line x1=\"12\" y1=\"17\" x2=\"12\" y2=\"21\"/></svg>"
                         "</button>"
                         "</div>"));

    if (_authRequired) {
        res.sendChunk_P(PSTR("<a href=\"/logout\" class=\"btn btn-outline btn-sm\" style=\"margin-left:4px;\" title=\"Sign Out\">"
                             "<svg width=\"14\" height=\"14\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\"><path d=\"M9 21H5a2 2 0 0 1-2-2V5a2 2 0 0 1 2-2h4\"/><polyline points=\"16 17 21 12 16 7\"/><line x1=\"21\" y1=\"12\" x2=\"9\" y2=\"12\"/></svg>"
                             "</a>"));
    }

    res.sendChunk_P(PSTR("</div></div></nav></div><div class=\"container\">"));

    if (WiFi.getMode() == WIFI_MODE_AP && !WiFi.isConnected()) {
        res.sendChunk_P(PSTR("<div class=\"ap-banner\">"
                             "<span>&#9888; <b>Device is currently in Standalone Access Point mode.</b> Connect to your local network to enable station management.</span>"
                             "<a href=\"/wifi\" class=\"btn btn-sm btn-outline\">Configure Wi-Fi &rarr;</a>"
                             "</div>"));
    }
}

void WebPortal::streamFooter(ResponseWriter &res) {
    res.sendChunk_P(PSTR("</div><script>(function(){"
                         "var el=document.getElementById('header_clock');"
                         "if(!el)return;"
                         "var value=el.getAttribute('data-time')||'';"
                         "if(!/^\\d{2}:\\d{2}:\\d{2}$/.test(value))return;"
                         "var p=value.split(':'),h=Number(p[0]),m=Number(p[1]),s=Number(p[2]);"
                         "var is24=el.getAttribute('data-24h')==='1';"
                         "function pad(n){return n<10?'0'+n:String(n);}"
                         "setInterval(function(){s++;if(s>=60){s=0;m++;if(m>=60){m=0;h=(h+1)%24;}}"
                         "var displayHour=h,suffix='';"
                         "if(!is24){suffix=h>=12?' PM':' AM';displayHour=h%12||12;}"
                         "el.textContent=pad(displayHour)+':'+pad(m)+':'+pad(s)+suffix;},1000);"
                         "})();</script>"
                         "<footer style=\"text-align:center;padding:24px 0 32px 0;font-size:0.80rem;color:var(--muted);border-top:1px solid var(--border);margin-top:40px;\">"
                         "Firmware v" FIRMWARE_VERSION
                         " &bull; Built " FIRMWARE_BUILD_DATE " " FIRMWARE_BUILD_TIME
                         " &bull; <span style=\"color:var(--green);font-weight:600;\">&#9889; Load: <span id=\"page_load_time\">-- ms</span></span>"
                         "</footer></body></html>"));
    res.end();
}

// =============================================================================
// Dashboard Page (/)
// =============================================================================
void WebPortal::renderRoot(ResponseWriter &res) {
    streamHeader(res, "dashboard", "Dashboard");

    SystemStatsData stats;
    SystemStats::update(stats);

    char uptimeBuf[32], heapBuf[16], minHeapBuf[16], rxBuf[16], txBuf[16], timeBuf[32], lastNtpBuf[40];
    SystemStats::formatUptime(stats.uptimeSeconds, uptimeBuf, sizeof(uptimeBuf));
    SystemStats::formatBytes(stats.freeHeapBytes, heapBuf, sizeof(heapBuf));
    SystemStats::formatBytes(stats.minFreeHeapBytes, minHeapBuf, sizeof(minHeapBuf));
    SystemStats::formatBytes(stats.serialRxBytes, rxBuf, sizeof(rxBuf));
    SystemStats::formatBytes(stats.serialTxBytes, txBuf, sizeof(txBuf));

    bool is24h = _prefs.getBool(NVS_KEY_TIME_FORMAT_24H, true);
    if (stats.currentTime > 1577836800) {
        struct tm ti;
        localtime_r(&stats.currentTime, &ti);
        if (is24h) {
            strftime(timeBuf, sizeof(timeBuf), "%Y-%m-%d %H:%M", &ti);
        } else {
            strftime(timeBuf, sizeof(timeBuf), "%Y-%m-%d %I:%M %p", &ti);
        }
    } else {
        snprintf(timeBuf, sizeof(timeBuf), "Unsynchronized (RTC uncalibrated)");
    }

    if (g_lastNtpSyncTimestamp > 0) {
        struct tm syncInfo;
        localtime_r(&g_lastNtpSyncTimestamp, &syncInfo);
        if (is24h) {
            strftime(lastNtpBuf, sizeof(lastNtpBuf), "%Y-%m-%d %H:%M", &syncInfo);
        } else {
            strftime(lastNtpBuf, sizeof(lastNtpBuf), "%Y-%m-%d %I:%M %p", &syncInfo);
        }
    } else {
        snprintf(lastNtpBuf, sizeof(lastNtpBuf), "Never / Just booted");
    }

    String tzCity = _prefs.getString(NVS_KEY_NTP_TZ_CITY, DEFAULT_TZ_CITY);
    String ntpServer = _prefs.getString(NVS_KEY_NTP_SERVER, NTP_DEFAULT_SERVER);

    char framing[8];
    snprintf(framing, sizeof(framing), "%u%c%u",
             serialBridge.getDataBits(),
             (serialBridge.getParity() == 1 ? 'O' : (serialBridge.getParity() == 2 ? 'E' : 'N')),
             serialBridge.getStopBits());

    String netMode = stats.wifiStaConnected ? "Station" : (stats.wifiApActive ? "Access Point" : "Disconnected");

    res.sendChunk_P(PSTR("<div class=\"grid-cards\">\n"));

    // 1. Health
    String card1 = "";
    card1.reserve(600);
    card1 += "  <div class=\"card\">\n";
    card1 += "    <h3>System Health <span id=\"cpu_badge\" style=\"font-size:0.75rem;color:var(--navy);\">" + String(stats.cpuFreqMhz) + " MHz</span></h3>\n";
    card1 += "    <div class=\"val val-accent\" id=\"uptime_val\">" + String(uptimeBuf) + "</div>\n";
    card1 += "    <div class=\"sub\">Uptime</div>\n";
    card1 += "    <div style=\"margin-top:14px;\">\n";
    card1 += "      <div class=\"stat-row\"><span class=\"stat-label\">Free RAM</span><span class=\"stat-val\"><span id=\"heap_val\">" + String(heapBuf) + "</span> <span id=\"frag_val\" style=\"color:var(--muted);font-size:0.80rem;\">(" + String(stats.heapFragPercent) + "% frag)</span></span></div>\n";
    card1 += "      <div class=\"stat-row\"><span class=\"stat-label\">Min Free Heap</span><span class=\"stat-val\" id=\"min_heap_val\">" + String(minHeapBuf) + "</span></div>\n";
    card1 += "      <div class=\"stat-row\"><span class=\"stat-label\">CPU Load / Temp</span><span class=\"stat-val\"><span id=\"cpu_load_val\">" + String(stats.cpuLoadPercent, 1) + "%</span> <span id=\"temp_val\" style=\"color:var(--muted);font-size:0.80rem;\">(" + String(stats.temperatureCelsius, 1) + " &deg;C)</span></span></div>\n";
    card1 += "    </div>\n";
    card1 += "  </div>\n";
    res.sendChunk(card1);

    // 2. Serial Interface
    String card2 = "";
    card2.reserve(600);
    card2 += "  <div class=\"card\">\n";
    card2 += "    <h3>Serial Interface <span id=\"baud_badge\" style=\"font-size:0.75rem;color:var(--green);\">" + String(stats.baudRate) + " " + String(framing) + "</span></h3>\n";
    card2 += "    <div class=\"val val-green\" id=\"rx_bytes_val\">" + String(rxBuf) + "</div>\n";
    card2 += "    <div class=\"sub\">RX Bytes Received</div>\n";
    card2 += "    <div style=\"margin-top:14px;\">\n";
    card2 += "      <div class=\"stat-row\"><span class=\"stat-label\">TX Bytes Sent</span><span class=\"stat-val\" id=\"tx_bytes_val\">" + String(txBuf) + "</span></div>\n";
    card2 += "      <div class=\"stat-row\"><span class=\"stat-label\">RX Overflows</span><span class=\"stat-val\" id=\"overflow_val\">" + String(stats.serialRxOverflow) + "</span></div>\n";
    card2 += "      <div class=\"stat-row\"><span class=\"stat-label\">Active Sessions</span><span class=\"stat-val\" id=\"sessions_val\">WS: " + String(stats.activeWsClients) + " | Telnet: " + String(stats.activeTelnetClients) + "</span></div>\n";
    card2 += "    </div>\n";
    card2 += "  </div>\n";
    res.sendChunk(card2);

    // 3. Network & WiFi
    String card3 = "";
    card3.reserve(600);
    card3 += "  <div class=\"card\">\n";
    card3 += "    <h3>Network & WiFi <span id=\"net_mode_badge\" style=\"font-size:0.75rem;color:var(--cyan);\">" + netMode + "</span></h3>\n";
    card3 += "    <div class=\"val\" id=\"ip_val\">" + String(stats.ipAddress) + "</div>\n";
    card3 += "    <div class=\"sub\" id=\"ssid_val\">SSID: " + (stats.ssid[0] ? String(stats.ssid) : "(None)") + "</div>\n";
    card3 += "    <div style=\"margin-top:14px;\">\n";
    card3 += "      <div class=\"stat-row\"><span class=\"stat-label\">MAC Address</span><span class=\"stat-val\" id=\"mac_val\">" + String(stats.macAddress) + "</span></div>\n";
    card3 += "      <div class=\"stat-row\"><span class=\"stat-label\">Signal RSSI</span><span class=\"stat-val\" id=\"rssi_val\">" + (stats.wifiStaConnected ? (String(stats.wifiRssi) + " dBm") : "N/A") + "</span></div>\n";
    card3 += "      <div class=\"stat-row\"><span class=\"stat-label\">Connected AP Clients</span><span class=\"stat-val\" id=\"ap_clients_val\">" + String(stats.wifiApClients) + "</span></div>\n";
    card3 += "    </div>\n";
    card3 += "  </div>\n";
    res.sendChunk(card3);

    // 4. Time & Clock
    String card4 = "";
    card4.reserve(600);
    card4 += "  <div class=\"card\">\n";
    card4 += "    <h3>Time &amp; Clock</h3>\n";
    card4 += "    <div class=\"val\" id=\"time_val\" style=\"font-size:1.6rem;letter-spacing:-0.02em;\">" + String(timeBuf) + "</div>\n";
    card4 += "    <div class=\"sub\" id=\"tz_val\">TZ: " + tzCity + "</div>\n";
    card4 += "    <div style=\"margin-top:14px;\">\n";
    card4 += "      <div class=\"stat-row\"><span class=\"stat-label\">NTP Status</span><span class=\"stat-val\"><span id=\"ntp_badge\" class=\"badge " + String(stats.ntpSynced ? "badge-ok\">Synced (OK)" : "badge-warn\">Waiting for sync") + "</span></span></div>\n";
    card4 += "      <div class=\"stat-row\"><span class=\"stat-label\">NTP Server</span><span class=\"stat-val\" id=\"ntp_srv_val\">" + ntpServer + "</span></div>\n";
    card4 += "      <div class=\"stat-row\"><span class=\"stat-label\">Last NTP Sync</span><span class=\"stat-val\" id=\"last_ntp_val\">" + String(lastNtpBuf) + "</span></div>\n";
    card4 += "    </div>\n";
    card4 += "  </div>\n";
    card4 += "</div>\n";
    res.sendChunk(card4);

    // 5. System Event Log Card
    res.sendChunk_P(PSTR(
        "<div class=\"table-card\" style=\"margin-top:20px;\">\n"
        "  <div style=\"display:flex;justify-content:space-between;align-items:center;margin-bottom:12px;\">\n"
        "    <h3 style=\"margin:0;color:var(--navy);\">&#128220; System Event Log</h3>\n"
        "    <button type=\"button\" class=\"btn btn-outline btn-sm\" onclick=\"fetchLogs()\">&#128260; Refresh Log</button>\n"
        "  </div>\n"
        "  <div id=\"log_box\" class=\"log-box\">"
    ));
    size_t lCount = logger.getCount();
    if (lCount == 0) {
        res.sendChunk_P(PSTR("<span style=\"color:var(--muted);\">No log entries recorded yet.</span>"));
    } else {
        for (size_t i = 0; i < lCount; i++) {
            const LogEntry &e = logger.getEntry(i);
            char lTimeBuf[32];
            if (e.timestamp > 1577836800) {
                time_t t = (time_t)e.timestamp;
                struct tm ti;
                localtime_r(&t, &ti);
                strftime(lTimeBuf, sizeof(lTimeBuf), "%H:%M:%S", &ti);
            } else {
                snprintf(lTimeBuf, sizeof(lTimeBuf), "+%us", e.timestamp);
            }
            const char *cls = (e.level == LOG_LVL_ERROR) ? "log-error" : ((e.level == LOG_LVL_WARN) ? "log-warn" : "log-info");
            const char *prefix = (e.level == LOG_LVL_ERROR) ? "[ERROR]" : ((e.level == LOG_LVL_WARN) ? "[WARN]" : "[INFO]");
            String line = "<span style=\"color:var(--muted);\">" + String(lTimeBuf) + "</span> <span class=\"" + String(cls) + "\">" + String(prefix) + "</span> " + String(e.msg) + "\n";
            res.sendChunk(line);
        }
    }
    res.sendChunk_P(PSTR("</div>\n</div>\n"));

    res.sendChunk_P(PSTR(
        "<script>\n"
        "function fetchLogs(){\n"
        "  fetch('/api/logs').then(r=>r.json()).then(entries=>{\n"
        "    var box = document.getElementById('log_box');\n"
        "    if(!box) return;\n"
        "    if(!entries || entries.length === 0){ box.innerHTML = '<span style=\"color:var(--muted);\">No log entries recorded yet.</span>'; return; }\n"
        "    var html = '';\n"
        "    entries.forEach(function(e){\n"
        "      var cls = e.lvl === 2 ? 'log-error' : (e.lvl === 1 ? 'log-warn' : 'log-info');\n"
        "      var prefix = e.lvl === 2 ? '[ERROR]' : (e.lvl === 1 ? '[WARN]' : '[INFO]');\n"
        "      html += '<span style=\"color:var(--muted);\">' + e.time + '</span> ' +\n"
        "              '<span class=\"' + cls + '\">' + prefix + '</span> ' +\n"
        "              e.msg + '\\n';\n"
        "    });\n"
        "    box.innerHTML = html;\n"
        "    box.scrollTop = box.scrollHeight;\n"
        "  }).catch(()=>{});\n"
        "}\n"
        "(function(){ var b = document.getElementById('log_box'); if(b) b.scrollTop = b.scrollHeight; })();\n"
        "function updateDashboardUI(d){\n"
        "  document.getElementById('uptime_val').innerText = d.uptime_str || '--';\n"
        "  document.getElementById('cpu_badge').innerText = d.cpu_freq + ' MHz';\n"
        "  document.getElementById('heap_val').innerText = d.free_heap_str || (Math.round(d.free_heap/1024) + ' KB');\n"
        "  if(document.getElementById('frag_val')) document.getElementById('frag_val').innerText = '(' + (d.heap_frag || 0) + '% frag)';\n"
        "  document.getElementById('min_heap_val').innerText = d.min_heap_str || (Math.round(d.min_free_heap/1024) + ' KB');\n"
        "  if(document.getElementById('cpu_load_val')) document.getElementById('cpu_load_val').innerText = (d.cpu_load !== undefined ? d.cpu_load.toFixed(1) : '0.0') + '%';\n"
        "  if(document.getElementById('temp_val')) document.getElementById('temp_val').innerText = '(' + (d.temp_c !== undefined ? d.temp_c.toFixed(1) : '--') + ' °C)';\n"
        "  document.getElementById('baud_badge').innerText = d.baud + ' ' + d.framing;\n"
        "  document.getElementById('rx_bytes_val').innerText = d.rx_bytes_str || (d.rx_bytes + ' B');\n"
        "  document.getElementById('tx_bytes_val').innerText = d.tx_bytes_str || (d.tx_bytes + ' B');\n"
        "  document.getElementById('sessions_val').innerText = 'WS: ' + d.active_ws + ' | Telnet: ' + d.active_telnet;\n"
        "  document.getElementById('net_mode_badge').innerText = d.net_mode;\n"
        "  document.getElementById('ip_val').innerText = d.ip;\n"
        "  document.getElementById('ssid_val').innerText = 'SSID: ' + (d.ssid || '(None)');\n"
        "  document.getElementById('mac_val').innerText = d.mac;\n"
        "  document.getElementById('rssi_val').innerText = d.rssi ? (d.rssi + ' dBm') : 'N/A';\n"
        "  document.getElementById('ap_clients_val').innerText = d.ap_clients || 0;\n"
        "  document.getElementById('ntp_badge').innerText = d.ntp_synced ? 'Synced (OK)' : 'Waiting for sync';\n"
        "  document.getElementById('ntp_badge').className = d.ntp_synced ? 'badge badge-ok' : 'badge badge-warn';\n"
        "  document.getElementById('time_val').innerText = d.time_str || '--:--';\n"
        "  document.getElementById('tz_val').innerText = 'TZ: ' + (d.tz_city || 'UTC');\n"
        "  document.getElementById('ntp_srv_val').innerText = d.ntp_server || 'pool.ntp.org';\n"
        "  document.getElementById('last_ntp_val').innerText = d.last_ntp_str || '--';\n"
        "}\n"
        "var statusPollTimer = null;\n"
        "var statusPollInFlight = false;\n"
        "function pollDashboardStatus(){\n"
        "  if(document.hidden || statusPollInFlight) return;\n"
        "  statusPollInFlight = true;\n"
        "  fetch('/api/status').then(r=>r.json()).then(updateDashboardUI).catch(()=>{}).then(function(){\n"
        "    statusPollInFlight = false;\n"
        "    if(!document.hidden) statusPollTimer = setTimeout(pollDashboardStatus, " WEBPORTAL_STRINGIFY(DASHBOARD_AJAX_REFRESH_MS) ");\n"
        "  });\n"
        "}\n"
        "document.addEventListener('visibilitychange', function(){\n"
        "  if(statusPollTimer){ clearTimeout(statusPollTimer); statusPollTimer = null; }\n"
        "  if(!document.hidden) pollDashboardStatus();\n"
        "});\n"
        "pollDashboardStatus();\n"
        "</script>\n"
    ));
#undef WEBPORTAL_STRINGIFY
#undef WEBPORTAL_STRINGIFY_IMPL

    streamFooter(res);
}

// =============================================================================
// Terminal Page (/terminal)
// =============================================================================
void WebPortal::renderTerminal(ResponseWriter &res) {
    streamHeader(res, "terminal", "Serial Console");

    String curPlatform = _prefs.getString(NVS_KEY_CLI_PLATFORM, "mikrotik");

    // Top Controls Bar & Responsive Terminal Screen
    res.sendChunk_P(PSTR(
        "<div style=\"display:flex;justify-content:space-between;align-items:center;flex-wrap:wrap;gap:8px;margin-bottom:10px;\">\n"
        "  <div style=\"font-size:0.88rem;font-weight:600;display:flex;align-items:center;gap:8px;white-space:nowrap;\">\n"
        "    <span id=\"ws_status_dot\" style=\"display:inline-block;width:10px;height:10px;border-radius:50%;background:var(--warn);\"></span>\n"
        "    <span id=\"ws_status_text\">Connecting...</span>\n"
        "  </div>\n"
        "  <div style=\"display:flex;align-items:center;gap:4px;flex-wrap:wrap;\">\n"
        "    <span style=\"font-size:0.75rem;font-weight:700;color:var(--muted);margin-right:2px;\">KEYS:</span>\n"
        "    <button class=\"term-key\" onclick=\"sendSpecialKey(27)\">ESC</button>\n"
        "    <button class=\"term-key\" onclick=\"sendSpecialKey(9)\">TAB</button>\n"
        "    <button class=\"term-key\" onclick=\"sendSpecialKey(3)\">Ctrl+C</button>\n"
        "    <button class=\"term-key\" onclick=\"sendSpecialKey(26)\">Ctrl+Z</button>\n"
        "    <button class=\"term-key\" onclick=\"sendSpecialKey(4)\">Ctrl+D</button>\n"
        "    <button class=\"term-key\" onclick=\"sendEscapeSeq('[A')\">&uarr;</button>\n"
        "    <button class=\"term-key\" onclick=\"sendEscapeSeq('[B')\">&darr;</button>\n"
        "    <button class=\"term-key\" onclick=\"sendEscapeSeq('[D')\">&larr;</button>\n"
        "    <button class=\"term-key\" onclick=\"sendEscapeSeq('[C')\">&rarr;</button>\n"
        "    <button class=\"term-key\" onclick=\"sendSpecialKey(13)\">ENTER</button>\n"
        "  </div>\n"
        "  <div style=\"display:flex;gap:6px;\">\n"
        "    <button class=\"btn btn-outline btn-sm\" onclick=\"clearTerminal()\">Clear</button>\n"
        "    <button class=\"btn btn-outline btn-sm\" onclick=\"reconnectWs()\">Reconnect</button>\n"
        "  </div>\n"
        "</div>\n"
        "<div id=\"terminal\" class=\"term-container\" tabindex=\"0\"></div>\n"
        "<div class=\"table-card\" style=\"margin-top:10px;padding:8px 14px;display:flex;align-items:center;gap:14px;flex-wrap:wrap;\">\n"
        "  <div style=\"display:flex;align-items:center;gap:6px;\">\n"
        "    <span style=\"font-size:0.80rem;font-weight:700;color:var(--muted);white-space:nowrap;\">PLATFORM:</span>\n"
        "    <select id=\"platform_select\" class=\"form-control\" style=\"padding:4px 8px;font-size:0.82rem;font-weight:600;width:auto;\" onchange=\"changePlatform(this.value)\">\n"
    ));

    res.sendChunk(String("      <option value=\"mikrotik\"") + (curPlatform == "mikrotik" ? " selected" : "") + ">MikroTik RouterOS</option>\n");
    res.sendChunk(String("      <option value=\"linux\"") + (curPlatform == "linux" ? " selected" : "") + ">Linux / OpenWrt / EdgeOS</option>\n");
    res.sendChunk(String("      <option value=\"cisco\"") + (curPlatform == "cisco" ? " selected" : "") + ">Cisco IOS / Catalyst</option>\n");
    res.sendChunk(String("      <option value=\"pfsense\"") + (curPlatform == "pfsense" ? " selected" : "") + ">pfSense / FreeBSD / OPNsense</option>\n");
    res.sendChunk(String("      <option value=\"generic\"") + (curPlatform == "generic" ? " selected" : "") + ">Generic Diagnostics</option>\n");

    res.sendChunk_P(PSTR(
        "    </select>\n"
        "  </div>\n"
        "  <div style=\"display:flex;align-items:center;gap:6px;flex-wrap:wrap;\">\n"
        "    <span style=\"font-size:0.80rem;font-weight:700;color:var(--muted);margin-right:2px;\">QUICK COMMANDS:</span>\n"
        "    <div id=\"quick_cmds\" style=\"display:inline-flex;gap:6px;flex-wrap:wrap;\"></div>\n"
        "  </div>\n"
        "</div>\n"
        "<script>\n"
        "var ANSI_COLORS=['#0f172a','#ef4444','#10b981','#f59e0b','#3b82f6','#a855f7','#06b6d4','#cbd5e1',"
        "'#64748b','#f87171','#34d399','#fbbf24','#60a5fa','#c084fc','#22d3ee','#ffffff'];\n"
        "function getAnsiColor(code){\n"
        "  if(code>=0&&code<16) return ANSI_COLORS[code];\n"
        "  if(code>=232&&code<=255){\n"
        "    var v=Math.round((code-232)*10+8);\n"
        "    var h=v.toString(16).padStart(2,'0');\n"
        "    return '#'+h+h+h;\n"
        "  }\n"
        "  if(code>=16&&code<=231){\n"
        "    var n=code-16,b=n%6,g=Math.floor(n/6)%6,r=Math.floor(n/36);\n"
        "    var steps=[0,95,135,175,215,255];\n"
        "    return 'rgb('+steps[r]+','+steps[g]+','+steps[b]+')';\n"
        "  }\n"
        "  return null;\n"
        "}\n"
        "function AnsiTerminal(el,maxLines){\n"
        "  this.el=el;\n"
        "  this.maxLines=maxLines||1500;\n"
        "  this.lines=[[]];\n"
        "  this.cursorRow=0;\n"
        "  this.cursorCol=0;\n"
        "  this.fg=null; this.bg=null; this.bold=false; this.underline=false; this.inverse=false;\n"
        "  this.state=0;\n"
        "  this.csiParamStr='';\n"
        "  this.renderTimer=null;\n"
        "}\n"
        "AnsiTerminal.prototype.reset=function(){\n"
        "  this.lines=[[]]; this.cursorRow=0; this.cursorCol=0;\n"
        "  this.fg=null; this.bg=null; this.bold=false; this.underline=false; this.inverse=false;\n"
        "  this.state=0; this.csiParamStr='';\n"
        "  this.render();\n"
        "};\n"
        "AnsiTerminal.prototype.write=function(str){\n"
        "  for(var i=0;i<str.length;i++){\n"
        "    var ch=str.charAt(i),code=str.charCodeAt(i);\n"
        "    if(this.state===0){\n"
        "      if(code===27){ this.state=1; }\n"
        "      else if(code===13){ this.cursorCol=0; }\n"
        "      else if(code===10){\n"
        "        this.cursorRow=this.lines.length-1;\n"
        "        this.lines.push([]);\n"
        "        this.cursorRow=this.lines.length-1;\n"
        "        this.cursorCol=0;\n"
        "        if(this.lines.length>this.maxLines){\n"
        "          this.lines.shift();\n"
        "          this.cursorRow=Math.max(0,this.lines.length-1);\n"
        "        }\n"
        "      }\n"
        "      else if(code===8||code===127){ if(this.cursorCol>0) this.cursorCol--; }\n"
        "      else if(code===9){\n"
        "        var nextTab=(Math.floor(this.cursorCol/8)+1)*8;\n"
        "        while(this.cursorCol<nextTab){ this.putChar(' '); }\n"
        "      }\n"
        "      else if(code>=32){ this.putChar(ch); }\n"
        "    }else if(this.state===1){\n"
        "      if(ch==='['){ this.state=2; this.csiParamStr=''; }\n"
        "      else if(ch===']'){ this.state=3; }\n"
        "      else if(ch==='('||ch===')'){ this.state=4; }\n"
        "      else if(ch==='c'){ this.reset(); this.state=0; }\n"
        "      else if(ch==='7'){ this.savedRow=this.cursorRow; this.savedCol=this.cursorCol; this.state=0; }\n"
        "      else if(ch==='8'){ if(this.savedRow!==undefined) this.cursorRow=this.savedRow; if(this.savedCol!==undefined) this.cursorCol=this.savedCol; this.state=0; }\n"
        "      else { this.state=0; }\n"
        "    }else if(this.state===4){\n"
        "      this.state=0;\n"
        "    }else if(this.state===3){\n"
        "      if(code===7){ this.state=0; }\n"
        "      else if(code===27){ this.state=1; }\n"
        "    }else if(this.state===2){\n"
        "      if((this.csiParamStr===''||this.csiParamStr==='?')&&(ch==='?'||ch==='>'||ch==='=')){ this.csiParamStr+=ch; }\n"
        "      else if((code>=48&&code<=57)||ch===';'||ch==='?'){ this.csiParamStr+=ch; }\n"
        "      else { this.handleCsi(ch); this.state=0; }\n"
        "    }\n"
        "  }\n"
        "  this.scheduleRender();\n"
        "};\n"
        "AnsiTerminal.prototype.putChar=function(ch){\n"
        "  while(this.lines.length<=this.cursorRow){ this.lines.push([]); }\n"
        "  var line=this.lines[this.cursorRow];\n"
        "  while(line.length<this.cursorCol){\n"
        "    line.push({ch:' ',fg:null,bg:null,bold:false,underline:false,inverse:false});\n"
        "  }\n"
        "  line[this.cursorCol]={ch:ch,fg:this.fg,bg:this.bg,bold:this.bold,underline:this.underline,inverse:this.inverse};\n"
        "  this.cursorCol++;\n"
        "};\n"
        "AnsiTerminal.prototype.handleCsi=function(cmd){\n"
        "  var cleanStr=this.csiParamStr.replace(/^[\\?\\>\\=]+/,'');\n"
        "  var raw=cleanStr?cleanStr.split(';'):[];\n"
        "  var p=raw.map(function(x){ return parseInt(x,10); });\n"
        "  if(cmd==='m'){\n"
        "    if(p.length===0) p=[0];\n"
        "    for(var i=0;i<p.length;i++){\n"
        "      var v=isNaN(p[i])?0:p[i];\n"
        "      if(v===0){ this.fg=null; this.bg=null; this.bold=false; this.underline=false; this.inverse=false; }\n"
        "      else if(v===1){ this.bold=true; }\n"
        "      else if(v===4){ this.underline=true; }\n"
        "      else if(v===7){ this.inverse=true; }\n"
        "      else if(v===22){ this.bold=false; }\n"
        "      else if(v===24){ this.underline=false; }\n"
        "      else if(v===27){ this.inverse=false; }\n"
        "      else if(v>=30&&v<=37){ this.fg=getAnsiColor(v-30); }\n"
        "      else if(v===39){ this.fg=null; }\n"
        "      else if(v>=40&&v<=47){ this.bg=getAnsiColor(v-40); }\n"
        "      else if(v===49){ this.bg=null; }\n"
        "      else if(v>=90&&v<=97){ this.fg=getAnsiColor(v-90+8); }\n"
        "      else if(v>=100&&v<=107){ this.bg=getAnsiColor(v-100+8); }\n"
        "      else if(v===38&&p[i+1]===5){ this.fg=getAnsiColor(p[i+2]); i+=2; }\n"
        "      else if(v===48&&p[i+1]===5){ this.bg=getAnsiColor(p[i+2]); i+=2; }\n"
        "      else if(v===38&&p[i+1]===2){ this.fg='rgb('+p[i+2]+','+p[i+3]+','+p[i+4]+')'; i+=4; }\n"
        "      else if(v===48&&p[i+1]===2){ this.bg='rgb('+p[i+2]+','+p[i+3]+','+p[i+4]+')'; i+=4; }\n"
        "    }\n"
        "  }else if(cmd==='K'){\n"
        "    var m=p[0]||0;\n"
        "    while(this.lines.length<=this.cursorRow) this.lines.push([]);\n"
        "    var line=this.lines[this.cursorRow];\n"
        "    if(m===0){\n"
        "      if(line.length>this.cursorCol) line.length=this.cursorCol;\n"
        "    }else if(m===1){\n"
        "      for(var c=0;c<=Math.min(this.cursorCol,line.length-1);c++){\n"
        "        line[c]={ch:' ',fg:null,bg:null,bold:false,underline:false,inverse:false};\n"
        "      }\n"
        "    }else if(m===2){\n"
        "      this.lines[this.cursorRow]=[];\n"
        "      this.cursorCol=0;\n"
        "    }\n"
        "  }else if(cmd==='J'){\n"
        "    var m=p[0]||0;\n"
        "    if(m===2||m===3){\n"
        "      this.lines.push([]);\n"
        "      this.cursorRow=this.lines.length-1;\n"
        "      this.cursorCol=0;\n"
        "    }else if(m===0){\n"
        "      while(this.lines.length<=this.cursorRow) this.lines.push([]);\n"
        "      var cur=this.lines[this.cursorRow];\n"
        "      if(cur&&cur.length>this.cursorCol) cur.length=this.cursorCol;\n"
        "    }\n"
        "  }else if(cmd==='A'){\n"
        "    this.cursorRow=Math.max(0,this.cursorRow-(p[0]||1));\n"
        "  }else if(cmd==='B'){\n"
        "    this.cursorRow=Math.min(this.lines.length-1,this.cursorRow+(p[0]||1));\n"
        "  }else if(cmd==='C'){\n"
        "    this.cursorCol+=(p[0]||1);\n"
        "  }else if(cmd==='D'){\n"
        "    this.cursorCol=Math.max(0,this.cursorCol-(p[0]||1));\n"
        "  }else if(cmd==='G'||cmd==='`'){\n"
        "    this.cursorCol=Math.max(0,(p[0]||1)-1);\n"
        "  }else if(cmd==='H'||cmd==='f'){\n"
        "    var c=p[1];\n"
        "    if(c!==undefined){\n"
        "      this.cursorCol=Math.max(0,c-1);\n"
        "    }else{\n"
        "      this.cursorCol=0;\n"
        "    }\n"
        "  }else if(cmd==='c'){\n"
        "    if(ws&&ws.readyState===1){\n"
        "      ws.send(new TextEncoder().encode('\\x1b[?1;2c'));\n"
        "    }\n"
        "  }\n"
        "};\n"
        "AnsiTerminal.prototype.scheduleRender=function(){\n"
        "  if(this.renderTimer) return;\n"
        "  var self=this;\n"
        "  this.renderTimer=requestAnimationFrame(function(){ self.renderTimer=null; self.render(); });\n"
        "};\n"
        "AnsiTerminal.prototype.render=function(){\n"
        "  var html='';\n"
        "  for(var r=0;r<this.lines.length;r++){\n"
        "    var line=this.lines[r], isCur=(r===this.cursorRow);\n"
        "    if(!line||line.length===0){\n"
        "      html+=isCur?'<span class=\"term-cursor\">&nbsp;</span>\\n':'\\n';\n"
        "      continue;\n"
        "    }\n"
        "    var curStyle=null, spanBuf='', maxCol=Math.max(line.length,isCur?(this.cursorCol+1):line.length);\n"
        "    for(var c=0;c<maxCol;c++){\n"
        "      if(isCur&&c===this.cursorCol){\n"
        "        if(spanBuf){ html+=this.wrapSpan(spanBuf,curStyle); spanBuf=''; curStyle=null; }\n"
        "        var curChar=(line[c]&&line[c].ch)?line[c].ch:'&nbsp;';\n"
        "        html+='<span class=\"term-cursor\">'+(curChar===' '?'&nbsp;':this.escapeChar(curChar))+'</span>';\n"
        "        continue;\n"
        "      }\n"
        "      var cell=line[c]||{ch:' ',fg:null,bg:null,bold:false,underline:false,inverse:false};\n"
        "      var ch=cell.ch||' ';\n"
        "      var fg=cell.fg, bg=cell.bg;\n"
        "      if(cell.inverse){ var tmp=fg||'#cbd5e1'; fg=bg||'#000000'; bg=tmp; }\n"
        "      var sk=(fg||'')+'|'+(bg||'')+'|'+(cell.bold?'1':'0')+'|'+(cell.underline?'1':'0');\n"
        "      if(curStyle!==sk){\n"
        "        if(spanBuf){ html+=this.wrapSpan(spanBuf,curStyle); spanBuf=''; }\n"
        "        curStyle=sk;\n"
        "      }\n"
        "      spanBuf+=this.escapeChar(ch);\n"
        "    }\n"
        "    if(spanBuf) html+=this.wrapSpan(spanBuf,curStyle);\n"
        "    if(isCur&&this.cursorCol>=maxCol) html+='<span class=\"term-cursor\">&nbsp;</span>';\n"
        "    html+='\\n';\n"
        "  }\n"
        "  this.el.innerHTML=html;\n"
        "  this.el.scrollTop=this.el.scrollHeight;\n"
        "};\n"
        "AnsiTerminal.prototype.escapeChar=function(c){\n"
        "  if(c==='&') return '&amp;'; if(c==='<') return '&lt;'; if(c==='>') return '&gt;'; if(c==='\"') return '&quot;'; return c;\n"
        "};\n"
        "AnsiTerminal.prototype.wrapSpan=function(text,sk){\n"
        "  if(!sk) return text;\n"
        "  var p=sk.split('|'), fg=p[0], bg=p[1], bold=p[2]==='1', und=p[3]==='1';\n"
        "  if(!fg&&!bg&&!bold&&!und) return text;\n"
        "  var s='';\n"
        "  if(fg) s+='color:'+fg+';';\n"
        "  if(bg) s+='background-color:'+bg+';';\n"
        "  if(bold) s+='font-weight:700;';\n"
        "  if(und) s+='text-decoration:underline;';\n"
        "  return '<span style=\"'+s+'\">'+text+'</span>';\n"
        "};\n\n"
        "var ws, termEl=document.getElementById('terminal'), dot=document.getElementById('ws_status_dot'), stext=document.getElementById('ws_status_text');\n"
        "var emulator = new AnsiTerminal(termEl, 1200);\n\n"
        "try{\n"
        "  var savedRaw = sessionStorage.getItem('oobm_term_raw') || '';\n"
        "  if(savedRaw.length > 0) emulator.write(savedRaw);\n"
        "}catch(e){}\n\n"
        "var PLATFORM_CMDS = {\n"
        "  mikrotik: [\n"
        "    { label: '/system resource print', cmd: '/system resource print' },\n"
        "    { label: '/ip address print', cmd: '/ip address print' },\n"
        "    { label: '/interface print', cmd: '/interface print' },\n"
        "    { label: '/log print', cmd: '/log print follow-only' },\n"
        "    { label: '/ip route print', cmd: '/ip route print' }\n"
        "  ],\n"
        "  linux: [\n"
        "    { label: 'ip addr show', cmd: 'ip addr show' },\n"
        "    { label: 'dmesg | tail -n 20', cmd: 'dmesg -T | tail -n 20' },\n"
        "    { label: 'logread -f', cmd: 'logread -f' },\n"
        "    { label: 'systemctl status', cmd: 'systemctl status' },\n"
        "    { label: 'df -h', cmd: 'df -h' }\n"
        "  ],\n"
        "  cisco: [\n"
        "    { label: 'show ip int brief', cmd: 'show ip interface brief' },\n"
        "    { label: 'show running-config', cmd: 'show running-config' },\n"
        "    { label: 'show int status', cmd: 'show interfaces status' },\n"
        "    { label: 'show logging', cmd: 'show logging' },\n"
        "    { label: 'term len 0', cmd: 'terminal length 0' }\n"
        "  ],\n"
        "  pfsense: [\n"
        "    { label: 'ifconfig', cmd: 'ifconfig' },\n"
        "    { label: 'pfctl -d (Disable FW)', cmd: 'pfctl -d' },\n"
        "    { label: 'pfctl -e (Enable FW)', cmd: 'pfctl -e' },\n"
        "    { label: 'top -b', cmd: 'top -b' },\n"
        "    { label: 'netstat -rn', cmd: 'netstat -rn' }\n"
        "  ],\n"
        "  generic: [\n"
        "    { label: 'help', cmd: 'help' },\n"
        "    { label: 'status', cmd: 'status' },\n"
        "    { label: 'version', cmd: 'version' },\n"
        "    { label: 'ping 8.8.8.8', cmd: 'ping 8.8.8.8' },\n"
        "    { label: 'reboot', cmd: 'reboot' }\n"
        "  ]\n"
        "};\n"
        "function renderQuickCmds(p){\n"
        "  var list = PLATFORM_CMDS[p] || PLATFORM_CMDS.mikrotik;\n"
        "  var c = document.getElementById('quick_cmds');\n"
        "  if(!c) return;\n"
        "  c.innerHTML = '';\n"
        "  list.forEach(function(item){\n"
        "    var btn = document.createElement('button');\n"
        "    btn.className = 'btn btn-outline btn-sm';\n"
        "    btn.innerText = item.label;\n"
        "    btn.onclick = function(){ sendCmd(item.cmd); };\n"
        "    c.appendChild(btn);\n"
        "  });\n"
        "}\n"
        "function changePlatform(p){\n"
        "  renderQuickCmds(p);\n"
        "  try{ localStorage.setItem('oobm_cli_platform', p); }catch(e){}\n"
        "  fetch('/api/platform?p=' + encodeURIComponent(p), { method: 'POST' }).catch(()=>{});\n"
        "}\n"
        "var wsRetryTimer = null;\n"
        "function initWs(){\n"
        "  if(wsRetryTimer){ clearTimeout(wsRetryTimer); wsRetryTimer = null; }\n"
        "  var wsUrl = 'ws://' + location.hostname + ':81';\n"
        "  try{ if(ws) ws.close(); }catch(e){}\n"
        "  ws = new WebSocket(wsUrl);\n"
        "  ws.binaryType = 'arraybuffer';\n"
        "  ws.onopen = function(){\n"
        "    if(wsRetryTimer){ clearTimeout(wsRetryTimer); wsRetryTimer = null; }\n"
        "    dot.style.background = 'var(--green)';\n"
        "    stext.innerText = 'Connected (WS / Port 81)';\n"
    ));

    if (_authRequired) {
        String authStr = "    ws.send('AUTH:" + String(_authUser) + ":" + String(_authPass) + "');\n";
        res.sendChunk(authStr);
    }

    res.sendChunk_P(PSTR(
        "  };\n"
        "  ws.onclose = function(){\n"
        "    dot.style.background = 'var(--danger)';\n"
        "    stext.innerText = 'Disconnected (Reconnecting...)';\n"
        "    if(!wsRetryTimer){\n"
        "      wsRetryTimer = setTimeout(function(){ wsRetryTimer = null; initWs(); }, 2000);\n"
        "    }\n"
        "  };\n"
        "  ws.onerror = function(){\n"
        "    dot.style.background = 'var(--danger)';\n"
        "    stext.innerText = 'Connecting / Error...';\n"
        "  };\n"
        "  ws.onmessage = function(e){\n"
        "    var text = '';\n"
        "    if(e.data instanceof ArrayBuffer){\n"
        "      text = new TextDecoder().decode(e.data);\n"
        "    } else { text = e.data; }\n"
        "    try{\n"
        "      var prev = sessionStorage.getItem('oobm_term_raw') || '';\n"
        "      if(prev.length + text.length > 300000){\n"
        "        prev = prev.substring(prev.length - 200000);\n"
        "      }\n"
        "      sessionStorage.setItem('oobm_term_raw', prev + text);\n"
        "    }catch(err){}\n"
        "    emulator.write(text);\n"
        "  };\n"
        "}\n"
        "function reconnectWs(){ if(wsRetryTimer){ clearTimeout(wsRetryTimer); wsRetryTimer = null; } initWs(); }\n"
        "function sendSpecialKey(code){ if(ws && ws.readyState === 1){ ws.send(new Uint8Array([code])); } }\n"
        "function sendEscapeSeq(seq){\n"
        "  if(ws && ws.readyState === 1){\n"
        "    var bytes = [27];\n"
        "    for(var i=0; i<seq.length; i++) bytes.push(seq.charCodeAt(i));\n"
        "    ws.send(new Uint8Array(bytes));\n"
        "  }\n"
        "}\n"
        "function sendCmd(cmd){ if(ws && ws.readyState === 1){ ws.send(cmd + '\\r\\n'); } }\n"
        "function clearTerminal(){\n"
        "  try{ sessionStorage.removeItem('oobm_term_raw'); }catch(e){}\n"
        "  emulator.reset();\n"
        "}\n"
        "function sendPasteData(clipText){\n"
        "  if(!clipText || !ws || ws.readyState !== 1) return;\n"
        "  var lines = clipText.replace(/\\r\\n/g, '\\n').replace(/\\r/g, '\\n').split('\\n');\n"
        "  if(lines.length <= 1){\n"
        "    ws.send(clipText);\n"
        "    return;\n"
        "  }\n"
        "  var idx = 0;\n"
        "  function sendNext(){\n"
        "    if(idx >= lines.length || !ws || ws.readyState !== 1) return;\n"
        "    var l = lines[idx];\n"
        "    idx++;\n"
        "    var isLast = (idx >= lines.length);\n"
        "    var hasTrailing = (clipText.endsWith('\\n') || clipText.endsWith('\\r'));\n"
        "    ws.send(l + (isLast && !hasTrailing ? '' : '\\r'));\n"
        "    if(!isLast) setTimeout(sendNext, 25);\n"
        "  }\n"
        "  sendNext();\n"
        "}\n"
        "termEl.addEventListener('paste', function(e){\n"
        "  e.preventDefault();\n"
        "  var clipText = (e.clipboardData || window.clipboardData).getData('text');\n"
        "  sendPasteData(clipText);\n"
        "});\n"
        "window.addEventListener('paste', function(e){\n"
        "  if(document.activeElement === termEl || termEl.contains(document.activeElement)){\n"
        "    e.preventDefault();\n"
        "    var clipText = (e.clipboardData || window.clipboardData).getData('text');\n"
        "    sendPasteData(clipText);\n"
        "  }\n"
        "});\n"
        "termEl.addEventListener('keydown', function(e){\n"
        "  if(e.key === 'Backspace'){ e.preventDefault(); sendSpecialKey(8); return; }\n"
        "  if(e.key === 'Tab'){ e.preventDefault(); sendSpecialKey(9); return; }\n"
        "  if(e.key === 'Enter'){ e.preventDefault(); sendSpecialKey(13); return; }\n"
        "  if(e.key === 'ArrowUp'){ e.preventDefault(); sendEscapeSeq('[A'); return; }\n"
        "  if(e.key === 'ArrowDown'){ e.preventDefault(); sendEscapeSeq('[B'); return; }\n"
        "  if(e.key === 'ArrowLeft'){ e.preventDefault(); sendEscapeSeq('[D'); return; }\n"
        "  if(e.key === 'ArrowRight'){ e.preventDefault(); sendEscapeSeq('[C'); return; }\n"
        "  if(e.key === 'Delete'){ e.preventDefault(); sendEscapeSeq('[3~'); return; }\n"
        "  if(e.key === 'Home'){ e.preventDefault(); sendEscapeSeq('[H'); return; }\n"
        "  if(e.key === 'End'){ e.preventDefault(); sendEscapeSeq('[F'); return; }\n"
        "  if(e.key === 'PageUp'){ e.preventDefault(); sendEscapeSeq('[5~'); return; }\n"
        "  if(e.key === 'PageDown'){ e.preventDefault(); sendEscapeSeq('[6~'); return; }\n"
        "  if(e.ctrlKey && !e.altKey && !e.metaKey){\n"
        "    var k = e.key.toLowerCase();\n"
        "    if(k === 'c'){\n"
        "      var sel = window.getSelection().toString();\n"
        "      if(sel.length > 0) return;\n"
        "      e.preventDefault(); sendSpecialKey(3); return;\n"
        "    }\n"
        "    if(k === 'v'){\n"
        "      if(navigator.clipboard && navigator.clipboard.readText){\n"
        "        e.preventDefault();\n"
        "        navigator.clipboard.readText().then(function(t){ sendPasteData(t); }).catch(function(){});\n"
        "        return;\n"
        "      }\n"
        "      return;\n"
        "    }\n"
        "    if(k.length === 1 && k >= 'a' && k <= 'z'){\n"
        "      e.preventDefault();\n"
        "      var code = k.charCodeAt(0) - 96;\n"
        "      sendSpecialKey(code);\n"
        "      return;\n"
        "    }\n"
        "  }\n"
        "  if(e.key.length === 1 && !e.ctrlKey && !e.altKey && !e.metaKey){\n"
        "    e.preventDefault();\n"
        "    if(ws && ws.readyState === 1) ws.send(e.key);\n"
        "  }\n"
        "});\n"
        "renderQuickCmds(document.getElementById('platform_select').value);\n"
        "initWs();\n"
        "</script>\n"
    ));

    streamFooter(res);
}

// =============================================================================
// Settings Page (/settings)
// =============================================================================
void WebPortal::renderSettings(ResponseWriter &res) {
    streamHeader(res, "settings", "Settings");

    String devHost = getDeviceHostname(_prefs);
    bool staticEn = _prefs.getBool(NVS_KEY_WIFI_DHCP, true) == false;
    String staticIp = _prefs.getString(NVS_KEY_WIFI_IP, "192.168.1.50");
    String staticMask = _prefs.getString(NVS_KEY_WIFI_SN, "255.255.255.0");
    String staticGw = _prefs.getString(NVS_KEY_WIFI_GW, "192.168.1.1");
    String staticDns = _prefs.getString(NVS_KEY_WIFI_DNS, "1.1.1.1");

    bool ntpEn = _prefs.getBool(NVS_KEY_NTP_ENABLED, true);
    String ntpSrv = _prefs.getString(NVS_KEY_NTP_SERVER, NTP_DEFAULT_SERVER);
    String savedCity = _prefs.getString(NVS_KEY_NTP_TZ_CITY, DEFAULT_TZ_CITY);
    bool timeFormat24h = _prefs.getBool(NVS_KEY_TIME_FORMAT_24H, true);

    time_t nowT = time(nullptr);
    bool isSynced = (nowT > 1577836800);
    String currentTimeStr = "Waiting for sync (RTC uncalibrated)";
    String lastSyncStr = "Never / Just booted";

    if (isSynced) {
        struct tm ti;
        localtime_r(&nowT, &ti);
        char tBuf[40];
        if (timeFormat24h) {
            strftime(tBuf, sizeof(tBuf), "%Y-%m-%d %H:%M", &ti);
        } else {
            strftime(tBuf, sizeof(tBuf), "%Y-%m-%d %I:%M %p", &ti);
        }
        currentTimeStr = String(tBuf);

        time_t syncT = (g_lastNtpSyncTimestamp > 0) ? g_lastNtpSyncTimestamp : nowT;
        struct tm sTi;
        localtime_r(&syncT, &sTi);
        char sBuf[40];
        if (timeFormat24h) {
            strftime(sBuf, sizeof(sBuf), "%Y-%m-%d %H:%M", &sTi);
        } else {
            strftime(sBuf, sizeof(sBuf), "%Y-%m-%d %I:%M %p", &sTi);
        }
        long diffSec = (long)(nowT - syncT);
        if (diffSec < 60) {
            lastSyncStr = String(sBuf) + " (" + String(diffSec) + "s ago)";
        } else {
            lastSyncStr = String(sBuf) + " (" + String(diffSec / 60) + "m ago)";
        }
    }


    // 1. SYSTEM ACTIONS
    res.sendChunk_P(PSTR(
        "<div class=\"table-card\">\n"
        "  <h3 style=\"color:var(--navy);\">&#9881; System Actions</h3>\n"
        "  <div style=\"display:flex;gap:12px;flex-wrap:wrap;\">\n"
        "    <a href=\"/wifi\" class=\"btn btn-outline\">&#128246; Reconfigure WiFi</a>\n"
        "    <a href=\"/update\" class=\"btn btn-outline\">&#128640; Firmware Update (OTA)</a>\n"
        "    <button type=\"button\" onclick=\"restartDevice()\" class=\"btn btn-outline\">&#128260; Restart Device</button>\n"
        "    <button type=\"button\" onclick=\"forgetWifi()\" class=\"btn btn-outline\">&#9888; Forget WiFi</button>\n"
        "    <button type=\"button\" onclick=\"factoryReset()\" class=\"btn btn-outline\" style=\"color:var(--danger);border-color:var(--danger);\">&#9888; Factory Reset</button>\n"
        "  </div>\n"
        "</div>\n"
        "<form id=\"settings_form\" onsubmit=\"saveSettings(event)\">\n"
    ));

    // 2. NETWORK IDENTITY CARD
    {
        String cNet = "";
        cNet.reserve(1200);
        cNet += "  <div class=\"table-card\">\n";
        cNet += "    <h3 style=\"color:var(--navy);\">&#127760; Network &amp; Device Identity</h3>\n";
        cNet += "    <div style=\"margin-bottom:16px;max-width:440px;\">\n";
        cNet += "      <label style=\"font-size:0.82rem;font-weight:600;display:block;margin-bottom:4px;color:var(--muted);\">Device Hostname (mDNS / OTA / DHCP):</label>\n";
        cNet += "      <input type=\"text\" name=\"dev_host\" id=\"dev_host\" value=\"" + devHost + "\" placeholder=\"esp-oobm-xxxxxx\" maxlength=\"32\" class=\"form-control\" style=\"font-family:monospace;\">\n";
        cNet += "      <small style=\"color:var(--muted);display:block;margin-top:4px;\">Local URL: <code>http://" + devHost + ".local/</code></small>\n";
        cNet += "    </div>\n";
        cNet += "    <label style=\"display:flex;align-items:center;gap:8px;font-weight:600;font-size:0.9rem;margin-bottom:12px;\">\n";
        cNet += "      <input type=\"checkbox\" name=\"ip_static\" id=\"ip_static\" value=\"1\" " + String(staticEn ? "checked " : "") + "onchange=\"toggleStaticIp(this.checked)\"> Use Static IP Configuration (instead of DHCP)\n";
        cNet += "    </label>\n";
        cNet += "    <div id=\"static_ip_fields\" style=\"" + String(staticEn ? "" : "display:none;") + "\" class=\"form-grid\">\n";
        cNet += "      <div class=\"form-group\"><label>Static IP Address:</label><input type=\"text\" name=\"ip_addr\" value=\"" + staticIp + "\" placeholder=\"192.168.1.150\" class=\"form-control\"></div>\n";
        cNet += "      <div class=\"form-group\"><label>Subnet Mask:</label><input type=\"text\" name=\"ip_mask\" value=\"" + staticMask + "\" placeholder=\"255.255.255.0\" class=\"form-control\"></div>\n";
        cNet += "      <div class=\"form-group\"><label>Default Gateway:</label><input type=\"text\" name=\"ip_gw\" value=\"" + staticGw + "\" placeholder=\"192.168.1.1\" class=\"form-control\"></div>\n";
        cNet += "      <div class=\"form-group\"><label>Primary DNS Server:</label><input type=\"text\" name=\"ip_dns\" value=\"" + staticDns + "\" placeholder=\"1.1.1.1\" class=\"form-control\"></div>\n";
        cNet += "    </div>\n";
        cNet += "    <small style=\"color:var(--muted);display:block;margin-top:8px;\">When unchecked, device dynamically receives network parameters via DHCP from your router.</small>\n";
        cNet += "  </div>\n";
        res.sendChunk(cNet);
    }

    // 4. TIME SYNCHRONIZATION & TIMEZONE CARD
    {
        String c2 = "";
        c2.reserve(1200);
        c2 += "  <div class=\"table-card\">\n";
        c2 += "    <div style=\"display:flex;justify-content:space-between;align-items:center;margin-bottom:14px;flex-wrap:gap:10px;\">\n";
        c2 += "      <h3 style=\"margin:0;color:var(--navy);\">&#128344; Time Synchronization &amp; Timezone</h3>\n";
        c2 += "      <span id=\"ntp_status_badge\" class=\"badge " + String(isSynced ? "badge-ok\">Synced (OK)" : "badge-warn\">Waiting for sync") + "</span>\n";
        c2 += "    </div>\n";
        c2 += "    <div style=\"background:var(--bg);border:1px solid var(--border);border-radius:8px;padding:12px 16px;margin-bottom:16px;display:flex;justify-content:space-between;align-items:center;flex-wrap:wrap;gap:12px;\">\n";
        c2 += "      <div>\n";
        c2 += "        <div style=\"font-size:0.88rem;color:var(--text);font-weight:600;\">Current Device Time: <span id=\"live_time_str\" style=\"color:var(--navy);font-family:monospace;font-size:0.95rem;font-weight:700;margin-left:4px;\">" + currentTimeStr + "</span></div>\n";
        c2 += "        <div style=\"font-size:0.80rem;color:var(--muted);margin-top:4px;\">Last Sync: <b id=\"live_sync_str\">" + lastSyncStr + "</b> &bull; Sync Interval: <b>Every 1 hour (3600s)</b></div>\n";
        c2 += "      </div>\n";
        c2 += "      <div><button type=\"button\" onclick=\"syncNtpNow()\" class=\"btn btn-outline\" style=\"padding:5px 12px;font-size:0.80rem;\">&#128260; Sync Time Now</button></div>\n";
        c2 += "    </div>\n";
        c2 += "    <label style=\"display:flex;align-items:center;gap:8px;font-weight:600;font-size:0.9rem;margin-bottom:12px;\">\n";
        c2 += "      <input type=\"checkbox\" name=\"ntp_en\" value=\"1\" " + String(ntpEn ? "checked " : "") + "> Enable NTP Network Time Protocol\n";
        c2 += "    </label>\n";
        c2 += "    <div class=\"form-grid\">\n";
        c2 += "      <div class=\"form-group\"><label>NTP Server:</label><input type=\"text\" name=\"ntp_srv\" value=\"" + ntpSrv + "\" placeholder=\"pool.ntp.org\" class=\"form-control\"></div>\n";
        c2 += "      <div class=\"form-group\"><label>City &amp; Timezone:</label>\n";
        c2 += "        <select name=\"tz_idx\" class=\"form-control\">\n";
        res.sendChunk(c2);
    }

    {
        String tzOptions = "";
        tzOptions.reserve(1500);
        for (size_t i = 0; i < TIMEZONE_COUNT; ++i) {
            bool sel = (savedCity == TIMEZONE_LIST[i].city);
            tzOptions += "<option value=\"" + String(i) + "\"" + (sel ? " selected" : "") + ">" + TIMEZONE_LIST[i].city + "</option>\n";
        }
        res.sendChunk(tzOptions);
    }

    {
        String c2End = "";
        c2End.reserve(600);
        c2End += "        </select>\n";
        c2End += "      </div>\n";
        c2End += "      <div class=\"form-group\"><label>Time Format:</label>\n";
        c2End += "        <select name=\"time_fmt\" class=\"form-control\">\n";
        c2End += "          <option value=\"24\" " + String(timeFormat24h ? "selected" : "") + ">24 Hours (e.g. 20:15:00)</option>\n";
        c2End += "          <option value=\"12\" " + String(!timeFormat24h ? "selected" : "") + ">12 Hours (e.g. 8:15:00 PM)</option>\n";
        c2End += "        </select>\n";
        c2End += "      </div>\n";
        c2End += "    </div>\n";
        c2End += "    <small style=\"color:var(--muted);display:block;margin-top:8px;\">Device time adjusts automatically for Daylight Saving Time (DST). Resynchronizes clock every 1 hour.</small>\n";
        c2End += "  </div>\n";
        res.sendChunk(c2End);
    }

    // 5. SERIAL PORT & BRIDGE CARD
    {
        String c3 = "";
        c3.reserve(1200);
        uint32_t curBaud = serialBridge.getBaudRate();
        uint8_t curDbits = serialBridge.getDataBits();
        uint8_t curPar = serialBridge.getParity();
        uint8_t curSbits = serialBridge.getStopBits();

        c3 += "  <div class=\"table-card\">\n";
        c3 += "    <h3 style=\"color:var(--navy);\">&#9889; Serial Console Port (UART0)</h3>\n";
        c3 += "    <div class=\"form-grid\">\n";
        c3 += "      <div class=\"form-group\"><label>Baud Rate</label>\n";
        c3 += "        <select name=\"ser_baud\" class=\"form-control\">\n";
        const uint32_t bauds[] = {9600, 19200, 38400, 57600, 115200, 230400, 460800, 921600};
        for (size_t b = 0; b < sizeof(bauds)/sizeof(bauds[0]); b++) {
            c3 += "<option value=\"" + String(bauds[b]) + "\"" + String(curBaud == bauds[b] ? " selected" : "") + ">" + String(bauds[b]) + " bps" + (bauds[b] == 115200 ? " (Default)" : "") + "</option>\n";
        }
        c3 += "        </select>\n";
        c3 += "      </div>\n";
        c3 += "      <div class=\"form-group\"><label>Data Bits</label>\n";
        c3 += "        <select name=\"ser_dbits\" class=\"form-control\">\n";
        c3 += "          <option value=\"8\" " + String(curDbits == 8 ? "selected" : "") + ">8 Bits (Standard)</option>\n";
        c3 += "          <option value=\"7\" " + String(curDbits == 7 ? "selected" : "") + ">7 Bits</option>\n";
        c3 += "        </select>\n";
        c3 += "      </div>\n";
        c3 += "      <div class=\"form-group\"><label>Parity</label>\n";
        c3 += "        <select name=\"ser_parity\" class=\"form-control\">\n";
        c3 += "          <option value=\"0\" " + String(curPar == 0 ? "selected" : "") + ">None (8N1)</option>\n";
        c3 += "          <option value=\"1\" " + String(curPar == 1 ? "selected" : "") + ">Odd</option>\n";
        c3 += "          <option value=\"2\" " + String(curPar == 2 ? "selected" : "") + ">Even</option>\n";
        c3 += "        </select>\n";
        c3 += "      </div>\n";
        c3 += "      <div class=\"form-group\"><label>Stop Bits</label>\n";
        c3 += "        <select name=\"ser_sbits\" class=\"form-control\">\n";
        c3 += "          <option value=\"1\" " + String(curSbits == 1 ? "selected" : "") + ">1 Stop Bit</option>\n";
        c3 += "          <option value=\"2\" " + String(curSbits == 2 ? "selected" : "") + ">2 Stop Bits</option>\n";
        c3 += "        </select>\n";
        c3 += "      </div>\n";
        c3 += "    </div>\n";
        c3 += "    <div style=\"margin-top:16px;display:flex;gap:20px;flex-wrap:wrap;\">\n";
        c3 += "      <label class=\"switch-label\"><input type=\"checkbox\" name=\"ser_echo\" value=\"1\" " + String(serialBridge.getForceEcho() ? "checked " : "") + "> Force Local Echo</label>\n";
        c3 += "      <label class=\"switch-label\"><input type=\"checkbox\" name=\"ser_banner\" value=\"1\" " + String(serialBridge.getGreetingBanner() ? "checked " : "") + "> Greeting Banner on Connect</label>\n";
        c3 += "    </div>\n";
        c3 += "  </div>\n";
        res.sendChunk(c3);
    }

    // 6. TELNET SERVICE & 7. SECURITY CARD
    {
        String c4 = "";
        c4.reserve(1500);
        c4 += "  <div class=\"table-card\">\n";
        c4 += "    <h3 style=\"color:var(--navy);\">&#128268; Telnet Console Service (RFC 854)</h3>\n";
        c4 += "    <label style=\"display:flex;align-items:center;gap:8px;font-weight:600;font-size:0.9rem;margin-bottom:12px;\">\n";
        c4 += "      <input type=\"checkbox\" name=\"tel_en\" value=\"1\" " + String(telnetServer.isEnabled() ? "checked " : "") + "> Enable Telnet Server Daemon\n";
        c4 += "    </label>\n";
        c4 += "    <div class=\"form-grid\">\n";
        c4 += "      <div class=\"form-group\"><label>Telnet Port:</label><input type=\"number\" name=\"tel_port\" value=\"" + String(telnetServer.getPort()) + "\" class=\"form-control\"></div>\n";
        c4 += "      <div class=\"form-group\">\n";
        c4 += "        <label class=\"switch-label\" style=\"margin-top:24px;\"><input type=\"checkbox\" name=\"tel_auth\" value=\"1\" " + String(telnetServer.isAuthRequired() ? "checked " : "") + "> Require Telnet Password</label>\n";
        c4 += "      </div>\n";
        c4 += "      <div class=\"form-group\"><label>Telnet Password:</label>\n";
        c4 += "        <div class=\"pwd-wrap\"><input type=\"password\" name=\"tel_pass\" value=\"" + _prefs.getString(NVS_KEY_TELNET_PASS, DEFAULT_TELNET_PASS) + "\" placeholder=\"Telnet Password\" class=\"form-control\">" + String(FPSTR(PWD_EYE_TOGGLE_HTML)) + "</div>\n";
        c4 += "      </div>\n";
        c4 += "    </div>\n";
        c4 += "  </div>\n";

        c4 += "  <div class=\"table-card\">\n";
        c4 += "    <h3 style=\"color:var(--navy);\">&#128274; Web &amp; API Security</h3>\n";
        c4 += "    <label style=\"display:flex;align-items:center;gap:8px;font-weight:600;font-size:0.9rem;margin-bottom:12px;\">\n";
        c4 += "      <input type=\"checkbox\" name=\"auth_en\" value=\"1\" " + String(_authRequired ? "checked " : "") + "> Enable HTTP Basic Authentication for Web UI\n";
        c4 += "    </label>\n";
        c4 += "    <div class=\"form-grid\">\n";
        c4 += "      <div class=\"form-group\"><label>Admin Username:</label><input type=\"text\" name=\"auth_usr\" value=\"" + String(_authUser) + "\" class=\"form-control\"></div>\n";
        c4 += "      <div class=\"form-group\"><label>Admin Password:</label>\n";
        c4 += "        <div class=\"pwd-wrap\"><input type=\"password\" name=\"auth_pwd\" value=\"" + String(_authPass) + "\" placeholder=\"Admin Password\" class=\"form-control\">" + String(FPSTR(PWD_EYE_TOGGLE_HTML)) + "</div>\n";
        c4 += "      </div>\n";
        c4 += "    </div>\n";
        c4 += "  </div>\n";

        c4 += "  <div style=\"margin-bottom:30px;\">\n";
        c4 += "    <button type=\"submit\" class=\"btn btn-primary\" style=\"padding:10px 24px;\">Save All Settings</button>\n";
        c4 += "  </div>\n";
        c4 += "</form>\n";
        res.sendChunk(c4);
    }

    // Settings JavaScript
    res.sendChunk_P(PSTR(
        "<script>\n"
        "function toggleStaticIp(chk){ document.getElementById('static_ip_fields').style.display = chk ? 'grid' : 'none'; }\n"
        "function syncNtpNow(){\n"
        "  fetch('/api/ntp/sync', { method: 'POST' }).then(r=>r.json()).then(res=>{\n"
        "    showToast('NTP sync triggered!');\n"
        "    setTimeout(()=>location.reload(), 1500);\n"
        "  }).catch(()=>showToast('Sync request failed', true));\n"
        "}\n"
        "function restartDevice(){\n"
        "  if(confirm('Restart ESP32 device?')){\n"
        "    fetch('/api/restart', { method: 'POST' }).then(()=>{\n"
        "      showToast('Device is restarting... Please wait 10s.');\n"
        "      setTimeout(()=>location.reload(), 10000);\n"
        "    });\n"
        "  }\n"
        "}\n"
        "function forgetWifi(){\n"
        "  if(confirm('Forget WiFi settings? Device will disconnect and start in AP mode.')){\n"
        "    location.href = '/reset_wifi';\n"
        "  }\n"
        "}\n"
        "function factoryReset(){\n"
        "  if(confirm('WARNING: Factory Reset will erase ALL configuration, passwords, and WiFi settings! Continue?')){\n"
        "    fetch('/api/factory_reset', { method: 'POST' }).then(()=>{\n"
        "      showToast('Factory reset complete. Restarting in AP mode...');\n"
        "      setTimeout(()=>location.href='/', 8000);\n"
        "    }).catch(()=>showToast('Reset failed', true));\n"
        "  }\n"
        "}\n"
        "function saveSettings(e){\n"
        "  e.preventDefault();\n"
        "  var fd = new FormData(document.getElementById('settings_form'));\n"
        "  fetch('/api/settings/save', { method: 'POST', body: fd }).then(r=>r.json()).then(res=>{\n"
        "    if(res.success){ showToast('Settings saved successfully!'); }\n"
        "    else { showToast('Error saving settings', true); }\n"
        "  }).catch(()=>showToast('Save failed', true));\n"
        "}\n"
        "</script>\n"
    ));

    streamFooter(res);
}

// =============================================================================
// Dedicated Reconfigure WiFi Page (/wifi)
// =============================================================================
void WebPortal::renderWifiPage(ResponseWriter &res) {
    streamHeader(res, "settings", "Reconfigure WiFi");

    String staSsid = _prefs.getString(NVS_KEY_WIFI_SSID, "");
    String staPass = _prefs.getString(NVS_KEY_WIFI_PASS, "");
    String apSsid = _prefs.getString(NVS_KEY_AP_SSID, "");
    String apPass = _prefs.getString(NVS_KEY_AP_PASS, AP_DEFAULT_PASSWORD);
    if (apSsid.length() == 0) {
        String mac = WiFi.macAddress();
        mac.replace(":", "");
        apSsid = String(AP_SSID_PREFIX) + mac.substring(mac.length() - 6);
    }
    uint8_t apChan = _prefs.getUChar(NVS_KEY_AP_CHAN, AP_DEFAULT_CHANNEL);
    bool apHidden = _prefs.getBool(NVS_KEY_AP_HIDDEN, false);
    bool apCaptive = _prefs.getBool(NVS_KEY_AP_CAPTIVE, true);
    bool mndpEn = _prefs.getBool(NVS_KEY_MNDP_EN, true);

    res.sendChunk_P(PSTR(
        "<div style=\"margin-bottom:16px;display:flex;justify-content:space-between;align-items:center;flex-wrap:wrap;gap:10px;\">\n"
        "  <h2 style=\"margin:0;font-size:1.25rem;\">&#128246; Reconfigure Wi-Fi Network</h2>\n"
        "  <a href=\"/settings\" class=\"btn btn-outline btn-sm\">&larr; Back to Settings</a>\n"
        "</div>\n"
        "<form id=\"wifi_form\" onsubmit=\"saveWifi(event)\">\n"
        "  <div class=\"table-card\">\n"
        "    <h3 style=\"color:var(--navy);\">1. Station Mode (Connect to Existing Wi-Fi)</h3>\n"
        "    <div style=\"margin-bottom:16px;\">\n"
        "      <label style=\"font-size:0.84rem;font-weight:600;color:var(--muted);display:block;margin-bottom:6px;\">Available Networks (Scanned):</label>\n"
        "      <div style=\"display:flex;gap:8px;flex-wrap:wrap;align-items:stretch;\">\n"
        "        <select id=\"scanned_ssid\" class=\"form-control\" onchange=\"selectSsid(this.value)\" style=\"flex:1;min-width:200px;\">\n"
        "          <option value=\"\">-- Click 'Scan Networks' to search --</option>\n"
        "        </select>\n"
        "        <button type=\"button\" id=\"btn_scan\" class=\"btn btn-outline\" onclick=\"triggerScan()\" style=\"padding:8px 16px;flex-shrink:0;\">&#128269; <span>Scan Networks</span></button>\n"
        "      </div>\n"
        "    </div>\n"
        "    <div class=\"form-grid\">\n"
    ));

    String wSta = "";
    wSta.reserve(800);
    wSta += "      <div class=\"form-group\"><label>Network SSID:</label><input type=\"text\" name=\"sta_ssid\" id=\"sta_ssid\" value=\"" + staSsid + "\" placeholder=\"Enter Wi-Fi SSID\" class=\"form-control\"></div>\n";
    wSta += "      <div class=\"form-group\"><label>Wi-Fi Password:</label>\n";
    wSta += "        <div class=\"pwd-wrap\"><input type=\"password\" name=\"sta_pass\" id=\"sta_pass\" value=\"" + staPass + "\" placeholder=\"WPA/WPA2 Password\" class=\"form-control\">" + String(FPSTR(PWD_EYE_TOGGLE_HTML)) + "</div>\n";
    wSta += "      </div>\n";
    wSta += "    </div>\n";
    wSta += "  </div>\n";
    res.sendChunk(wSta);

    String wAp = "";
    wAp.reserve(1200);
    wAp += "  <div class=\"table-card\">\n";
    wAp += "    <h3 style=\"color:var(--navy);\">2. Access Point (AP) Settings</h3>\n";
    wAp += "    <div class=\"form-grid\">\n";
    wAp += "      <div class=\"form-group\"><label>AP SSID:</label><input type=\"text\" name=\"ap_ssid\" value=\"" + apSsid + "\" placeholder=\"ESP-OOBM-XXXXXX\" class=\"form-control\"></div>\n";
    wAp += "      <div class=\"form-group\"><label>AP Password (Empty = Open):</label>\n";
    wAp += "        <div class=\"pwd-wrap\"><input type=\"password\" name=\"ap_pass\" value=\"" + apPass + "\" placeholder=\"Optional AP Password\" class=\"form-control\">" + String(FPSTR(PWD_EYE_TOGGLE_HTML)) + "</div>\n";
    wAp += "      </div>\n";
    wAp += "      <div class=\"form-group\"><label>AP Channel:</label>\n";
    wAp += "        <select name=\"ap_chan\" class=\"form-control\">\n";
    wAp += "          <option value=\"1\" " + String(apChan == 1 ? "selected" : "") + ">Channel 1</option>\n";
    wAp += "          <option value=\"6\" " + String(apChan == 6 ? "selected" : "") + ">Channel 6</option>\n";
    wAp += "          <option value=\"11\" " + String(apChan == 11 ? "selected" : "") + ">Channel 11</option>\n";
    wAp += "        </select>\n";
    wAp += "      </div>\n";
    wAp += "    </div>\n";
    wAp += "    <div style=\"margin-top:14px;display:flex;gap:20px;flex-wrap:wrap;\">\n";
    wAp += "      <label class=\"switch-label\"><input type=\"checkbox\" name=\"ap_hidden\" value=\"1\" " + String(apHidden ? "checked " : "") + "> Hide AP SSID</label>\n";
    wAp += "      <label class=\"switch-label\"><input type=\"checkbox\" name=\"ap_captive\" value=\"1\" " + String(apCaptive ? "checked " : "") + "> Captive Portal Redirection</label>\n";
    wAp += "      <label class=\"switch-label\"><input type=\"checkbox\" name=\"mndp_en\" value=\"1\" " + String(mndpEn ? "checked " : "") + "> MNDP Neighbor Discovery (UDP 5678)</label>\n";
    wAp += "    </div>\n";
    wAp += "  </div>\n";
    wAp += "  <div style=\"margin-bottom:30px;\">\n";
    wAp += "    <button type=\"submit\" class=\"btn btn-primary\" style=\"padding:10px 24px;\">Connect &amp; Save Wi-Fi</button>\n";
    wAp += "  </div>\n";
    wAp += "</form>\n";
    res.sendChunk(wAp);

    res.sendChunk_P(PSTR(
        "<script>\n"
        "function selectSsid(v){\n"
        "  if(v){\n"
        "    var sta = document.getElementById('sta_ssid');\n"
        "    if(sta) sta.value = v;\n"
        "    var pwd = document.getElementById('sta_pass');\n"
        "    if(pwd) pwd.focus();\n"
        "  }\n"
        "}\n"
        "var scanInterval = null;\n"
        "function triggerScan(){\n"
        "  var btn = document.getElementById('btn_scan');\n"
        "  var s = document.getElementById('scanned_ssid');\n"
        "  if(btn){ btn.disabled = true; btn.innerHTML = '&#9203; <span>Scanning...</span>'; }\n"
        "  if(s){ s.innerHTML = '<option value=\"\">Scanning networks, please wait...</option>'; }\n"
        "  var scanDeadline = Date.now() + 15000;\n"
        "  function finishScan(ready){\n"
        "    if(scanInterval){ clearTimeout(scanInterval); scanInterval = null; }\n"
        "    if(btn){ btn.disabled = false; btn.innerHTML = ready ? '&#128260; <span>Rescan</span>' : '&#128269; <span>Scan Networks</span>'; }\n"
        "  }\n"
        "  function pollScan(){\n"
        "    fetch('/api/scan').then(r=>r.json()).then(res=>{\n"
        "      if(res.status === 'ready' && res.networks){\n"
        "        finishScan(true);\n"
        "        s.innerHTML = '<option value=\"\">-- Select a network (' + res.networks.length + ' found) --</option>';\n"
        "        res.networks.forEach(function(net){\n"
        "          var opt = document.createElement('option');\n"
        "          opt.value = net.ssid;\n"
        "          opt.innerText = net.ssid + ' (' + net.rssi + ' dBm' + (net.secure ? ' \\u{1F512}' : '') + ')';\n"
        "          s.appendChild(opt);\n"
        "        });\n"
        "      } else if(Date.now() < scanDeadline){\n"
        "        scanInterval = setTimeout(pollScan, 2000);\n"
        "      } else { finishScan(false); }\n"
        "    }).catch(function(){\n"
        "      if(Date.now() < scanDeadline) scanInterval = setTimeout(pollScan, 2000);\n"
        "      else finishScan(false);\n"
        "    });\n"
        "  }\n"
        "  pollScan();\n"
        "}\n"
        "function saveWifi(e){\n"
        "  e.preventDefault();\n"
        "  var fd = new FormData(document.getElementById('wifi_form'));\n"
        "  var btn = document.querySelector('#wifi_form button[type=\"submit\"]');\n"
        "  if(btn){ btn.disabled = true; btn.innerText = 'Connecting & Rebooting...'; }\n"
        "  fetch('/api/wifi/save', { method: 'POST', body: fd }).then(r=>r.json()).then(res=>{\n"
        "    if(res.success){\n"
        "      showToast('Wi-Fi saved! Rebooting to connect to network...');\n"
        "      setTimeout(function(){ location.href = '/'; }, 6000);\n"
        "    } else {\n"
        "      if(btn){ btn.disabled = false; btn.innerText = 'Connect & Save Wi-Fi'; }\n"
        "      showToast('Error saving Wi-Fi', true);\n"
        "    }\n"
        "  }).catch(()=>{\n"
        "    showToast('Rebooting device...', false);\n"
        "    setTimeout(function(){ location.href = '/'; }, 6000);\n"
        "  });\n"
        "}\n"
        "</script>\n"
    ));

    streamFooter(res);
}

// =============================================================================
// Dedicated Firmware Update (OTA) Page (/update)
// =============================================================================
void WebPortal::renderUpdatePage(ResponseWriter &res) {
    streamHeader(res, "settings", "Firmware Update");

    res.sendChunk_P(PSTR(
        "<div style=\"display:flex;justify-content:space-between;align-items:center;margin-bottom:16px;\">"
        "  <h2 style=\"margin:0;font-size:1.25rem;\">&#128640; Over-The-Air Firmware Update</h2>"
        "  <a href=\"/settings\" class=\"btn btn-outline btn-sm\">&larr; Back to Settings</a>"
        "</div>"
        "<div class=\"table-card\">"
        "  <h3 style=\"color:var(--navy);\">Upload New Firmware Binary (.bin)</h3>"
        "  <p style=\"color:var(--muted);font-size:0.88rem;line-height:1.6;\">"
        "    Select the compiled <code>firmware.bin</code> file from PlatformIO (located in <code>.pio/build/esp32_pico_d4/firmware.bin</code>).<br>"
        "    The device uses a fail-safe dual OTA partition scheme and will automatically reboot into the updated firmware upon verification."
        "  </p>"
        "  <form id=\"ota_form\" method=\"POST\" action=\"/update\" enctype=\"multipart/form-data\" onsubmit=\"uploadFirmware(event)\" style=\"margin-top:20px;\">"
        "    <div style=\"display:flex;gap:12px;align-items:center;flex-wrap:wrap;\">"
        "      <input type=\"file\" name=\"update\" id=\"ota_file\" class=\"form-control\" accept=\".bin\" required style=\"max-width:360px;\">"
        "      <button type=\"submit\" id=\"ota_btn\" class=\"btn btn-green\">Upload &amp; Flash Firmware</button>"
        "    </div>"
        "    <div id=\"progress_wrap\" class=\"progress-bar-wrap\" style=\"display:none;max-width:500px;\">"
        "      <div id=\"progress_fill\" class=\"progress-bar-fill\"></div>"
        "    </div>"
        "    <div id=\"ota_status\" style=\"font-size:0.85rem;margin-top:10px;color:var(--muted);font-weight:600;\"></div>"
        "  </form>"
        "</div>"
        "<script>"
        "function uploadFirmware(e){"
        "  e.preventDefault();"
        "  var file = document.getElementById('ota_file').files[0];"
        "  if(!file) return;"
        "  var fd = new FormData();"
        "  fd.append('update', file);"
        "  var xhr = new XMLHttpRequest();"
        "  var pwrap = document.getElementById('progress_wrap');"
        "  var pfill = document.getElementById('progress_fill');"
        "  var stat = document.getElementById('ota_status');"
        "  var btn = document.getElementById('ota_btn');"
        "  pwrap.style.display = 'block';"
        "  btn.disabled = true;"
        "  stat.innerText = 'Starting upload...';"
        "  xhr.upload.onprogress = function(pe){"
        "    if(pe.lengthComputable){"
        "      var pct = Math.round((pe.loaded / pe.total) * 100);"
        "      pfill.style.width = pct + '%';"
        "      stat.innerText = 'Flashing: ' + pct + '% (' + Math.round(pe.loaded/1024) + ' KB / ' + Math.round(pe.total/1024) + ' KB)';"
        "    }"
        "  };"
        "  xhr.onload = function(){"
        "    if(xhr.status === 200){"
        "      stat.innerText = 'Flash successful! Rebooting into new firmware in 10s...';"
        "      pfill.style.background = 'var(--green)';"
        "      setTimeout(()=>location.href='/', 10000);"
        "    } else {"
        "      stat.innerText = 'OTA update failed: ' + xhr.responseText;"
        "      pfill.style.background = 'var(--danger)';"
        "      btn.disabled = false;"
        "    }"
        "  };"
        "  xhr.open('POST', '/update');"
        "  xhr.send(fd);"
        "}"
        "</script>"
    ));

    streamFooter(res);
}

void WebPortal::renderMetrics(ResponseWriter &res) {
    SystemStatsData stats;
    SystemStats::update(stats);
    PrometheusExporter::generateMetrics(res, stats);
}

// =============================================================================
// Login & Session Authentication Handlers
// =============================================================================
void WebPortal::renderLoginPage(ResponseWriter &res, const char *errMsg) {
    res.setContentType("text/html; charset=utf-8");

    res.sendChunk_P(PSTR("<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\">"
                         "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
                         "<title>Sign In - ESP-OOBM</title>"
                         "<link rel=\"icon\" type=\"image/svg+xml\" href=\"/favicon.svg\">"
                         "<link rel=\"icon\" type=\"image/x-icon\" href=\"/favicon.ico\">"
                         "<style>"));
    res.sendChunk_P(COMMON_CSS);
    res.sendChunk_P(PSTR(
        ".login-wrap{min-height:100vh;display:flex;align-items:center;justify-content:center;padding:20px;}\n"
        ".login-card{background:var(--card);border:1px solid var(--border);border-radius:12px;box-shadow:0 8px 30px rgba(0,0,0,0.25);width:100%;max-width:380px;overflow:hidden;position:relative;}\n"
        ".login-body{padding:28px 24px;}\n"
    ));
    res.sendChunk_P(PSTR("</style><script>"));
    res.sendChunk_P(COMMON_JS);
    res.sendChunk_P(PSTR(
        "function doLogin(e){\n"
        "  if(e) e.preventDefault();\n"
        "  var u = document.getElementById('usr').value.trim();\n"
        "  var p = document.getElementById('pwd').value;\n"
        "  var err = document.getElementById('login_err');\n"
        "  var btn = document.getElementById('btn_submit');\n"
        "  if(!u || !p){\n"
        "    err.style.display = 'block';\n"
        "    err.innerText = 'Please enter both username and password.';\n"
        "    return false;\n"
        "  }\n"
        "  btn.disabled = true;\n"
        "  btn.innerText = 'Signing In...';\n"
        "  var params = new URLSearchParams();\n"
        "  params.append('usr', u);\n"
        "  params.append('pwd', p);\n"
        "  fetch('/api/login', {\n"
        "    method: 'POST',\n"
        "    headers: { 'Content-Type': 'application/x-www-form-urlencoded' },\n"
        "    body: params.toString()\n"
        "  })\n"
        "    .then(function(r){\n"
        "      return r.json().catch(function(){ return { success: false, error: 'Server returned HTTP ' + r.status }; });\n"
        "    })\n"
        "    .then(function(d){\n"
        "      btn.disabled = false;\n"
        "      btn.innerText = 'Sign In \\u2192';\n"
        "      if(d.success){\n"
        "        var redir = new URLSearchParams(window.location.search).get('redir') || '/';\n"
        "        window.location.href = redir;\n"
        "      } else {\n"
        "        err.style.display = 'block';\n"
        "        err.innerText = d.error || 'Invalid credentials.';\n"
        "      }\n"
        "    })\n"
        "    .catch(function(errObj){\n"
        "      btn.disabled = false;\n"
        "      btn.innerText = 'Sign In \\u2192';\n"
        "      err.style.display = 'block';\n"
        "      err.innerText = 'Connection error: ' + (errObj.message || 'Please retry.');\n"
        "    });\n"
        "  return false;\n"
        "}\n"
    ));
    res.sendChunk_P(PSTR("</script></head><body>"
                         "<div class=\"login-wrap\"><div class=\"login-card\">"
                         "<div class=\"top-accent\"></div><div class=\"login-body\">"
                         "<div style=\"text-align:center;margin-bottom:20px;\">"));
    res.sendChunk_P(OOBM_LOGO_SVG);
    res.sendChunk_P(PSTR("<div style=\"font-size:0.82rem;color:var(--muted);margin-top:6px;font-weight:500;\">"
                         "Wireless Out-of-Band Management Dongle</div></div>"
                         "<div id=\"login_err\" style=\"display:none;margin-bottom:16px;padding:8px 12px;border-radius:6px;background:#fee2e2;color:#b91c1c;font-size:0.84rem;font-weight:600;border:1px solid #fca5a5;\"></div>"
                         "<form onsubmit=\"return doLogin(event);\">"
                         "<div class=\"form-group\" style=\"margin-bottom:14px;\">"
                         "  <label for=\"usr\">Username</label>"
                         "  <input type=\"text\" id=\"usr\" class=\"form-control\" placeholder=\"admin\" value=\"admin\" autofocus autocomplete=\"username\" required>"
                         "</div>"
                         "<div class=\"form-group\" style=\"margin-bottom:18px;\">"
                         "  <label for=\"pwd\">Password</label>"
                         "  <div class=\"pwd-wrap\">"
                         "    <input type=\"password\" id=\"pwd\" class=\"form-control\" placeholder=\"••••••••\" autocomplete=\"current-password\" required>"
                         "    <button type=\"button\" class=\"pwd-toggle\" onclick=\"togglePassword(this)\" title=\"Toggle password visibility\" tabindex=\"-1\">"
                         "      <svg class=\"eye-open\" width=\"16\" height=\"16\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\"><path d=\"M1 12s4-8 11-8 11 8 11 8-4 8-11 8-11-8-11-8z\"/><circle cx=\"12\" cy=\"12\" r=\"3\"/></svg>"
                         "      <svg class=\"eye-closed\" width=\"16\" height=\"16\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\" style=\"display:none;\"><path d=\"M17.94 17.94A10.07 10.07 0 0 1 12 20c-7 0-11-8-11-8a18.45 18.45 0 0 1 5.06-5.94M9.9 4.24A9.12 9.12 0 0 1 12 4c7 0 11 8 11 8a18.5 18.5 0 0 1-2.16 3.19m-6.72-1.07a3 3 0 1 1-4.24-4.24\"/><line x1=\"1\" y1=\"1\" x2=\"23\" y2=\"23\"/></svg>"
                         "    </button>"
                         "  </div>"
                         "</div>"
                         "<button type=\"submit\" id=\"btn_submit\" class=\"btn btn-primary\" style=\"width:100%;justify-content:center;padding:10px 16px;\">"
                         "  Sign In &rarr;"
                         "</button>"
                         "</form>"
                         "<div style=\"margin-top:22px;display:flex;justify-content:space-between;align-items:center;border-top:1px solid var(--border);padding-top:14px;\">"
                         "  <span style=\"font-size:0.75rem;color:var(--muted);\">v" FIRMWARE_VERSION "</span>"
                         "  <div class=\"theme-switch\">"
                         "    <button id=\"themeBtnLight\" class=\"theme-btn\" onclick=\"setTheme('light')\" title=\"Light Theme\">"
                         "      <svg width=\"12\" height=\"12\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\"><circle cx=\"12\" cy=\"12\" r=\"4\"/><path d=\"M12 2v2M12 20v2M4.93 4.93l1.41 1.41M17.66 17.66l1.41 1.41M2 12h2M20 12h2M6.34 17.66l-1.41 1.41M19.07 4.93l-1.41 1.41\"/></svg>"
                         "    </button>"
                         "    <button id=\"themeBtnDark\" class=\"theme-btn\" onclick=\"setTheme('dark')\" title=\"Dark Theme\">"
                         "      <svg width=\"12\" height=\"12\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\"><path d=\"M21 12.79A9 9 0 1 1 11.21 3 7 7 0 0 0 21 12.79z\"/></svg>"
                         "    </button>"
                         "    <button id=\"themeBtnSystem\" class=\"theme-btn\" onclick=\"setTheme('system')\" title=\"System Theme\">"
                         "      <svg width=\"12\" height=\"12\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\"><rect x=\"2\" y=\"3\" width=\"20\" height=\"14\" rx=\"2\"/><line x1=\"8\" y1=\"21\" x2=\"16\" y2=\"21\"/><line x1=\"12\" y1=\"17\" x2=\"12\" y2=\"21\"/></svg>"
                         "    </button>"
                         "  </div>"
                         "</div>"
                         "</div></div></div></body></html>"
    ));
    res.end();
}

// =============================================================================
// Unified AJAX Endpoints
// =============================================================================
void WebPortal::handleApiLogin(ResponseWriter &res, const String &u, const String &p) {
    if (u == _authUser && p == _authPass) {
        String setCookie = "oobm_session=" + _sessionToken + "; Path=/; Max-Age=86400; SameSite=Lax";
        res.setHeader("Set-Cookie", setCookie.c_str());
        res.setContentType("application/json");
        logger.logInfo("Web user '%s' logged in successfully.", _authUser);
        res.sendChunk("{\"success\":true,\"token\":\"" + _sessionToken + "\"}");
    } else {
        logger.logWarn("Failed web login attempt with username '%s'.", u.c_str());
        res.setStatus(401, "401 Unauthorized");
        res.setContentType("application/json");
        res.sendChunk("{\"success\":false,\"error\":\"Invalid username or password\"}");
    }
    res.end();
}

void WebPortal::handleApiStatus(ResponseWriter &res) {
    SystemStatsData stats;
    SystemStats::update(stats);

    char uptimeBuf[32], heapBuf[16], minHeapBuf[16], rxBuf[16], txBuf[16], timeBuf[32], lastNtpBuf[40];
    SystemStats::formatUptime(stats.uptimeSeconds, uptimeBuf, sizeof(uptimeBuf));
    SystemStats::formatBytes(stats.freeHeapBytes, heapBuf, sizeof(heapBuf));
    SystemStats::formatBytes(stats.minFreeHeapBytes, minHeapBuf, sizeof(minHeapBuf));
    SystemStats::formatBytes(stats.serialRxBytes, rxBuf, sizeof(rxBuf));
    SystemStats::formatBytes(stats.serialTxBytes, txBuf, sizeof(txBuf));

    bool is24h = _prefs.getBool(NVS_KEY_TIME_FORMAT_24H, true);
    if (stats.currentTime > 1577836800) {
        struct tm ti;
        localtime_r(&stats.currentTime, &ti);
        if (is24h) {
            strftime(timeBuf, sizeof(timeBuf), "%Y-%m-%d %H:%M", &ti);
        } else {
            strftime(timeBuf, sizeof(timeBuf), "%Y-%m-%d %I:%M %p", &ti);
        }
    } else {
        snprintf(timeBuf, sizeof(timeBuf), "--:--");
    }

    if (g_lastNtpSyncTimestamp > 0) {
        struct tm syncInfo;
        localtime_r(&g_lastNtpSyncTimestamp, &syncInfo);
        if (is24h) {
            strftime(lastNtpBuf, sizeof(lastNtpBuf), "%Y-%m-%d %H:%M", &syncInfo);
        } else {
            strftime(lastNtpBuf, sizeof(lastNtpBuf), "%Y-%m-%d %I:%M %p", &syncInfo);
        }
    } else {
        snprintf(lastNtpBuf, sizeof(lastNtpBuf), "Never / Just booted");
    }

    char framing[8];
    snprintf(framing, sizeof(framing), "%u%c%u",
             serialBridge.getDataBits(),
             (serialBridge.getParity() == 1 ? 'O' : (serialBridge.getParity() == 2 ? 'E' : 'N')),
             serialBridge.getStopBits());

    char json[1500];
    snprintf(json, sizeof(json),
             "{\"uptime_sec\":%u,\"uptime_str\":\"%s\",\"cpu_freq\":%u,\"cpu_load\":%.1f,\"temp_c\":%.1f,\"free_heap\":%u,\"free_heap_str\":\"%s\","
             "\"min_free_heap\":%u,\"min_heap_str\":\"%s\",\"heap_frag\":%u,\"baud\":%u,\"framing\":\"%s\","
             "\"rx_bytes\":%u,\"rx_bytes_str\":\"%s\",\"tx_bytes\":%u,\"tx_bytes_str\":\"%s\",\"rx_overflow\":%u,"
             "\"active_ws\":%u,\"active_telnet\":%u,"
             "\"net_mode\":\"%s\",\"ip\":\"%s\",\"ssid\":\"%s\",\"mac\":\"%s\","
             "\"rssi\":%d,\"ap_clients\":%u,\"ntp_synced\":%s,\"time_str\":\"%s\",\"last_ntp_str\":\"%s\","
             "\"tz_city\":\"%s\",\"ntp_server\":\"%s\"}",
             stats.uptimeSeconds, uptimeBuf, stats.cpuFreqMhz, stats.cpuLoadPercent, stats.temperatureCelsius,
             stats.freeHeapBytes, heapBuf,
             stats.minFreeHeapBytes, minHeapBuf, stats.heapFragPercent, stats.baudRate, framing,
             stats.serialRxBytes, rxBuf, stats.serialTxBytes, txBuf, stats.serialRxOverflow,
             stats.activeWsClients, stats.activeTelnetClients,
             stats.wifiStaConnected ? "Station" : (stats.wifiApActive ? "Access Point" : "Disconnected"),
             stats.ipAddress, stats.ssid, stats.macAddress, stats.wifiRssi, stats.wifiApClients,
             stats.ntpSynced ? "true" : "false", timeBuf, lastNtpBuf,
             _prefs.getString(NVS_KEY_NTP_TZ_CITY, DEFAULT_TZ_CITY).c_str(),
             _prefs.getString(NVS_KEY_NTP_SERVER, NTP_DEFAULT_SERVER).c_str());

    res.setContentType("application/json");
    res.sendChunk(json);
    res.end();
}

void WebPortal::handleApiScan(ResponseWriter &res) {
    int16_t scanStatus = WiFi.scanComplete();

    if (scanStatus == WIFI_SCAN_RUNNING) {
        res.setContentType("application/json");
        res.sendChunk("{\"status\":\"scanning\",\"networks\":[]}");
        res.end();
        return;
    }

    if (scanStatus >= 0) {
        res.setContentType("application/json");
        res.sendChunk("{\"status\":\"ready\",\"networks\":[");
        char itemBuf[160];
        for (int i = 0; i < scanStatus; ++i) {
            if (i > 0) res.sendChunk(",");
            String ssid = WiFi.SSID(i);
            ssid.replace("\\", "\\\\");
            ssid.replace("\"", "\\\"");
            snprintf(itemBuf, sizeof(itemBuf), "{\"ssid\":\"%s\",\"rssi\":%d,\"secure\":%s}",
                     ssid.c_str(), WiFi.RSSI(i),
                     (WiFi.encryptionType(i) != WIFI_AUTH_OPEN) ? "true" : "false");
            res.sendChunk(itemBuf);
        }
        res.sendChunk("]}");
        WiFi.scanDelete();
        res.end();
        return;
    }

    WiFi.scanNetworks(true, false, false, 150);
    res.setContentType("application/json");
    res.sendChunk("{\"status\":\"scanning\",\"networks\":[]}");
    res.end();
}

void WebPortal::handleApiLogs(ResponseWriter &res) {
    res.setContentType("application/json");
    res.sendChunk("[");
    size_t count = logger.getCount();
    char buf[192];
    for (size_t i = 0; i < count; i++) {
        const LogEntry &e = logger.getEntry(i);
        char timeBuf[32];
        if (e.timestamp > 1577836800) {
            time_t t = (time_t)e.timestamp;
            struct tm ti;
            localtime_r(&t, &ti);
            strftime(timeBuf, sizeof(timeBuf), "%H:%M:%S", &ti);
        } else {
            snprintf(timeBuf, sizeof(timeBuf), "+%us", e.timestamp);
        }

        snprintf(buf, sizeof(buf), "%s{\"time\":\"%s\",\"lvl\":%u,\"msg\":\"%s\"}",
                 (i > 0 ? "," : ""), timeBuf, (unsigned)e.level, e.msg);
        res.sendChunk(buf);
    }
    res.sendChunk("]");
    res.end();
}

void WebPortal::handleApiSaveSettings(ResponseWriter &res, const String &body) {
    String devHost = extractFormArg(body, "dev_host");
    if (devHost.length() > 0) {
        _prefs.putString(NVS_KEY_HOSTNAME, devHost);
        mndpDiscovery.setHostname(devHost.c_str());
    }

    bool isStatic = (extractFormArg(body, "ip_static") == "1");
    _prefs.putBool(NVS_KEY_WIFI_DHCP, !isStatic);
    String ipAddr = extractFormArg(body, "ip_addr"); if (ipAddr.length() > 0) _prefs.putString(NVS_KEY_WIFI_IP, ipAddr);
    String ipMask = extractFormArg(body, "ip_mask"); if (ipMask.length() > 0) _prefs.putString(NVS_KEY_WIFI_SN, ipMask);
    String ipGw   = extractFormArg(body, "ip_gw");   if (ipGw.length() > 0)   _prefs.putString(NVS_KEY_WIFI_GW, ipGw);
    String ipDns  = extractFormArg(body, "ip_dns");  if (ipDns.length() > 0)  _prefs.putString(NVS_KEY_WIFI_DNS, ipDns);


    bool ntpEn = (extractFormArg(body, "ntp_en") == "1");
    _prefs.putBool(NVS_KEY_NTP_ENABLED, ntpEn);
    String ntpSrv = extractFormArg(body, "ntp_srv");
    if (ntpSrv.length() > 0) _prefs.putString(NVS_KEY_NTP_SERVER, ntpSrv);

    String tzIdxStr = extractFormArg(body, "tz_idx");
    if (tzIdxStr.length() > 0) {
        int idx = tzIdxStr.toInt();
        if (idx >= 0 && idx < (int)TIMEZONE_COUNT) {
            _prefs.putString(NVS_KEY_NTP_TZ_POSIX, TIMEZONE_LIST[idx].posix);
            _prefs.putString(NVS_KEY_NTP_TZ_CITY, TIMEZONE_LIST[idx].city);
        }
    }
    String timeFmt = extractFormArg(body, "time_fmt");
    if (timeFmt.length() > 0) {
        _prefs.putBool(NVS_KEY_TIME_FORMAT_24H, timeFmt == "24");
    }

    triggerNtpSync();

    String baudStr = extractFormArg(body, "ser_baud");
    if (baudStr.length() > 0) {
        uint32_t baud = baudStr.toInt();
        uint8_t dbits = extractFormArg(body, "ser_dbits").toInt(); if (dbits == 0) dbits = 8;
        uint8_t parity = extractFormArg(body, "ser_parity").toInt();
        uint8_t sbits = extractFormArg(body, "ser_sbits").toInt(); if (sbits == 0) sbits = 1;
        bool echo = (extractFormArg(body, "ser_echo") == "1");
        bool banner = (extractFormArg(body, "ser_banner") == "1");

        _prefs.putUInt(NVS_KEY_SER_BAUD, baud);
        _prefs.putUChar(NVS_KEY_SER_DBITS, dbits);
        _prefs.putUChar(NVS_KEY_SER_PARITY, parity);
        _prefs.putUChar(NVS_KEY_SER_SBITS, sbits);
        _prefs.putBool(NVS_KEY_SER_ECHO, echo);
        _prefs.putBool(NVS_KEY_SER_BANNER, banner);

        serialBridge.updateConfig(baud, dbits, parity, sbits);
        serialBridge.setForceEcho(echo);
        serialBridge.setGreetingBanner(banner);
    }

    String telPortStr = extractFormArg(body, "tel_port");
    if (telPortStr.length() > 0) {
        bool telEn = (extractFormArg(body, "tel_en") == "1");
        uint16_t telPort = (uint16_t)telPortStr.toInt();
        bool telAuth = (extractFormArg(body, "tel_auth") == "1");
        String telPass = extractFormArg(body, "tel_pass");

        _prefs.putBool(NVS_KEY_TELNET_EN, telEn);
        _prefs.putUShort(NVS_KEY_TELNET_PORT, telPort);
        _prefs.putBool(NVS_KEY_TELNET_AUTH, telAuth);
        if (telPass.length() > 0) _prefs.putString(NVS_KEY_TELNET_PASS, telPass);

        telnetServer.setEnabled(telEn);
        telnetServer.setPort(telPort);
        telnetServer.setAuth(telAuth, telPass.length() > 0 ? telPass.c_str() : nullptr);
    }

    String authUsr = extractFormArg(body, "auth_usr");
    if (authUsr.length() > 0) {
        bool authEn = (extractFormArg(body, "auth_en") == "1");
        String authPwd = extractFormArg(body, "auth_pwd");

        _prefs.putBool(NVS_KEY_AUTH_EN, authEn);
        _prefs.putString(NVS_KEY_AUTH_USER, authUsr);
        if (authPwd.length() > 0) _prefs.putString(NVS_KEY_AUTH_PASS, authPwd);

        setAuthCredentials(authEn, authUsr.c_str(), authPwd.length() > 0 ? authPwd.c_str() : _authPass);
        webTerminal.setAuth(authEn, authUsr.c_str(), authPwd.length() > 0 ? authPwd.c_str() : _authPass);
    }

    logger.logInfo("Settings updated successfully.");
    res.setContentType("application/json");
    res.sendChunk("{\"success\":true}");
    res.end();
}

void WebPortal::handleApiSaveWifi(ResponseWriter &res, const String &body) {
    String staSsid = extractFormArg(body, "sta_ssid");
    if (staSsid.length() > 0) {
        _prefs.putString(NVS_KEY_WIFI_SSID, staSsid);
        String staPass = extractFormArg(body, "sta_pass");
        if (staPass.length() > 0) _prefs.putString(NVS_KEY_WIFI_PASS, staPass);
    }

    String apSsid = extractFormArg(body, "ap_ssid");
    if (apSsid.length() > 0) _prefs.putString(NVS_KEY_AP_SSID, apSsid);
    String apPass = extractFormArg(body, "ap_pass");
    if (apPass.length() >= 8) _prefs.putString(NVS_KEY_AP_PASS, apPass);
    String apChan = extractFormArg(body, "ap_chan");
    if (apChan.length() > 0) _prefs.putUChar(NVS_KEY_AP_CHAN, (uint8_t)apChan.toInt());

    _prefs.putBool(NVS_KEY_AP_HIDDEN, extractFormArg(body, "ap_hidden") == "1");
    _prefs.putBool(NVS_KEY_AP_CAPTIVE, extractFormArg(body, "ap_captive") == "1");
    _prefs.putBool(NVS_KEY_MNDP_EN, extractFormArg(body, "mndp_en") == "1");

    logger.logInfo("Wi-Fi configuration saved. Rebooting ESP32 to apply network changes...");
    res.setContentType("application/json");
    res.sendChunk("{\"success\":true,\"reboot\":true}");
    res.end();
    delay(800);
    ESP.restart();
}

void WebPortal::handleApiRestart(ResponseWriter &res) {
    logger.logWarn("Reboot triggered via Web API.");
    res.setContentType("application/json");
    res.sendChunk("{\"rebooting\":true}");
    res.end();
    delay(500);
    ESP.restart();
}

void WebPortal::handleApiFactoryReset(ResponseWriter &res) {
    logger.logWarn("Factory Reset triggered via Web API.");
    _prefs.clear();
    res.setContentType("application/json");
    res.sendChunk("{\"reset\":true}");
    res.end();
    delay(500);
    ESP.restart();
}

void WebPortal::handleApiPlatform(ResponseWriter &res, const String &platform) {
    if (platform.length() > 0) {
        _prefs.putString(NVS_KEY_CLI_PLATFORM, platform);
    }
    res.setContentType("application/json");
    res.sendChunk("{\"success\":true}");
    res.end();
}

// =============================================================================
// Captive Portal & Not Found Handlers
// =============================================================================
void WebPortal::handleCaptivePortal() {
    IPAddress ip = WiFi.softAPIP();
    _server.sendHeader("Location", "http://" + ip.toString() + "/");
    _server.send(302, "text/plain", "");
}

void WebPortal::handleNotFound() {
    if ((WiFi.getMode() & WIFI_MODE_AP) && _captiveEnabled) {
        String host = _server.hostHeader();
        String apIp = WiFi.softAPIP().toString();
        if (host.length() > 0 && !host.startsWith(apIp) && !host.startsWith("esp-oobm")) {
            handleCaptivePortal();
            return;
        }
    }
    _server.send(404, "text/plain", "404: Not Found");
}

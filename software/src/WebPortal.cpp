#include "WebPortal.h"
#include "VectorGraphics.h"
#include "Timezones.h"
#include "SerialBridge.h"
#include "TelnetServer.h"
#include "WebTerminal.h"
#include "PrometheusExporter.h"
#include "MndpDiscovery.h"
#include "ConsoleLogger.h"
#include <Update.h>
#include <esp_sntp.h>

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
html.light .brand-title-main{fill:#0f172a!important;}
html.light .brand-title-prefix{fill:#0284c7!important;}
html.light .brand-sub{fill:#059669!important;}
html:not(.light) .brand-title-main{fill:#f8fafc!important;}
html:not(.light) .brand-title-prefix{fill:#38bdf8!important;}
html:not(.light) .brand-sub{fill:#34d399!important;}
.nav-links{display:flex;gap:8px;align-items:center;}
.nav-link{padding:6px 14px;border-radius:6px;font-size:0.90rem;font-weight:600;text-decoration:none;color:var(--muted);border:1px solid transparent;transition:all 0.15s;}
.nav-link:hover{color:var(--text);background:var(--hover);}
.nav-link.active{background:var(--navy);color:#ffffff;border-color:var(--navy);box-shadow:0 2px 6px rgba(2,132,199,0.3);}
.badge-clock{font-family:Consolas,monospace;font-size:0.82rem;padding:4px 9px;border-radius:6px;background:var(--hover);border:1px solid var(--border);color:var(--muted);display:flex;align-items:center;gap:6px;}

.theme-switch{display:inline-flex;align-items:center;background:var(--hover);border:1px solid var(--border);border-radius:20px;padding:2px;gap:2px;}
.theme-btn{background:transparent;border:none;border-radius:16px;padding:4px 7px;display:inline-flex;align-items:center;justify-content:center;color:var(--muted);cursor:pointer;transition:all 0.15s;}
.theme-btn:hover{color:var(--text);}
.theme-btn.active{background:var(--card);color:var(--text);box-shadow:0 1px 3px rgba(0,0,0,0.25);}

@media(min-width:961px){
  .nav-links{position:absolute;left:50%;transform:translateX(-50%);}
}
@media(min-width:1250px){
  .theme-switch{position:absolute;right:24px;top:50%;transform:translateY(-50%);}
}
@media(max-width:1249px){
  .nav-inner{flex-wrap:wrap;gap:12px;}
  .theme-switch{margin-left:auto;}
}
@media(max-width:960px){
  .header-wrap{position:static;}
  .navbar{padding:10px 0;}
  .nav-inner{flex-direction:column;align-items:center;gap:10px;}
  .nav-links{position:static;transform:none;}
  .theme-switch{margin-left:0;}
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

.btn{padding:8px 16px;border-radius:6px;font-size:0.88rem;font-weight:600;text-decoration:none;display:inline-flex;align-items:center;gap:6px;border:none;cursor:pointer;transition:all 0.18s;}
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
.form-group{display:flex;flex-direction:column;gap:6px;}
.form-group label{font-size:0.84rem;font-weight:600;color:var(--muted);}
.form-control{padding:8px 12px;border-radius:6px;border:1px solid var(--border);background:var(--bg);color:var(--text);font-size:0.88rem;outline:none;transition:border-color 0.15s;}
.form-control:focus{border-color:var(--navy);}
.switch-label{display:inline-flex;align-items:center;gap:10px;cursor:pointer;user-select:none;font-size:0.88rem;font-weight:500;}

.term-container{background:#000000;border:1px solid var(--border);border-radius:8px;padding:12px;font-family:Consolas,Courier,monospace;font-size:0.92rem;color:#e2e8f0;height:calc(100vh - 280px);min-height:360px;overflow-y:auto;white-space:pre-wrap;word-break:break-all;outline:none;box-shadow:inset 0 2px 8px rgba(0,0,0,0.6);}
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
)rawliteral";

// =============================================================================
// Constructor & Initialization
// =============================================================================
WebPortal::WebPortal(WebServer &server, DNSServer &dnsServer, Preferences &prefs)
    : _server(server),
      _dnsServer(dnsServer),
      _prefs(prefs),
      _authRequired(false),
      _isApMode(false),
      _captiveEnabled(true) {
    strncpy(_authUser, "admin", sizeof(_authUser) - 1);
    strncpy(_authPass, "admin", sizeof(_authPass) - 1);
}

void WebPortal::setAuthCredentials(bool enabled, const char *user, const char *pass) {
    _authRequired = enabled;
    if (user) strncpy(_authUser, user, sizeof(_authUser) - 1);
    if (pass) strncpy(_authPass, pass, sizeof(_authPass) - 1);
}

bool WebPortal::checkAuth() {
    if (!_authRequired) return true;
    if (_server.authenticate(_authUser, _authPass)) return true;
    _server.requestAuthentication(BASIC_AUTH, "ESP-OOBM");
    return false;
}

void WebPortal::begin() {
    _authRequired = _prefs.getBool(NVS_KEY_AUTH_EN, false);
    String u = _prefs.getString(NVS_KEY_AUTH_USER, "admin");
    String p = _prefs.getString(NVS_KEY_AUTH_PASS, "admin");
    strncpy(_authUser, u.c_str(), sizeof(_authUser) - 1);
    strncpy(_authPass, p.c_str(), sizeof(_authPass) - 1);

    _isApMode = (WiFi.getMode() & WIFI_MODE_AP);
    _captiveEnabled = _prefs.getBool(NVS_KEY_AP_CAPTIVE, true);

    // Setup DNS Server for Captive Portal if in AP mode
    if (_isApMode && _captiveEnabled) {
        _dnsServer.start(DNS_PORT, "*", IPAddress(AP_IP_ADDRESS));
    }

    // HTML Page Routes
    _server.on("/", HTTP_GET, [this]() { handleRoot(); });
    _server.on("/terminal", HTTP_GET, [this]() { handleTerminal(); });
    _server.on("/settings", HTTP_GET, [this]() { handleSettings(); });
    _server.on("/wifi", HTTP_GET, [this]() { handleWifiPage(); });
    _server.on("/update", HTTP_GET, [this]() { handleUpdatePage(); });
    _server.on("/update", HTTP_POST, [this]() { handleUpdateFinish(); }, [this]() { handleUpdateUpload(); });
    _server.on("/metrics", HTTP_GET, [this]() { handleMetrics(); });
    _server.on("/sync_ntp", HTTP_GET, [this]() { handleSyncNtp(); });
    _server.on("/reset_wifi", HTTP_GET, [this]() { handleResetWifi(); });

    // Favicon Routes
    _server.on("/favicon.svg", HTTP_GET, [this]() {
        _server.send_P(200, "image/svg+xml", OOBM_FAVICON_SVG);
    });
    _server.on("/favicon.ico", HTTP_GET, [this]() {
        _server.send_P(200, "image/svg+xml", OOBM_FAVICON_SVG);
    });

    // AJAX API Routes
    _server.on("/api/status", HTTP_GET, [this]() { handleApiStatus(); });
    _server.on("/api/scan", HTTP_GET, [this]() { handleApiScan(); });
    _server.on("/api/logs", HTTP_GET, [this]() { handleApiLogs(); });
    _server.on("/api/ntp/sync", HTTP_POST, [this]() { handleSyncNtp(); });
    _server.on("/api/settings/save", HTTP_POST, [this]() { handleApiSaveSettings(); });
    _server.on("/api/wifi/save", HTTP_POST, [this]() { handleApiSaveWifi(); });
    _server.on("/api/restart", HTTP_POST, [this]() { handleApiRestart(); });
    _server.on("/api/factory_reset", HTTP_POST, [this]() { handleApiFactoryReset(); });
    _server.on("/api/platform", HTTP_POST, [this]() { handleApiPlatform(); });

    // Captive Portal Probes
    _server.on("/generate_204", HTTP_GET, [this]() { handleCaptivePortal(); });
    _server.on("/hotspot-detect.html", HTTP_GET, [this]() { handleCaptivePortal(); });
    _server.on("/ncsi.txt", HTTP_GET, [this]() { handleCaptivePortal(); });
    _server.on("/connecttest.txt", HTTP_GET, [this]() { handleCaptivePortal(); });

    _server.onNotFound([this]() { handleNotFound(); });

    _server.begin();
    logger.logInfo("Web server started on port %u (Auth: %s)", HTTP_PORT, _authRequired ? "Enabled" : "Disabled");
}

void WebPortal::loop() {
    if (_isApMode && _captiveEnabled) {
        _dnsServer.processNextRequest();
    }
    _server.handleClient();
}

// =============================================================================
// HTML Helpers (PROGMEM Streaming)
// =============================================================================
void WebPortal::streamHeader(const char *activeTab, const char *title) {
    _server.setContentLength(CONTENT_LENGTH_UNKNOWN);
    _server.send(200, "text/html; charset=utf-8", "");

    time_t nowT = time(nullptr);
    bool is24h = _prefs.getBool(NVS_KEY_TIME_FORMAT_24H, true);
    char curTimeBuf[32];
    if (nowT > 1577836800) {
        struct tm ti;
        localtime_r(&nowT, &ti);
        if (is24h) {
            strftime(curTimeBuf, sizeof(curTimeBuf), "%Y-%m-%d %H:%M", &ti);
        } else {
            strftime(curTimeBuf, sizeof(curTimeBuf), "%Y-%m-%d %I:%M %p", &ti);
        }
    } else {
        snprintf(curTimeBuf, sizeof(curTimeBuf), "--:--");
    }

    _server.sendContent_P(PSTR("<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\">"
                               "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
                               "<title>"));
    _server.sendContent(title);
    _server.sendContent_P(PSTR(" - ESP-OOBM</title>"
                               "<link rel=\"icon\" type=\"image/svg+xml\" href=\"/favicon.svg\">"
                               "<link rel=\"icon\" type=\"image/x-icon\" href=\"/favicon.ico\">"
                               "<style>"));
    _server.sendContent_P(COMMON_CSS);
    _server.sendContent_P(PSTR("</style><script>"));
    _server.sendContent_P(COMMON_JS);
    _server.sendContent_P(PSTR("</script></head><body><div class=\"header-wrap\"><div class=\"top-accent\"></div>"
                               "<div class=\"navbar\"><div class=\"nav-inner\"><a href=\"/\" class=\"brand\">"));
    _server.sendContent_P(OOBM_LOGO_SVG);
    _server.sendContent_P(PSTR("</a><nav class=\"nav-links\">"
                               "<a href=\"/\" class=\"nav-link "));
    if (strcmp(activeTab, "dashboard") == 0) _server.sendContent_P(PSTR("active"));
    _server.sendContent_P(PSTR("\">Dashboard</a>"
                               "<a href=\"/terminal\" class=\"nav-link "));
    if (strcmp(activeTab, "terminal") == 0) _server.sendContent_P(PSTR("active"));
    _server.sendContent_P(PSTR("\">Terminal</a>"
                               "<a href=\"/settings\" class=\"nav-link "));
    if (strcmp(activeTab, "settings") == 0) _server.sendContent_P(PSTR("active"));
    _server.sendContent_P(PSTR("\">Settings</a></nav><div id=\"clock_badge\" class=\"badge-clock\">"));
    _server.sendContent(curTimeBuf);
    _server.sendContent_P(PSTR("</div></div>"
                               "<div class=\"theme-switch\" role=\"group\" aria-label=\"Theme switcher\">"
                               "  <button id=\"themeBtnLight\" class=\"theme-btn\" onclick=\"setTheme('light')\" title=\"Light Theme\">"
                               "    <svg width=\"14\" height=\"14\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\"><circle cx=\"12\" cy=\"12\" r=\"4\"/><path d=\"M12 2v2M12 20v2M4.93 4.93l1.41 1.41M17.66 17.66l1.41 1.41M2 12h2M20 12h2M6.34 17.66l-1.41 1.41M19.07 4.93l-1.41 1.41\"/></svg>"
                               "  </button>"
                               "  <button id=\"themeBtnDark\" class=\"theme-btn\" onclick=\"setTheme('dark')\" title=\"Dark Theme\">"
                               "    <svg width=\"14\" height=\"14\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\"><path d=\"M21 12.79A9 9 0 1 1 11.21 3 7 7 0 0 0 21 12.79z\"/></svg>"
                               "  </button>"
                               "  <button id=\"themeBtnSystem\" class=\"theme-btn\" onclick=\"setTheme('system')\" title=\"System Theme\">"
                               "    <svg width=\"14\" height=\"14\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\"><rect x=\"2\" y=\"3\" width=\"20\" height=\"14\" rx=\"2\"/><line x1=\"8\" y1=\"21\" x2=\"16\" y2=\"21\"/><line x1=\"12\" y1=\"17\" x2=\"12\" y2=\"21\"/></svg>"
                               "  </button>"
                               "</div>"
                               "</div></div><div class=\"container\">"));

    if (WiFi.status() != WL_CONNECTED) {
        _server.sendContent_P(PSTR(
            "<div class=\"ap-banner\">"
            "  <span>&#9888; <b>Standalone AP Mode (192.168.4.1)</b> &bull; Wi-Fi not configured or offline</span>"
            "  <a href=\"/wifi\">Configure Wi-Fi &rarr;</a>"
            "</div>"
        ));
    }
}

void WebPortal::streamFooter() {
    _server.sendContent_P(PSTR(
        "<footer style=\"margin-top:40px;padding:20px 0;border-top:1px solid var(--border);text-align:center;font-size:0.80rem;color:var(--muted);\">"
        "  ESP-OOBM Firmware <b>v" FIRMWARE_VERSION "</b> &bull; Built on " FIRMWARE_BUILD_DATE " " FIRMWARE_BUILD_TIME
        "</footer>"
        "</div><script>"
        "function pollStatus(){"
        "  fetch('/api/status').then(r=>r.json()).then(d=>{"
        "    if(d.time_str){"
        "      var cb = document.getElementById('clock_badge');"
        "      if(cb) cb.innerText = d.time_str;"
        "    }"
        "    if(typeof updateDashboardUI === 'function') updateDashboardUI(d);"
        "  }).catch(()=>{});"
        "}"
        "setInterval(pollStatus, 2500);"
        "</script></body></html>"
    ));
    _server.sendContent("");
}

// =============================================================================
// Dashboard Page
// =============================================================================
void WebPortal::handleRoot() {
    if (!checkAuth()) return;

    streamHeader("dashboard", "Dashboard");

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
        struct tm timeinfo;
        localtime_r(&stats.currentTime, &timeinfo);
        if (is24h) {
            strftime(timeBuf, sizeof(timeBuf), "%Y-%m-%d %H:%M", &timeinfo);
        } else {
            strftime(timeBuf, sizeof(timeBuf), "%Y-%m-%d %I:%M %p", &timeinfo);
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

    String netMode = stats.wifiStaConnected ? "STATION" : (stats.wifiApActive ? "ACCESS POINT" : "DISCONNECTED");
    String tzCity = _prefs.getString(NVS_KEY_NTP_TZ_CITY, DEFAULT_TZ_CITY);
    String ntpServer = _prefs.getString(NVS_KEY_NTP_SERVER, NTP_DEFAULT_SERVER);

    String dHtml = "";
    dHtml.reserve(4000);
    dHtml += "<div class=\"grid-cards\">\n";
    // 1. System Health
    dHtml += "  <div class=\"card\">\n";
    dHtml += "    <h3>System Health <span id=\"cpu_badge\" style=\"font-size:0.75rem;color:var(--navy);\">" + String(stats.cpuFreqMhz) + " MHz</span></h3>\n";
    dHtml += "    <div class=\"val val-accent\" id=\"uptime_val\">" + String(uptimeBuf) + "</div>\n";
    dHtml += "    <div class=\"sub\">Uptime</div>\n";
    dHtml += "    <div style=\"margin-top:14px;\">\n";
    dHtml += "      <div class=\"stat-row\"><span class=\"stat-label\">Free Heap</span><span class=\"stat-val\" id=\"heap_val\">" + String(heapBuf) + "</span></div>\n";
    dHtml += "      <div class=\"stat-row\"><span class=\"stat-label\">Min Free Heap</span><span class=\"stat-val\" id=\"min_heap_val\">" + String(minHeapBuf) + "</span></div>\n";
    dHtml += "      <div class=\"stat-row\"><span class=\"stat-label\">Heap Fragmentation</span><span class=\"stat-val\" id=\"frag_val\">" + String(stats.heapFragPercent) + "%</span></div>\n";
    dHtml += "    </div>\n";
    dHtml += "  </div>\n";

    // 2. Serial Interface
    dHtml += "  <div class=\"card\">\n";
    dHtml += "    <h3>Serial Interface <span id=\"baud_badge\" style=\"font-size:0.75rem;color:var(--green);\">" + String(stats.baudRate) + " " + String(framing) + "</span></h3>\n";
    dHtml += "    <div class=\"val val-green\" id=\"rx_bytes_val\">" + String(rxBuf) + "</div>\n";
    dHtml += "    <div class=\"sub\">RX Bytes Received</div>\n";
    dHtml += "    <div style=\"margin-top:14px;\">\n";
    dHtml += "      <div class=\"stat-row\"><span class=\"stat-label\">TX Bytes Sent</span><span class=\"stat-val\" id=\"tx_bytes_val\">" + String(txBuf) + "</span></div>\n";
    dHtml += "      <div class=\"stat-row\"><span class=\"stat-label\">RX Overflows</span><span class=\"stat-val\" id=\"overflow_val\">" + String(stats.serialRxOverflow) + "</span></div>\n";
    dHtml += "      <div class=\"stat-row\"><span class=\"stat-label\">Active Sessions</span><span class=\"stat-val\" id=\"sessions_val\">WS: " + String(stats.activeWsClients) + " | Tel: " + String(stats.activeTelnetClients) + "</span></div>\n";
    dHtml += "    </div>\n";
    dHtml += "  </div>\n";

    // 3. Network & WiFi
    dHtml += "  <div class=\"card\">\n";
    dHtml += "    <h3>Network & WiFi <span id=\"net_mode_badge\" style=\"font-size:0.75rem;color:var(--cyan);\">" + netMode + "</span></h3>\n";
    dHtml += "    <div class=\"val\" id=\"ip_val\">" + String(stats.ipAddress) + "</div>\n";
    dHtml += "    <div class=\"sub\" id=\"ssid_val\">SSID: " + (stats.ssid[0] ? String(stats.ssid) : "(None)") + "</div>\n";
    dHtml += "    <div style=\"margin-top:14px;\">\n";
    dHtml += "      <div class=\"stat-row\"><span class=\"stat-label\">MAC Address</span><span class=\"stat-val\" id=\"mac_val\">" + String(stats.macAddress) + "</span></div>\n";
    dHtml += "      <div class=\"stat-row\"><span class=\"stat-label\">Signal RSSI</span><span class=\"stat-val\" id=\"rssi_val\">" + (stats.wifiStaConnected ? (String(stats.wifiRssi) + " dBm") : "N/A") + "</span></div>\n";
    dHtml += "      <div class=\"stat-row\"><span class=\"stat-label\">Connected AP Clients</span><span class=\"stat-val\" id=\"ap_clients_val\">" + String(stats.wifiApClients) + "</span></div>\n";
    dHtml += "    </div>\n";
    dHtml += "  </div>\n";

    // 4. Time & Clock
    dHtml += "  <div class=\"card\">\n";
    dHtml += "    <h3>Time &amp; Clock</h3>\n";
    dHtml += "    <div class=\"val\" id=\"time_val\" style=\"font-size:1.6rem;letter-spacing:-0.02em;\">" + String(timeBuf) + "</div>\n";
    dHtml += "    <div class=\"sub\" id=\"tz_val\">TZ: " + tzCity + "</div>\n";
    dHtml += "    <div style=\"margin-top:14px;\">\n";
    dHtml += "      <div class=\"stat-row\"><span class=\"stat-label\">NTP Status</span><span class=\"stat-val\"><span id=\"ntp_badge\" class=\"badge " + String(stats.ntpSynced ? "badge-ok\">Synced (OK)" : "badge-warn\">Waiting for sync") + "</span></span></div>\n";
    dHtml += "      <div class=\"stat-row\"><span class=\"stat-label\">NTP Server</span><span class=\"stat-val\" id=\"ntp_srv_val\">" + ntpServer + "</span></div>\n";
    dHtml += "      <div class=\"stat-row\"><span class=\"stat-label\">Last NTP Sync</span><span class=\"stat-val\" id=\"last_ntp_val\">" + String(lastNtpBuf) + "</span></div>\n";
    dHtml += "    </div>\n";
    dHtml += "  </div>\n";
    dHtml += "</div>\n";

    // 5. System Event Log Card
    dHtml += "<div class=\"table-card\" style=\"margin-top:20px;\">\n";
    dHtml += "  <div style=\"display:flex;justify-content:space-between;align-items:center;margin-bottom:12px;\">\n";
    dHtml += "    <h3 style=\"margin:0;color:var(--navy);\">&#128220; System Event Log</h3>\n";
    dHtml += "    <button type=\"button\" class=\"btn btn-outline btn-sm\" onclick=\"fetchLogs()\">&#128260; Refresh Log</button>\n";
    dHtml += "  </div>\n";
    dHtml += "  <div id=\"log_box\" class=\"log-box\">";
    size_t lCount = logger.getCount();
    if (lCount == 0) {
        dHtml += "<span style=\"color:var(--muted);\">No log entries recorded yet.</span>";
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
            dHtml += "<span style=\"color:var(--muted);\">" + String(lTimeBuf) + "</span> <span class=\"" + String(cls) + "\">" + String(prefix) + "</span> " + String(e.msg) + "\n";
        }
    }
    dHtml += "</div>\n";
    dHtml += "</div>\n";

    dHtml += "<script>\n";
    dHtml += "function fetchLogs(){\n";
    dHtml += "  fetch('/api/logs').then(r=>r.json()).then(entries=>{\n";
    dHtml += "    var box = document.getElementById('log_box');\n";
    dHtml += "    if(!box) return;\n";
    dHtml += "    if(!entries || entries.length === 0){ box.innerHTML = '<span style=\"color:var(--muted);\">No log entries recorded yet.</span>'; return; }\n";
    dHtml += "    var html = '';\n";
    dHtml += "    entries.forEach(function(e){\n";
    dHtml += "      var cls = e.lvl === 2 ? 'log-error' : (e.lvl === 1 ? 'log-warn' : 'log-info');\n";
    dHtml += "      var prefix = e.lvl === 2 ? '[ERROR]' : (e.lvl === 1 ? '[WARN]' : '[INFO]');\n";
    dHtml += "      html += '<span style=\"color:var(--muted);\">' + e.time + '</span> ' +\n";
    dHtml += "              '<span class=\"' + cls + '\">' + prefix + '</span> ' +\n";
    dHtml += "              e.msg + '\\n';\n";
    dHtml += "    });\n";
    dHtml += "    box.innerHTML = html;\n";
    dHtml += "    box.scrollTop = box.scrollHeight;\n";
    dHtml += "  }).catch(()=>{});\n";
    dHtml += "}\n";
    dHtml += "(function(){ var b = document.getElementById('log_box'); if(b) b.scrollTop = b.scrollHeight; })();\n";
    dHtml += "function updateDashboardUI(d){\n";
    dHtml += "  document.getElementById('uptime_val').innerText = d.uptime_str || '--';\n";
    dHtml += "  document.getElementById('cpu_badge').innerText = d.cpu_freq + ' MHz';\n";
    dHtml += "  document.getElementById('heap_val').innerText = d.free_heap_str || (Math.round(d.free_heap/1024) + ' KB');\n";
    dHtml += "  document.getElementById('min_heap_val').innerText = d.min_heap_str || (Math.round(d.min_free_heap/1024) + ' KB');\n";
    dHtml += "  document.getElementById('frag_val').innerText = d.heap_frag + '%';\n";
    dHtml += "  document.getElementById('baud_badge').innerText = d.baud + ' ' + d.framing;\n";
    dHtml += "  document.getElementById('rx_bytes_val').innerText = d.rx_bytes_str || (d.rx_bytes + ' B');\n";
    dHtml += "  document.getElementById('tx_bytes_val').innerText = d.tx_bytes_str || (d.tx_bytes + ' B');\n";
    dHtml += "  document.getElementById('overflow_val').innerText = d.rx_overflow;\n";
    dHtml += "  document.getElementById('sessions_val').innerText = 'WS: ' + d.active_ws + ' | Tel: ' + d.active_telnet;\n";
    dHtml += "  document.getElementById('net_mode_badge').innerText = d.net_mode;\n";
    dHtml += "  document.getElementById('ip_val').innerText = d.ip;\n";
    dHtml += "  document.getElementById('ssid_val').innerText = 'SSID: ' + (d.ssid || '(None)');\n";
    dHtml += "  document.getElementById('mac_val').innerText = d.mac;\n";
    dHtml += "  document.getElementById('rssi_val').innerText = d.rssi ? (d.rssi + ' dBm') : 'N/A';\n";
    dHtml += "  document.getElementById('ap_clients_val').innerText = d.ap_clients || 0;\n";
    dHtml += "  document.getElementById('ntp_badge').innerText = d.ntp_synced ? 'Synced (OK)' : 'Waiting for sync';\n";
    dHtml += "  document.getElementById('ntp_badge').className = d.ntp_synced ? 'badge badge-ok' : 'badge badge-warn';\n";
    dHtml += "  document.getElementById('tz_val').innerText = 'TZ: ' + (d.tz_city || 'UTC');\n";
    dHtml += "  document.getElementById('ntp_srv_val').innerText = d.ntp_server || 'pool.ntp.org';\n";
    dHtml += "  document.getElementById('last_ntp_val').innerText = d.last_ntp_str || '--';\n";
    dHtml += "}\n";
    dHtml += "</script>\n";

    _server.sendContent(dHtml);
    streamFooter();
}

// =============================================================================
// Terminal Page
// =============================================================================
void WebPortal::handleTerminal() {
    if (!checkAuth()) return;

    streamHeader("terminal", "Serial Console");

    String curPlatform = _prefs.getString(NVS_KEY_CLI_PLATFORM, "mikrotik");

    String t = "";
    t.reserve(5500);

    // Top Controls Bar (Status left, Keys middle, Clear/Reconnect right)
    t += "<div style=\"display:flex;justify-content:space-between;align-items:center;flex-wrap:wrap;gap:8px;margin-bottom:10px;\">\n";
    t += "  <div style=\"font-size:0.88rem;font-weight:600;display:flex;align-items:center;gap:8px;white-space:nowrap;\">\n";
    t += "    <span id=\"ws_status_dot\" style=\"display:inline-block;width:10px;height:10px;border-radius:50%;background:var(--warn);\"></span>\n";
    t += "    <span id=\"ws_status_text\">Connecting...</span>\n";
    t += "  </div>\n";
    t += "  <div style=\"display:flex;align-items:center;gap:4px;flex-wrap:wrap;\">\n";
    t += "    <span style=\"font-size:0.75rem;font-weight:700;color:var(--muted);margin-right:2px;\">KEYS:</span>\n";
    t += "    <button class=\"term-key\" onclick=\"sendSpecialKey(27)\">ESC</button>\n";
    t += "    <button class=\"term-key\" onclick=\"sendSpecialKey(9)\">TAB</button>\n";
    t += "    <button class=\"term-key\" onclick=\"sendSpecialKey(3)\">Ctrl+C</button>\n";
    t += "    <button class=\"term-key\" onclick=\"sendSpecialKey(26)\">Ctrl+Z</button>\n";
    t += "    <button class=\"term-key\" onclick=\"sendSpecialKey(4)\">Ctrl+D</button>\n";
    t += "    <button class=\"term-key\" onclick=\"sendEscapeSeq('[A')\">&uarr;</button>\n";
    t += "    <button class=\"term-key\" onclick=\"sendEscapeSeq('[B')\">&darr;</button>\n";
    t += "    <button class=\"term-key\" onclick=\"sendEscapeSeq('[D')\">&larr;</button>\n";
    t += "    <button class=\"term-key\" onclick=\"sendEscapeSeq('[C')\">&rarr;</button>\n";
    t += "    <button class=\"term-key\" onclick=\"sendSpecialKey(13)\">ENTER</button>\n";
    t += "  </div>\n";
    t += "  <div style=\"display:flex;gap:6px;\">\n";
    t += "    <button class=\"btn btn-outline btn-sm\" onclick=\"clearTerminal()\">Clear</button>\n";
    t += "    <button class=\"btn btn-outline btn-sm\" onclick=\"reconnectWs()\">Reconnect</button>\n";
    t += "  </div>\n";
    t += "</div>\n";

    // Responsive Terminal Screen
    t += "<div id=\"terminal\" class=\"term-container\" tabindex=\"0\"></div>\n";

    // Bottom Bar: Platform Dropdown & Dynamic Quick Commands
    t += "<div class=\"table-card\" style=\"margin-top:10px;padding:8px 14px;display:flex;align-items:center;gap:14px;flex-wrap:wrap;\">\n";
    t += "  <div style=\"display:flex;align-items:center;gap:6px;\">\n";
    t += "    <span style=\"font-size:0.80rem;font-weight:700;color:var(--muted);white-space:nowrap;\">PLATFORM:</span>\n";
    t += "    <select id=\"platform_select\" class=\"form-control\" style=\"padding:4px 8px;font-size:0.82rem;font-weight:600;width:auto;\" onchange=\"changePlatform(this.value)\">\n";
    t += "      <option value=\"mikrotik\"" + String(curPlatform == "mikrotik" ? " selected" : "") + ">MikroTik RouterOS</option>\n";
    t += "      <option value=\"linux\"" + String(curPlatform == "linux" ? " selected" : "") + ">Linux / OpenWrt / EdgeOS</option>\n";
    t += "      <option value=\"cisco\"" + String(curPlatform == "cisco" ? " selected" : "") + ">Cisco IOS / Catalyst</option>\n";
    t += "      <option value=\"pfsense\"" + String(curPlatform == "pfsense" ? " selected" : "") + ">pfSense / FreeBSD / OPNsense</option>\n";
    t += "      <option value=\"generic\"" + String(curPlatform == "generic" ? " selected" : "") + ">Generic Diagnostics</option>\n";
    t += "    </select>\n";
    t += "  </div>\n";
    t += "  <div style=\"display:flex;align-items:center;gap:6px;flex-wrap:wrap;\">\n";
    t += "    <span style=\"font-size:0.80rem;font-weight:700;color:var(--muted);margin-right:2px;\">QUICK COMMANDS:</span>\n";
    t += "    <div id=\"quick_cmds\" style=\"display:inline-flex;gap:6px;flex-wrap:wrap;\"></div>\n";
    t += "  </div>\n";
    t += "</div>\n";

    // Terminal JavaScript
    t += "<script>\n";
    t += "var ws, term = document.getElementById('terminal'), dot = document.getElementById('ws_status_dot'), stext = document.getElementById('ws_status_text');\n";
    t += "var PLATFORM_CMDS = {\n";
    t += "  mikrotik: [\n";
    t += "    { label: '/system resource print', cmd: '/system resource print' },\n";
    t += "    { label: '/ip address print', cmd: '/ip address print' },\n";
    t += "    { label: '/interface print', cmd: '/interface print' },\n";
    t += "    { label: '/log print', cmd: '/log print follow-only' },\n";
    t += "    { label: '/ip route print', cmd: '/ip route print' }\n";
    t += "  ],\n";
    t += "  linux: [\n";
    t += "    { label: 'ip addr show', cmd: 'ip addr show' },\n";
    t += "    { label: 'dmesg | tail -n 20', cmd: 'dmesg -T | tail -n 20' },\n";
    t += "    { label: 'logread -f', cmd: 'logread -f' },\n";
    t += "    { label: 'systemctl status', cmd: 'systemctl status' },\n";
    t += "    { label: 'df -h', cmd: 'df -h' }\n";
    t += "  ],\n";
    t += "  cisco: [\n";
    t += "    { label: 'show ip int brief', cmd: 'show ip interface brief' },\n";
    t += "    { label: 'show running-config', cmd: 'show running-config' },\n";
    t += "    { label: 'show int status', cmd: 'show interfaces status' },\n";
    t += "    { label: 'show logging', cmd: 'show logging' },\n";
    t += "    { label: 'term len 0', cmd: 'terminal length 0' }\n";
    t += "  ],\n";
    t += "  pfsense: [\n";
    t += "    { label: 'ifconfig', cmd: 'ifconfig' },\n";
    t += "    { label: 'pfctl -d (Disable FW)', cmd: 'pfctl -d' },\n";
    t += "    { label: 'pfctl -e (Enable FW)', cmd: 'pfctl -e' },\n";
    t += "    { label: 'top -b', cmd: 'top -b' },\n";
    t += "    { label: 'netstat -rn', cmd: 'netstat -rn' }\n";
    t += "  ],\n";
    t += "  generic: [\n";
    t += "    { label: 'help', cmd: 'help' },\n";
    t += "    { label: 'status', cmd: 'status' },\n";
    t += "    { label: 'version', cmd: 'version' },\n";
    t += "    { label: 'ping 8.8.8.8', cmd: 'ping 8.8.8.8' },\n";
    t += "    { label: 'reboot', cmd: 'reboot' }\n";
    t += "  ]\n";
    t += "};\n";
    t += "function renderQuickCmds(p){\n";
    t += "  var list = PLATFORM_CMDS[p] || PLATFORM_CMDS.mikrotik;\n";
    t += "  var c = document.getElementById('quick_cmds');\n";
    t += "  if(!c) return;\n";
    t += "  c.innerHTML = '';\n";
    t += "  list.forEach(function(item){\n";
    t += "    var btn = document.createElement('button');\n";
    t += "    btn.className = 'btn btn-outline btn-sm';\n";
    t += "    btn.innerText = item.label;\n";
    t += "    btn.onclick = function(){ sendCmd(item.cmd); };\n";
    t += "    c.appendChild(btn);\n";
    t += "  });\n";
    t += "}\n";
    t += "function changePlatform(p){\n";
    t += "  renderQuickCmds(p);\n";
    t += "  try{ localStorage.setItem('oobm_cli_platform', p); }catch(e){}\n";
    t += "  fetch('/api/platform?p=' + encodeURIComponent(p), { method: 'POST' }).catch(()=>{});\n";
    t += "}\n";
    t += "function initWs(){\n";
    t += "  var proto = location.protocol === 'https:' ? 'wss:' : 'ws:';\n";
    t += "  ws = new WebSocket(proto + '//' + location.hostname + ':81');\n";
    t += "  ws.binaryType = 'arraybuffer';\n";
    t += "  ws.onopen = function(){\n";
    t += "    dot.style.background = 'var(--green)';\n";
    t += "    stext.innerText = 'Connected (Port 81)';\n";
    t += "  };\n";
    t += "  ws.onclose = function(){\n";
    t += "    dot.style.background = 'var(--danger)';\n";
    t += "    stext.innerText = 'Disconnected';\n";
    t += "  };\n";
    t += "  ws.onmessage = function(e){\n";
    t += "    var text = '';\n";
    t += "    if(e.data instanceof ArrayBuffer){\n";
    t += "      text = new TextDecoder().decode(e.data);\n";
    t += "    } else { text = e.data; }\n";
    t += "    appendAnsiText(text);\n";
    t += "  };\n";
    t += "}\n";
    t += "function appendAnsiText(str){\n";
    t += "  var clean = str.replace(/\\x1b\\[[0-9;]*[a-zA-Z]/g, function(match){ return ''; });\n";
    t += "  term.textContent += clean;\n";
    t += "  term.scrollTop = term.scrollHeight;\n";
    t += "}\n";
    t += "function sendSpecialKey(code){ if(ws && ws.readyState === 1){ ws.send(new Uint8Array([code])); } }\n";
    t += "function sendEscapeSeq(seq){\n";
    t += "  if(ws && ws.readyState === 1){\n";
    t += "    var bytes = [27];\n";
    t += "    for(var i=0; i<seq.length; i++) bytes.push(seq.charCodeAt(i));\n";
    t += "    ws.send(new Uint8Array(bytes));\n";
    t += "  }\n";
    t += "}\n";
    t += "function sendCmd(cmd){ if(ws && ws.readyState === 1){ ws.send(cmd + '\\r\\n'); } }\n";
    t += "function clearTerminal(){ term.textContent = ''; }\n";
    t += "function reconnectWs(){ if(ws) ws.close(); initWs(); }\n";
    t += "term.addEventListener('keydown', function(e){\n";
    t += "  if(e.key === 'Backspace'){ e.preventDefault(); sendSpecialKey(8); return; }\n";
    t += "  if(e.key === 'Tab'){ e.preventDefault(); sendSpecialKey(9); return; }\n";
    t += "  if(e.key === 'Enter'){ e.preventDefault(); sendSpecialKey(13); return; }\n";
    t += "  if(e.key === 'ArrowUp'){ e.preventDefault(); sendEscapeSeq('[A'); return; }\n";
    t += "  if(e.key === 'ArrowDown'){ e.preventDefault(); sendEscapeSeq('[B'); return; }\n";
    t += "  if(e.key === 'ArrowLeft'){ e.preventDefault(); sendEscapeSeq('[D'); return; }\n";
    t += "  if(e.key === 'ArrowRight'){ e.preventDefault(); sendEscapeSeq('[C'); return; }\n";
    t += "  if(e.ctrlKey && e.key === 'c'){ e.preventDefault(); sendSpecialKey(3); return; }\n";
    t += "  if(e.ctrlKey && e.key === 'z'){ e.preventDefault(); sendSpecialKey(26); return; }\n";
    t += "  if(e.ctrlKey && e.key === 'd'){ e.preventDefault(); sendSpecialKey(4); return; }\n";
    t += "  if(e.key.length === 1 && !e.ctrlKey && !e.altKey && !e.metaKey){\n";
    t += "    e.preventDefault();\n";
    t += "    if(ws && ws.readyState === 1) ws.send(e.key);\n";
    t += "  }\n";
    t += "});\n";
    t += "renderQuickCmds(document.getElementById('platform_select').value);\n";
    t += "initWs();\n";
    t += "</script>\n";

    _server.sendContent(t);
    streamFooter();
}

// =============================================================================
// Settings Page (Matching Monitor & Emulator Design)
// =============================================================================
void WebPortal::handleSettings() {
    if (!checkAuth()) return;

    streamHeader("settings", "Settings");

    String devHost = _prefs.getString(NVS_KEY_HOSTNAME, DEFAULT_HOSTNAME);
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

    // --- 1. SYSTEM ACTIONS & 2. NETWORK IDENTITY CARDS ---
    String c1 = "";
    c1.reserve(2500);
    c1 += "<div class=\"table-card\">\n";
    c1 += "  <h3 style=\"color:var(--navy);\">&#9881; System Actions</h3>\n";
    c1 += "  <div style=\"display:flex;gap:12px;flex-wrap:wrap;\">\n";
    c1 += "    <a href=\"/wifi\" class=\"btn btn-outline\">&#128246; Reconfigure WiFi</a>\n";
    c1 += "    <a href=\"/update\" class=\"btn btn-outline\">&#128640; Firmware Update (OTA)</a>\n";
    c1 += "    <button type=\"button\" onclick=\"restartDevice()\" class=\"btn btn-outline\">&#128260; Restart Device</button>\n";
    c1 += "    <button type=\"button\" onclick=\"forgetWifi()\" class=\"btn btn-outline\">&#9888; Forget WiFi</button>\n";
    c1 += "    <button type=\"button\" onclick=\"factoryReset()\" class=\"btn btn-outline\" style=\"color:var(--danger);border-color:var(--danger);\">&#9888; Factory Reset</button>\n";
    c1 += "  </div>\n";
    c1 += "</div>\n";

    c1 += "<form id=\"settings_form\" onsubmit=\"saveSettings(event)\">\n";

    c1 += "  <div class=\"table-card\">\n";
    c1 += "    <h3 style=\"color:var(--navy);\">&#127760; Network &amp; Device Identity</h3>\n";
    c1 += "    <div style=\"margin-bottom:16px;max-width:440px;\">\n";
    c1 += "      <label style=\"font-size:0.82rem;font-weight:600;display:block;margin-bottom:4px;color:var(--muted);\">Device Hostname (mDNS / OTA / DHCP):</label>\n";
    c1 += "      <input type=\"text\" name=\"dev_host\" id=\"dev_host\" value=\"" + devHost + "\" placeholder=\"esp-oobm\" maxlength=\"32\" class=\"form-control\" style=\"font-family:monospace;\">\n";
    c1 += "      <small style=\"color:var(--muted);display:block;margin-top:4px;\">Local access URL: <code>http://" + devHost + ".local/</code></small>\n";
    c1 += "    </div>\n";
    c1 += "    <label style=\"display:flex;align-items:center;gap:8px;font-weight:600;font-size:0.9rem;margin-bottom:12px;\">\n";
    c1 += "      <input type=\"checkbox\" name=\"ip_static\" id=\"ip_static\" value=\"1\" " + String(staticEn ? "checked " : "") + "onchange=\"toggleStaticIp(this.checked)\"> Use Static IP Configuration (instead of DHCP)\n";
    c1 += "    </label>\n";
    c1 += "    <div id=\"static_ip_fields\" style=\"" + String(staticEn ? "" : "display:none;") + "\" class=\"form-grid\">\n";
    c1 += "      <div class=\"form-group\"><label>Static IP Address:</label><input type=\"text\" name=\"ip_addr\" value=\"" + staticIp + "\" placeholder=\"192.168.1.150\" class=\"form-control\"></div>\n";
    c1 += "      <div class=\"form-group\"><label>Subnet Mask:</label><input type=\"text\" name=\"ip_mask\" value=\"" + staticMask + "\" placeholder=\"255.255.255.0\" class=\"form-control\"></div>\n";
    c1 += "      <div class=\"form-group\"><label>Default Gateway:</label><input type=\"text\" name=\"ip_gw\" value=\"" + staticGw + "\" placeholder=\"192.168.1.1\" class=\"form-control\"></div>\n";
    c1 += "      <div class=\"form-group\"><label>Primary DNS Server:</label><input type=\"text\" name=\"ip_dns\" value=\"" + staticDns + "\" placeholder=\"1.1.1.1\" class=\"form-control\"></div>\n";
    c1 += "    </div>\n";
    c1 += "    <small style=\"color:var(--muted);display:block;margin-top:8px;\">When unchecked, device dynamically receives network parameters via DHCP from your router.</small>\n";
    c1 += "  </div>\n";
    _server.sendContent(c1);

    // --- 3. TIME SYNCHRONIZATION & TIMEZONE CARD ---
    String c2 = "";
    c2.reserve(3500);
    c2 += "  <div class=\"table-card\">\n";
    c2 += "    <div style=\"display:flex;justify-content:space-between;align-items:center;margin-bottom:14px;flex-wrap:wrap;gap:10px;\">\n";
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

    for (size_t i = 0; i < TIMEZONE_COUNT; ++i) {
        bool sel = (savedCity == TIMEZONE_LIST[i].city);
        c2 += "<option value=\"";
        c2 += String(i);
        c2 += "\"";
        if (sel) c2 += " selected";
        c2 += ">";
        c2 += TIMEZONE_LIST[i].city;
        c2 += "</option>\n";
    }

    c2 += "        </select>\n";
    c2 += "      </div>\n";
    c2 += "      <div class=\"form-group\"><label>Time Format:</label>\n";
    c2 += "        <select name=\"time_fmt\" class=\"form-control\">\n";
    c2 += "          <option value=\"24\" " + String(timeFormat24h ? "selected" : "") + ">24 Hours (e.g. 20:15:00)</option>\n";
    c2 += "          <option value=\"12\" " + String(!timeFormat24h ? "selected" : "") + ">12 Hours (e.g. 8:15:00 PM)</option>\n";
    c2 += "        </select>\n";
    c2 += "      </div>\n";
    c2 += "    </div>\n";
    c2 += "    <small style=\"color:var(--muted);display:block;margin-top:8px;\">Device time adjusts automatically for Daylight Saving Time (DST). Background daemon resynchronizes clock every 1 hour to correct hardware timer drift.</small>\n";
    c2 += "  </div>\n";
    _server.sendContent(c2);

    // --- 4. SERIAL PORT & BRIDGE CARD ---
    String c3 = "";
    c3.reserve(2500);
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
    _server.sendContent(c3);

    // --- 5. TELNET SERVICE & 6. SECURITY CARDS ---
    String c4 = "";
    c4.reserve(2500);
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
    c4 += "      <div class=\"form-group\"><label>Telnet Password:</label><input type=\"password\" name=\"tel_pass\" placeholder=\"Leave blank to keep\" class=\"form-control\"></div>\n";
    c4 += "    </div>\n";
    c4 += "  </div>\n";

    // --- 6. SECURITY CARD ---
    c4 += "  <div class=\"table-card\">\n";
    c4 += "    <h3 style=\"color:var(--navy);\">&#128274; Web &amp; API Security</h3>\n";
    c4 += "    <label style=\"display:flex;align-items:center;gap:8px;font-weight:600;font-size:0.9rem;margin-bottom:12px;\">\n";
    c4 += "      <input type=\"checkbox\" name=\"auth_en\" value=\"1\" " + String(_authRequired ? "checked " : "") + "> Enable HTTP Basic Authentication for Web UI\n";
    c4 += "    </label>\n";
    c4 += "    <div class=\"form-grid\">\n";
    c4 += "      <div class=\"form-group\"><label>Admin Username:</label><input type=\"text\" name=\"auth_usr\" value=\"" + String(_authUser) + "\" class=\"form-control\"></div>\n";
    c4 += "      <div class=\"form-group\"><label>Admin Password:</label><input type=\"password\" name=\"auth_pwd\" placeholder=\"Leave blank to keep\" class=\"form-control\"></div>\n";
    c4 += "    </div>\n";
    c4 += "  </div>\n";

    c4 += "  <div style=\"margin-bottom:30px;\">\n";
    c4 += "    <button type=\"submit\" class=\"btn btn-primary\" style=\"padding:10px 24px;\">Save All Settings</button>\n";
    c4 += "  </div>\n";
    c4 += "</form>\n";

    c4 += "<script>\n";
    c4 += "function toggleStaticIp(chk){ document.getElementById('static_ip_fields').style.display = chk ? 'grid' : 'none'; }\n";
    c4 += "function syncNtpNow(){\n";
    c4 += "  fetch('/api/ntp/sync', { method: 'POST' }).then(r=>r.json()).then(res=>{\n";
    c4 += "    showToast('NTP sync triggered!');\n";
    c4 += "    setTimeout(()=>location.reload(), 1500);\n";
    c4 += "  }).catch(()=>showToast('Sync request failed', true));\n";
    c4 += "}\n";
    c4 += "function restartDevice(){\n";
    c4 += "  if(confirm('Restart ESP32 device?')){\n";
    c4 += "    fetch('/api/restart', { method: 'POST' }).then(()=>{\n";
    c4 += "      showToast('Device is restarting... Please wait 10s.');\n";
    c4 += "      setTimeout(()=>location.reload(), 10000);\n";
    c4 += "    });\n";
    c4 += "  }\n";
    c4 += "}\n";
    c4 += "function forgetWifi(){\n";
    c4 += "  if(confirm('Forget WiFi settings? Device will disconnect and start in AP mode.')){\n";
    c4 += "    location.href = '/reset_wifi';\n";
    c4 += "  }\n";
    c4 += "}\n";
    c4 += "function factoryReset(){\n";
    c4 += "  if(confirm('WARNING: Factory Reset will erase ALL configuration, passwords, and WiFi settings! Continue?')){\n";
    c4 += "    fetch('/api/factory_reset', { method: 'POST' }).then(()=>{\n";
    c4 += "      showToast('Factory reset complete. Restarting in AP mode...');\n";
    c4 += "      setTimeout(()=>location.href='/', 8000);\n";
    c4 += "    }).catch(()=>showToast('Reset failed', true));\n";
    c4 += "  }\n";
    c4 += "}\n";
    c4 += "function saveSettings(e){\n";
    c4 += "  e.preventDefault();\n";
    c4 += "  var fd = new FormData(document.getElementById('settings_form'));\n";
    c4 += "  fetch('/api/settings/save', { method: 'POST', body: fd }).then(r=>r.json()).then(res=>{\n";
    c4 += "    if(res.success){ showToast('Settings saved successfully!'); }\n";
    c4 += "    else { showToast('Error saving settings', true); }\n";
    c4 += "  }).catch(()=>showToast('Save failed', true));\n";
    c4 += "}\n";
    c4 += "</script>\n";
    _server.sendContent(c4);

    streamFooter();
}

// =============================================================================
// Dedicated Reconfigure WiFi Page (/wifi)
// =============================================================================
void WebPortal::handleWifiPage() {
    if (!checkAuth()) return;

    streamHeader("settings", "Reconfigure WiFi");

    String staSsid = _prefs.getString(NVS_KEY_WIFI_SSID, "");
    String apSsid = _prefs.getString(NVS_KEY_AP_SSID, "");
    if (apSsid.length() == 0) {
        String mac = WiFi.macAddress();
        mac.replace(":", "");
        apSsid = String(AP_SSID_PREFIX) + mac.substring(mac.length() - 6);
    }
    uint8_t apChan = _prefs.getUChar(NVS_KEY_AP_CHAN, AP_DEFAULT_CHANNEL);
    bool apHidden = _prefs.getBool(NVS_KEY_AP_HIDDEN, false);
    bool apCaptive = _prefs.getBool(NVS_KEY_AP_CAPTIVE, true);
    bool mndpEn = _prefs.getBool(NVS_KEY_MNDP_EN, true);

    String w = "";
    w.reserve(4000);
    w += "<div style=\"margin-bottom:16px;\">\n";
    w += "  <h2 style=\"margin:0;font-size:1.25rem;\">&#128246; Reconfigure Wi-Fi Network</h2>\n";
    w += "</div>\n";
    w += "<form id=\"wifi_form\" onsubmit=\"saveWifi(event)\">\n";
    w += "  <div class=\"table-card\">\n";
    w += "    <h3 style=\"color:var(--navy);\">1. Station Mode (Connect to Existing Wi-Fi)</h3>\n";
    w += "    <div class=\"form-grid\">\n";
    w += "      <div class=\"form-group\"><label>Available Networks (Scanned):</label>\n";
    w += "        <select id=\"scanned_ssid\" class=\"form-control\" onchange=\"selectSsid(this.value)\">\n";
    w += "          <option value=\"\">-- Scanning nearby networks... --</option>\n";
    w += "        </select>\n";
    w += "      </div>\n";
    w += "      <div class=\"form-group\"><label>Network SSID:</label><input type=\"text\" name=\"sta_ssid\" id=\"sta_ssid\" value=\"" + staSsid + "\" placeholder=\"Enter Wi-Fi SSID\" class=\"form-control\"></div>\n";
    w += "      <div class=\"form-group\"><label>Wi-Fi Password:</label><input type=\"password\" name=\"sta_pass\" id=\"sta_pass\" placeholder=\"WPA/WPA2 Password\" class=\"form-control\"></div>\n";
    w += "    </div>\n";
    w += "  </div>\n";

    w += "  <div class=\"table-card\">\n";
    w += "    <h3 style=\"color:var(--navy);\">2. Access Point (AP) Settings</h3>\n";
    w += "    <div class=\"form-grid\">\n";
    w += "      <div class=\"form-group\"><label>AP SSID:</label><input type=\"text\" name=\"ap_ssid\" value=\"" + apSsid + "\" placeholder=\"ESP-OOBM-XXXXXX\" class=\"form-control\"></div>\n";
    w += "      <div class=\"form-group\"><label>AP Password (Empty = Open):</label><input type=\"password\" name=\"ap_pass\" placeholder=\"Optional AP Password\" class=\"form-control\"></div>\n";
    w += "      <div class=\"form-group\"><label>AP Channel:</label>\n";
    w += "        <select name=\"ap_chan\" class=\"form-control\">\n";
    w += "          <option value=\"1\" " + String(apChan == 1 ? "selected" : "") + ">Channel 1</option>\n";
    w += "          <option value=\"6\" " + String(apChan == 6 ? "selected" : "") + ">Channel 6</option>\n";
    w += "          <option value=\"11\" " + String(apChan == 11 ? "selected" : "") + ">Channel 11</option>\n";
    w += "        </select>\n";
    w += "      </div>\n";
    w += "    </div>\n";
    w += "    <div style=\"margin-top:14px;display:flex;gap:20px;flex-wrap:wrap;\">\n";
    w += "      <label class=\"switch-label\"><input type=\"checkbox\" name=\"ap_hidden\" value=\"1\" " + String(apHidden ? "checked " : "") + "> Hide AP SSID</label>\n";
    w += "      <label class=\"switch-label\"><input type=\"checkbox\" name=\"ap_captive\" value=\"1\" " + String(apCaptive ? "checked " : "") + "> Captive Portal Redirection</label>\n";
    w += "      <label class=\"switch-label\"><input type=\"checkbox\" name=\"mndp_en\" value=\"1\" " + String(mndpEn ? "checked " : "") + "> MNDP Neighbor Discovery (UDP 5678)</label>\n";
    w += "    </div>\n";
    w += "  </div>\n";

    w += "  <div style=\"margin-bottom:30px;\">\n";
    w += "    <button type=\"submit\" class=\"btn btn-primary\" style=\"padding:10px 24px;\">Connect &amp; Save Wi-Fi</button>\n";
    w += "  </div>\n";
    w += "</form>\n";

    w += "<script>\n";
    w += "function selectSsid(v){ if(v) document.getElementById('sta_ssid').value = v; }\n";
    w += "function loadScan(){\n";
    w += "  fetch('/api/scan').then(r=>r.json()).then(d=>{\n";
    w += "    var s = document.getElementById('scanned_ssid');\n";
    w += "    s.innerHTML = '<option value=\"\">-- Select a network (' + d.length + ' found) --</option>';\n";
    w += "    d.forEach(function(net){\n";
    w += "      var opt = document.createElement('option');\n";
    w += "      opt.value = net.ssid;\n";
    w += "      opt.innerText = net.ssid + ' (' + net.rssi + ' dBm' + (net.secure ? ' \\u{1F512}' : '') + ')';\n";
    w += "      s.appendChild(opt);\n";
    w += "    });\n";
    w += "  }).catch(()=>{});\n";
    w += "}\n";
    w += "function saveWifi(e){\n";
    w += "  e.preventDefault();\n";
    w += "  var fd = new FormData(document.getElementById('wifi_form'));\n";
    w += "  fetch('/api/wifi/save', { method: 'POST', body: fd }).then(r=>r.json()).then(res=>{\n";
    w += "    if(res.success){\n";
    w += "      showToast('Wi-Fi settings saved. Connecting...');\n";
    w += "      setTimeout(()=>location.href='/settings', 3000);\n";
    w += "    } else { showToast('Error saving Wi-Fi', true); }\n";
    w += "  }).catch(()=>showToast('Request failed', true));\n";
    w += "}\n";
    w += "loadScan();\n";
    w += "</script>\n";

    _server.sendContent(w);
    streamFooter();
}

// =============================================================================
// Dedicated Firmware Update (OTA) Page (/update)
// =============================================================================
void WebPortal::handleUpdatePage() {
    if (!checkAuth()) return;

    streamHeader("settings", "Firmware Update");

    _server.sendContent_P(PSTR(
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

    streamFooter();
}

void WebPortal::handleMetrics() {
    if (!checkAuth()) return;

    SystemStatsData stats;
    SystemStats::update(stats);
    PrometheusExporter::generateMetrics(_server, stats);
}

void WebPortal::handleSyncNtp() {
    if (!checkAuth()) return;
    triggerNtpSync();
    if (_server.hasArg("redirect")) {
        _server.sendHeader("Location", _server.arg("redirect"));
        _server.send(302, "text/plain", "");
    } else {
        _server.send(200, "application/json", "{\"success\":true}");
    }
}

void WebPortal::handleResetWifi() {
    if (!checkAuth()) return;
    _prefs.remove(NVS_KEY_WIFI_SSID);
    _prefs.remove(NVS_KEY_WIFI_PASS);
    _server.send(200, "text/html", "<!DOCTYPE html><html><head><meta charset='utf-8'><style>body{font-family:sans-serif;text-align:center;padding:50px;background:#0b1120;color:#e2e8f0;}</style></head><body><h3 style='color:#ef4444;'>WiFi credentials erased.</h3><p>Restarting into Standalone AP mode...</p></body></html>");
    delay(1000);
    ESP.restart();
}

// =============================================================================
// AJAX Endpoints
// =============================================================================
void WebPortal::handleApiStatus() {
    if (!checkAuth()) return;

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
        struct tm timeinfo;
        localtime_r(&stats.currentTime, &timeinfo);
        if (is24h) {
            strftime(timeBuf, sizeof(timeBuf), "%Y-%m-%d %H:%M", &timeinfo);
        } else {
            strftime(timeBuf, sizeof(timeBuf), "%Y-%m-%d %I:%M %p", &timeinfo);
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

    char json[1200];
    snprintf(json, sizeof(json),
             "{\"uptime_sec\":%u,\"uptime_str\":\"%s\",\"cpu_freq\":%u,\"free_heap\":%u,\"free_heap_str\":\"%s\","
             "\"min_free_heap\":%u,\"min_heap_str\":\"%s\",\"heap_frag\":%u,\"baud\":%u,\"framing\":\"%s\","
             "\"rx_bytes\":%u,\"rx_bytes_str\":\"%s\",\"tx_bytes\":%u,\"tx_bytes_str\":\"%s\",\"rx_overflow\":%u,"
             "\"active_ws\":%u,\"active_telnet\":%u,\"net_mode\":\"%s\",\"ip\":\"%s\",\"ssid\":\"%s\",\"mac\":\"%s\","
             "\"rssi\":%d,\"ap_clients\":%u,\"ntp_synced\":%s,\"time_str\":\"%s\",\"last_ntp_str\":\"%s\","
             "\"tz_city\":\"%s\",\"ntp_server\":\"%s\"}",
             stats.uptimeSeconds, uptimeBuf, stats.cpuFreqMhz, stats.freeHeapBytes, heapBuf,
             stats.minFreeHeapBytes, minHeapBuf, stats.heapFragPercent, stats.baudRate, framing,
             stats.serialRxBytes, rxBuf, stats.serialTxBytes, txBuf, stats.serialRxOverflow,
             stats.activeWsClients, stats.activeTelnetClients,
             stats.wifiStaConnected ? "Station" : (stats.wifiApActive ? "Access Point" : "Disconnected"),
             stats.ipAddress, stats.ssid, stats.macAddress, stats.wifiRssi, stats.wifiApClients,
             stats.ntpSynced ? "true" : "false", timeBuf, lastNtpBuf,
             _prefs.getString(NVS_KEY_NTP_TZ_CITY, DEFAULT_TZ_CITY).c_str(),
             _prefs.getString(NVS_KEY_NTP_SERVER, NTP_DEFAULT_SERVER).c_str());

    _server.send(200, "application/json", json);
}

void WebPortal::handleApiScan() {
    if (!checkAuth()) return;

    int n = WiFi.scanNetworks(false, true);
    String json = "[";
    for (int i = 0; i < n; ++i) {
        if (i > 0) json += ",";
        json += "{\"ssid\":\"" + WiFi.SSID(i) + "\",\"rssi\":" + String(WiFi.RSSI(i)) + ",\"secure\":" + String(WiFi.encryptionType(i) != WIFI_AUTH_OPEN ? "true" : "false") + "}";
    }
    json += "]";
    _server.send(200, "application/json", json);
}

void WebPortal::handleApiLogs() {
    if (!checkAuth()) return;

    String json = "[";
    size_t count = logger.getCount();
    for (size_t i = 0; i < count; i++) {
        const LogEntry &e = logger.getEntry(i);
        if (i > 0) json += ",";
        
        char timeBuf[32];
        if (e.timestamp > 1577836800) {
            time_t t = (time_t)e.timestamp;
            struct tm ti;
            localtime_r(&t, &ti);
            strftime(timeBuf, sizeof(timeBuf), "%H:%M:%S", &ti);
        } else {
            snprintf(timeBuf, sizeof(timeBuf), "+%us", e.timestamp);
        }

        json += "{\"time\":\"" + String(timeBuf) + "\",\"lvl\":" + String((int)e.level) + ",\"msg\":\"" + String(e.msg) + "\"}";
    }
    json += "]";
    _server.send(200, "application/json", json);
}

void WebPortal::handleApiSaveSettings() {
    if (!checkAuth()) return;

    // Device Hostname & Static IP
    if (_server.hasArg("dev_host")) {
        String h = _server.arg("dev_host");
        if (h.length() > 0) {
            _prefs.putString(NVS_KEY_HOSTNAME, h);
            mndpDiscovery.setHostname(h.c_str());
        }
    }

    bool isStatic = _server.hasArg("ip_static");
    _prefs.putBool(NVS_KEY_WIFI_DHCP, !isStatic);
    if (_server.hasArg("ip_addr")) _prefs.putString(NVS_KEY_WIFI_IP, _server.arg("ip_addr"));
    if (_server.hasArg("ip_mask")) _prefs.putString(NVS_KEY_WIFI_SN, _server.arg("ip_mask"));
    if (_server.hasArg("ip_gw"))   _prefs.putString(NVS_KEY_WIFI_GW, _server.arg("ip_gw"));
    if (_server.hasArg("ip_dns"))  _prefs.putString(NVS_KEY_WIFI_DNS, _server.arg("ip_dns"));

    // NTP Time settings
    bool ntpEn = _server.hasArg("ntp_en");
    _prefs.putBool(NVS_KEY_NTP_ENABLED, ntpEn);
    if (_server.hasArg("ntp_srv")) _prefs.putString(NVS_KEY_NTP_SERVER, _server.arg("ntp_srv"));

    if (_server.hasArg("tz_idx")) {
        int idx = _server.arg("tz_idx").toInt();
        if (idx >= 0 && idx < (int)TIMEZONE_COUNT) {
            _prefs.putString(NVS_KEY_NTP_TZ_POSIX, TIMEZONE_LIST[idx].posix);
            _prefs.putString(NVS_KEY_NTP_TZ_CITY, TIMEZONE_LIST[idx].city);
        }
    }
    if (_server.hasArg("time_fmt")) {
        _prefs.putBool(NVS_KEY_TIME_FORMAT_24H, _server.arg("time_fmt") == "24");
    }

    triggerNtpSync();

    // Serial settings
    if (_server.hasArg("ser_baud")) {
        uint32_t baud = _server.arg("ser_baud").toInt();
        uint8_t dbits = _server.hasArg("ser_dbits") ? (uint8_t)_server.arg("ser_dbits").toInt() : 8;
        uint8_t parity = _server.hasArg("ser_parity") ? (uint8_t)_server.arg("ser_parity").toInt() : 0;
        uint8_t sbits = _server.hasArg("ser_sbits") ? (uint8_t)_server.arg("ser_sbits").toInt() : 1;
        bool echo = _server.hasArg("ser_echo");
        bool banner = _server.hasArg("ser_banner");

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

    // Telnet settings
    if (_server.hasArg("tel_port")) {
        bool telEn = _server.hasArg("tel_en");
        uint16_t telPort = (uint16_t)_server.arg("tel_port").toInt();
        bool telAuth = _server.hasArg("tel_auth");
        String telPass = _server.arg("tel_pass");

        _prefs.putBool(NVS_KEY_TELNET_EN, telEn);
        _prefs.putUShort(NVS_KEY_TELNET_PORT, telPort);
        _prefs.putBool(NVS_KEY_TELNET_AUTH, telAuth);
        if (telPass.length() > 0) _prefs.putString(NVS_KEY_TELNET_PASS, telPass);

        telnetServer.setEnabled(telEn);
        telnetServer.setPort(telPort);
        telnetServer.setAuth(telAuth, telPass.length() > 0 ? telPass.c_str() : nullptr);
    }

    // Security settings
    if (_server.hasArg("auth_usr")) {
        bool authEn = _server.hasArg("auth_en");
        String u = _server.arg("auth_usr");
        String p = _server.arg("auth_pwd");

        _prefs.putBool(NVS_KEY_AUTH_EN, authEn);
        if (u.length() > 0) _prefs.putString(NVS_KEY_AUTH_USER, u);
        if (p.length() > 0) _prefs.putString(NVS_KEY_AUTH_PASS, p);

        setAuthCredentials(authEn, u.c_str(), p.length() > 0 ? p.c_str() : _authPass);
        webTerminal.setAuth(authEn, u.c_str(), p.length() > 0 ? p.c_str() : _authPass);
    }

    logger.logInfo("Settings saved successfully.");
    _server.send(200, "application/json", "{\"success\":true}");
}

void WebPortal::handleApiSaveWifi() {
    if (!checkAuth()) return;

    if (_server.hasArg("sta_ssid")) {
        _prefs.putString(NVS_KEY_WIFI_SSID, _server.arg("sta_ssid"));
        _prefs.putString(NVS_KEY_WIFI_PASS, _server.arg("sta_pass"));
    }
    if (_server.hasArg("ap_ssid")) {
        _prefs.putString(NVS_KEY_AP_SSID, _server.arg("ap_ssid"));
        _prefs.putString(NVS_KEY_AP_PASS, _server.arg("ap_pass"));
        _prefs.putUChar(NVS_KEY_AP_CHAN, (uint8_t)_server.arg("ap_chan").toInt());
        _prefs.putBool(NVS_KEY_AP_HIDDEN, _server.hasArg("ap_hidden"));
        _prefs.putBool(NVS_KEY_AP_CAPTIVE, _server.hasArg("ap_captive"));
        _prefs.putBool(NVS_KEY_MNDP_EN, _server.hasArg("mndp_en"));
    }

    logger.logInfo("Wi-Fi configuration updated.");
    _server.send(200, "application/json", "{\"success\":true}");
}

void WebPortal::handleApiRestart() {
    if (!checkAuth()) return;
    logger.logWarn("Reboot triggered via Web API.");
    _server.send(200, "application/json", "{\"rebooting\":true}");
    delay(500);
    ESP.restart();
}

void WebPortal::handleApiFactoryReset() {
    if (!checkAuth()) return;
    logger.logWarn("Factory Reset triggered via Web API.");
    _prefs.clear();
    _server.send(200, "application/json", "{\"reset\":true}");
    delay(500);
    ESP.restart();
}

void WebPortal::handleApiPlatform() {
    if (!checkAuth()) return;
    if (_server.hasArg("p")) {
        _prefs.putString(NVS_KEY_CLI_PLATFORM, _server.arg("p"));
    }
    _server.send(200, "application/json", "{\"success\":true}");
}


// =============================================================================
// OTA Firmware Upload Handlers
// =============================================================================
void WebPortal::handleUpdateUpload() {
    HTTPUpload &upload = _server.upload();
    if (upload.status == UPLOAD_FILE_START) {
        logger.logInfo("OTA Upload started: %s", upload.filename.c_str());
        if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
            Update.printError(Serial);
        }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
        if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
            Update.printError(Serial);
        }
    } else if (upload.status == UPLOAD_FILE_END) {
        if (Update.end(true)) {
            logger.logInfo("OTA Upload successful (%u bytes). Rebooting...", upload.totalSize);
        } else {
            Update.printError(Serial);
            logger.logError("OTA Update verification failed.");
        }
    }
}

void WebPortal::handleUpdateFinish() {
    if (!checkAuth()) return;
    if (Update.hasError()) {
        _server.send(500, "text/plain", "Update Failed");
    } else {
        _server.send(200, "text/plain", "OK");
        delay(500);
        ESP.restart();
    }
}

// =============================================================================
// Captive Portal & Not Found Handlers
// =============================================================================
void WebPortal::handleCaptivePortal() {
    _server.sendHeader("Location", "http://" + _server.client().localIP().toString() + "/");
    _server.send(302, "text/plain", "");
}

void WebPortal::handleNotFound() {
    if (_isApMode && _captiveEnabled) {
        handleCaptivePortal();
    } else {
        _server.send(404, "text/plain", "404: Not Found");
    }
}

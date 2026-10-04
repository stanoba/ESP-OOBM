# REST & WebSocket API Specification

The **ESP-OOBM** firmware exposes a complete JSON REST API and binary WebSocket streaming interface for automated integration with scripts, monitoring tools, and custom network dashboards.

---

## Authentication & Security

All API endpoints enforce authentication when enabled in **Settings &rarr; Web & API Security** (`auth_enabled = true`).

Two authentication mechanisms are supported:

1. **Form Login & Session Cookie (WebUI & Browser clients)**:
   - Endpoint: `POST /api/login`
   - Sets cookie: `Set-Cookie: oobm_session=<32-char-token>; Path=/; Max-Age=86400; SameSite=Lax`
2. **HTTP Basic Authentication (Scripts & Prometheus scrapers)**:
   - Header: `Authorization: Basic <base64(user:pass)>`

---

## REST Endpoints Overview

| Method | Path | Auth | Description |
| :--- | :--- | :---: | :--- |
| `POST` | `/api/login` | No | Authenticate credentials and receive session cookie |
| `GET` | `/api/status` | Yes | Real-time system telemetry and peripheral statistics |
| `GET` | `/api/logs` | Yes | Rolling event log buffer (up to 32 entries) |
| `GET` | `/api/scan` | Yes | Asynchronous Wi-Fi access point scan results |
| `POST` | `/api/settings/save` | Yes | Persist system, serial, security, and timezone settings |
| `POST` | `/api/wifi/save` | Yes | Update Wi-Fi Station / AP credentials and trigger auto-restart |
| `POST` | `/api/platform` | Yes | Persist selected CLI command platform |
| `POST` | `/api/ntp/sync` | Yes | Force immediate NTP time synchronization |
| `POST` | `/api/restart` | Yes | Gracefully reboot microcontroller |
| `POST` | `/api/factory_reset` | Yes | Wipe NVS flash and restore default factory settings |
| `GET` | `/metrics` | Yes | Prometheus OpenMetrics exporter (see [`prometheus.md`](prometheus.md)) |

---

## Detailed Endpoint Documentation

### 1. `POST /api/login`

Authenticates credentials and establishes a session.

#### Request (Form-encoded / Multipart):
```http
POST /api/login HTTP/1.1
Host: 192.168.1.150
Content-Type: application/x-www-form-urlencoded

usr=admin&pwd=oobmadm123
```

#### Response (Success):
```json
{
  "success": true,
  "token": "fcd5318d36c076c4a68f6bd79365d361"
}
```

#### Response (Invalid Credentials):
```json
{
  "success": false,
  "error": "Invalid username or password."
}
```

---

### 2. `GET /api/status`

Returns live telemetry and hardware diagnostic counters.

#### Example Response:
```json
{
  "uptime": 1532,
  "uptime_str": "25m 32s",
  "cpu_freq": 240,
  "free_heap": 151244,
  "free_heap_str": "147.6 KB",
  "min_free_heap": 105620,
  "min_heap_str": "103.1 KB",
  "heap_frag": 41,
  "baud": 115200,
  "framing": "8N1",
  "rx_bytes": 18944,
  "rx_bytes_str": "18.5 KB",
  "tx_bytes": 246,
  "tx_bytes_str": "246 B",
  "rx_overflow": 0,
  "active_ws": 4,
  "active_telnet": 1,
  "net_mode": "STATION",
  "ip": "192.168.1.150",
  "ssid": "Enterprise-LAN",
  "mac": "14:08:08:20:E7:C8",
  "rssi": -54,
  "ap_clients": 0,
  "ntp_synced": true,
  "tz_city": "Europe/Bratislava (UTC+1, CEST)",
  "ntp_server": "pool.ntp.org",
  "time_str": "2026-10-04 23:37",
  "last_ntp_str": "2026-10-04 23:12"
}
```

---

### 3. `GET /api/logs`

Returns the circular system event log buffer.

#### Example Response:
```json
[
  { "time": "23:12:07", "lvl": 0, "msg": "MNDP Neighbor Discovery active (Broadcast UDP :5678)" },
  { "time": "23:12:07", "lvl": 0, "msg": "Web server started on port 80 (Auth: Enabled)" },
  { "time": "23:12:22", "lvl": 0, "msg": "NTP Time synchronized successfully." },
  { "time": "23:22:53", "lvl": 0, "msg": "Telnet client connected from 192.168.1.100 (Awaiting auth)" },
  { "time": "23:23:09", "lvl": 0, "msg": "Telnet client from 192.168.1.100 authenticated successfully." },
  { "time": "23:23:31", "lvl": 0, "msg": "Web user 'admin' logged in successfully." }
]
```

Log levels (`lvl`): `0` = Info (`[INFO]`), `1` = Warning (`[WARN]`), `2` = Error (`[ERROR]`).

---

### 4. `POST /api/settings/save`

Updates device settings and persists them to NVS flash.

#### Form Parameters:
| Parameter | Type | Default | Description |
| :--- | :--- | :--- | :--- |
| `hostname` | string | `esp-oobm` | Device network hostname (mDNS / DHCP) |
| `baud` | integer | `115200` | UART baud rate (300 to 921600 bps) |
| `databits` | integer | `8` | Data bits (`5`, `6`, `7`, `8`) |
| `parity` | integer | `0` | Parity (`0` = None, `1` = Odd, `2` = Even) |
| `stopbits` | integer | `1` | Stop bits (`1`, `2`) |
| `echo` | boolean | `0` | Force local echo on serial bridge |
| `banner` | boolean | `1` | Show greeting banner on connection |
| `telnet_en` | boolean | `1` | Enable Telnet daemon (Port 23) |
| `telnet_port` | integer | `23` | Telnet TCP port |
| `telnet_auth` | boolean | `1` | Require password for Telnet sessions |
| `telnet_pass` | string | — | New Telnet password (blank to keep current) |
| `auth_en` | boolean | `1` | Enable HTTP Basic / Cookie Auth |
| `auth_user` | string | `admin` | Web & API admin username |
| `auth_pass` | string | — | New admin password (blank to keep current) |
| `ntp_en` | boolean | `1` | Enable NTP background daemon |
| `ntp_server` | string | `pool.ntp.org`| NTP server hostname or IP |
| `tz_city` | string | `Europe/Bratislava` | Timezone identifier (see `Timezones.h`) |
| `time_24h` | boolean | `1` | 24-hour time format (`true`) vs 12-hour (`false`) |

---

### 5. `POST /api/wifi/save`

Configures Wi-Fi Station and AP parameters. The microcontroller automatically commits settings to NVS flash and reboots after 800 ms.

#### Form Parameters:
| Parameter | Type | Description |
| :--- | :--- | :--- |
| `ssid` | string | Target Wi-Fi SSID |
| `pass` | string | Wi-Fi WPA2/WPA3 password |
| `ap_ssid` | string | Custom AP SSID prefix (blank = default) |
| `ap_pass` | string | AP WPA2-PSK password (blank = open) |
| `ap_chan` | integer | AP Wi-Fi Channel (`1` to `13`) |
| `hide_ap` | boolean | Hide AP SSID broadcast |
| `captive` | boolean | Enable Captive Portal DNS redirection |
| `mndp_en` | boolean | Enable MikroTik Neighbor Discovery Protocol (MNDP) |

---

## WebSocket Serial Stream Protocol

- **Port**: `81` (`ws://<ip>:81`)
- **Transport**: Binary (`ArrayBuffer`) or UTF-8 Text
- **Authentication**: If `auth_en` is enabled, the client must send an authentication packet immediately upon opening the WebSocket:
  ```text
  AUTH:<username>:<password>
  ```
  Example: `AUTH:admin:oobmadm123`
- **Greeting Banner**: Upon successful authentication, the server emits:
  ```ansi
  \x1b[32m[ESP-OOBM: Connected to Serial Console]\x1b[0m\r\n
  ```
- **Data Flow**: All subsequent binary or text payloads sent over the WebSocket are written directly to UART0 (`GPIO1 TXD`). Incoming UART0 bytes (`GPIO3 RXD`) are framed and pushed to all active WebSocket clients.

---

## Telnet Daemon Protocol (RFC 854)

- **Port**: `23` (`telnet <ip> 23`)
- **Negotiation**: Server negotiates `WILL ECHO` (`0xFF 0xFB 0x01`), `WILL SUPPRESS GO AHEAD` (`0xFF 0xFB 0x03`), and `DO SUPPRESS GO AHEAD` (`0xFF 0xFD 0x03`).
- **Password Authentication**: If `telnet_auth` is enabled, the server presents a password prompt:
  ```text
  =========================================
    ESP-OOBM Console - Authentication
  =========================================
  Password: 
  ```
  Clients have 3 attempts before disconnection. Upon correct authentication, the session is bridged to UART0.

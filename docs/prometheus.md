# Prometheus Exporter & Grafana Integration

The **ESP-OOBM** firmware includes a built-in OpenMetrics / Prometheus exporter accessible at `GET /metrics` on HTTP port `80`.

It exports high-resolution telemetry covering system health, memory allocation, Wi-Fi link quality, serial UART traffic rates, and active management sessions.

---

## Metric Reference Table

| Metric Name | Type | Unit | Description |
| :--- | :---: | :---: | :--- |
| `esp_oobm_uptime_seconds` | Gauge | seconds | Total system uptime since last boot or restart |
| `esp_oobm_heap_free_bytes` | Gauge | bytes | Current free heap memory |
| `esp_oobm_heap_min_free_bytes` | Gauge | bytes | Lowest recorded free heap since boot (watermark) |
| `esp_oobm_heap_fragmentation_percent` | Gauge | % | Heap memory fragmentation index (0–100%) |
| `esp_oobm_wifi_rssi_dbm` | Gauge | dBm | Wi-Fi Station signal strength (-100 to 0 dBm) |
| `esp_oobm_serial_rx_bytes_total` | Counter | bytes | Total bytes received on UART0 from host router |
| `esp_oobm_serial_tx_bytes_total` | Counter | bytes | Total bytes transmitted on UART0 to host router |
| `esp_oobm_serial_rx_overflows_total` | Counter | count | Total UART receive ring buffer overflow events |
| `esp_oobm_websocket_active_clients` | Gauge | count | Number of active WebSocket WebTerminal clients |
| `esp_oobm_telnet_active_clients` | Gauge | count | Number of active RFC 854 Telnet sessions |
| `esp_oobm_ntp_synced` | Gauge | boolean | NTP time synchronization status (`1` = synced, `0` = unsynced) |
| `esp_oobm_info` | Gauge | info | Firmware metadata labels (`version`, `firmware`, `hardware`, `board`, `hostname`, `mac`) |

---

## Sample OpenMetrics Output (`GET /metrics`)

```text
# HELP esp_oobm_uptime_seconds Device uptime in seconds
# TYPE esp_oobm_uptime_seconds gauge
esp_oobm_uptime_seconds 1532

# HELP esp_oobm_heap_free_bytes Free heap memory in bytes
# TYPE esp_oobm_heap_free_bytes gauge
esp_oobm_heap_free_bytes 151244

# HELP esp_oobm_heap_min_free_bytes Lowest free heap memory watermark since boot
# TYPE esp_oobm_heap_min_free_bytes gauge
esp_oobm_heap_min_free_bytes 105620

# HELP esp_oobm_heap_fragmentation_percent Heap fragmentation percentage
# TYPE esp_oobm_heap_fragmentation_percent gauge
esp_oobm_heap_fragmentation_percent 41

# HELP esp_oobm_wifi_rssi_dbm WiFi signal strength in dBm
# TYPE esp_oobm_wifi_rssi_dbm gauge
esp_oobm_wifi_rssi_dbm -54

# HELP esp_oobm_serial_rx_bytes_total Total bytes received from UART bridge
# TYPE esp_oobm_serial_rx_bytes_total counter
esp_oobm_serial_rx_bytes_total 18944

# HELP esp_oobm_serial_tx_bytes_total Total bytes transmitted to UART bridge
# TYPE esp_oobm_serial_tx_bytes_total counter
esp_oobm_serial_tx_bytes_total 246

# HELP esp_oobm_serial_rx_overflows_total Total serial RX ring buffer overflows
# TYPE esp_oobm_serial_rx_overflows_total counter
esp_oobm_serial_rx_overflows_total 0

# HELP esp_oobm_websocket_active_clients Active WebSocket console clients
# TYPE esp_oobm_websocket_active_clients gauge
esp_oobm_websocket_active_clients 4

# HELP esp_oobm_telnet_active_clients Active Telnet console clients
# TYPE esp_oobm_telnet_active_clients gauge
esp_oobm_telnet_active_clients 1

# HELP esp_oobm_ntp_synced NTP time synchronization status (1 = synced, 0 = unsynced)
# TYPE esp_oobm_ntp_synced gauge
esp_oobm_ntp_synced 1

# HELP esp_oobm_info Device and firmware build metadata
# TYPE esp_oobm_info gauge
esp_oobm_info{version="1.0.0",firmware="ESP-OOBM",hardware="ESP32-PICO-D4",board="ESP32 KEY V1.0",hostname="esp-oobm",mac="14:08:08:20:E7:C8"} 1
```

---

## Prometheus Configuration (`prometheus.yml`)

Add the following scrape job to your Prometheus server configuration:

```yaml
scrape_configs:
  - job_name: 'esp-oobm'
    scrape_interval: 15s
    scrape_timeout: 5s
    metrics_path: '/metrics'
    static_configs:
      - targets: ['192.168.1.150:80']
        labels:
          role: 'oobm-console'
          location: 'datacenter-rack-01'
    
    # If HTTP Basic Authentication is enabled in ESP-OOBM Settings:
    basic_auth:
      username: 'admin'
      password: 'oobmadm123'
```

---

## Recommended Grafana PromQL Queries

### 1. Serial Traffic Throughput (Bytes/sec)
```promql
# Incoming Serial Console Traffic Rate (RX)
rate(esp_oobm_serial_rx_bytes_total[1m])

# Outgoing Serial Console Traffic Rate (TX)
rate(esp_oobm_serial_tx_bytes_total[1m])
```

### 2. Memory Health & Free Heap
```promql
# Free Heap in Kilobytes
esp_oobm_heap_free_bytes / 1024

# Minimum Free Heap Watermark in Kilobytes
esp_oobm_heap_min_free_bytes / 1024

# Memory Fragmentation Percentage
esp_oobm_heap_fragmentation_percent
```

### 3. Wi-Fi Link Quality (Signal Strength)
```promql
# Wi-Fi RSSI in dBm
esp_oobm_wifi_rssi_dbm
```

### 4. Active Out-of-Band Management Sessions
```promql
# Total Active Console Sessions (WebSockets + Telnet)
esp_oobm_websocket_active_clients + esp_oobm_telnet_active_clients
```

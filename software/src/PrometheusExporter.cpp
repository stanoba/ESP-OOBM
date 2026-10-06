#include "PrometheusExporter.h"
#include "Config.h"
#include <stdio.h>

extern time_t g_lastNtpSyncTimestamp;

void PrometheusExporter::generateMetrics(ResponseWriter &res, const SystemStatsData &stats) {
    res.setContentType("text/plain; version=0.0.4; charset=utf-8");

    char buf[512];

    // 1. System Info & Hardware Identity
    snprintf(buf, sizeof(buf),
             "# HELP oobm_info Firmware and device hardware build metadata\n"
             "# TYPE oobm_info gauge\n"
             "oobm_info{version=\"%s\",build_date=\"%s\",build_time=\"%s\",chip=\"ESP32-PICO-D4\",mac=\"%s\"} 1\n\n",
             FIRMWARE_VERSION, FIRMWARE_BUILD_DATE, FIRMWARE_BUILD_TIME, stats.macAddress);
    res.sendChunk(buf);

    // 2. CPU, Temperature, Uptime & Reset Diagnostics
    snprintf(buf, sizeof(buf),
             "# HELP oobm_uptime_seconds Total system uptime in seconds\n"
             "# TYPE oobm_uptime_seconds counter\n"
             "oobm_uptime_seconds %u\n\n"
             "# HELP oobm_cpu_freq_mhz Current CPU clock frequency in MHz\n"
             "# TYPE oobm_cpu_freq_mhz gauge\n"
             "oobm_cpu_freq_mhz %u\n\n"
             "# HELP oobm_cpu_load_percent Estimated CPU active load percentage\n"
             "# TYPE oobm_cpu_load_percent gauge\n"
             "oobm_cpu_load_percent %.1f\n\n"
             "# HELP oobm_temperature_celsius ESP32 internal junction temperature in Celsius\n"
             "# TYPE oobm_temperature_celsius gauge\n"
             "oobm_temperature_celsius %.1f\n\n"
             "# HELP oobm_reset_reason Last hardware/software reset reason\n"
             "# TYPE oobm_reset_reason gauge\n"
             "oobm_reset_reason{code=\"%u\",reason=\"%s\"} 1\n\n",
             stats.uptimeSeconds, stats.cpuFreqMhz, stats.cpuLoadPercent, stats.temperatureCelsius,
             stats.resetReasonCode, stats.resetReasonStr);
    res.sendChunk(buf);

    // 3. Heap & Memory Diagnostics
    snprintf(buf, sizeof(buf),
             "# HELP oobm_free_heap_bytes Current available free heap memory in bytes\n"
             "# TYPE oobm_free_heap_bytes gauge\n"
             "oobm_free_heap_bytes %u\n\n"
             "# HELP oobm_min_free_heap_bytes Lowest recorded free heap watermark since boot\n"
             "# TYPE oobm_min_free_heap_bytes gauge\n"
             "oobm_min_free_heap_bytes %u\n\n"
             "# HELP oobm_max_alloc_heap_bytes Largest contiguous free block in heap\n"
             "# TYPE oobm_max_alloc_heap_bytes gauge\n"
             "oobm_max_alloc_heap_bytes %u\n\n",
             stats.freeHeapBytes, stats.minFreeHeapBytes, stats.maxAllocHeapBytes);
    res.sendChunk(buf);

    // Keep this in a separate buffer write: combining all four HELP/TYPE/value
    // groups exceeded the fixed 512-byte buffer and silently truncated the
    // fragmentation metric.
    snprintf(buf, sizeof(buf),
             "# HELP oobm_heap_fragmentation_percent Heap memory fragmentation percentage\n"
             "# TYPE oobm_heap_fragmentation_percent gauge\n"
             "oobm_heap_fragmentation_percent %u\n\n",
             stats.heapFragPercent);
    res.sendChunk(buf);

    // 4. Flash & OTA Partition metrics
    snprintf(buf, sizeof(buf),
             "# HELP oobm_flash_size_bytes Total embedded NOR Flash size in bytes\n"
             "# TYPE oobm_flash_size_bytes gauge\n"
             "oobm_flash_size_bytes %u\n\n"
             "# HELP oobm_sketch_size_bytes Compiled active application binary size in bytes\n"
             "# TYPE oobm_sketch_size_bytes gauge\n"
             "oobm_sketch_size_bytes %u\n\n"
             "# HELP oobm_free_ota_space_bytes Available OTA flash partition capacity in bytes\n"
             "# TYPE oobm_free_ota_space_bytes gauge\n"
             "oobm_free_ota_space_bytes %u\n\n",
             stats.flashSizeBytes, stats.sketchSizeBytes, stats.freeSketchSpaceBytes);
    res.sendChunk(buf);

    // 5. WiFi Network metrics
    snprintf(buf, sizeof(buf),
             "# HELP oobm_wifi_rssi_dbm WiFi received signal strength in dBm\n"
             "# TYPE oobm_wifi_rssi_dbm gauge\n"
             "oobm_wifi_rssi_dbm %d\n\n"
             "# HELP oobm_wifi_connected WiFi station connection state (1 = connected, 0 = disconnected)\n"
             "# TYPE oobm_wifi_connected gauge\n"
             "oobm_wifi_connected %d\n\n"
             "# HELP oobm_wifi_ap_active Access Point state (1 = active, 0 = inactive)\n"
             "# TYPE oobm_wifi_ap_active gauge\n"
             "oobm_wifi_ap_active %d\n\n"
             "# HELP oobm_wifi_ap_clients Number of connected wireless clients to Access Point\n"
             "# TYPE oobm_wifi_ap_clients gauge\n"
             "oobm_wifi_ap_clients %u\n\n",
             stats.wifiRssi, stats.wifiStaConnected ? 1 : 0, stats.wifiApActive ? 1 : 0, stats.wifiApClients);
    res.sendChunk(buf);

    // 6. Serial Bridge Hardware metrics
    snprintf(buf, sizeof(buf),
             "# HELP oobm_serial_baud_rate Active hardware UART baud rate\n"
             "# TYPE oobm_serial_baud_rate gauge\n"
             "oobm_serial_baud_rate %u\n\n"
             "# HELP oobm_serial_rx_bytes_total Total bytes received from hardware UART\n"
             "# TYPE oobm_serial_rx_bytes_total counter\n"
             "oobm_serial_rx_bytes_total %u\n\n"
             "# HELP oobm_serial_tx_bytes_total Total bytes transmitted to hardware UART\n"
             "# TYPE oobm_serial_tx_bytes_total counter\n"
             "oobm_serial_tx_bytes_total %u\n\n"
             "# HELP oobm_serial_rx_overflow_total Total bytes dropped due to RX ring buffer overflow\n"
             "# TYPE oobm_serial_rx_overflow_total counter\n"
             "oobm_serial_rx_overflow_total %u\n\n",
             stats.baudRate, stats.serialRxBytes, stats.serialTxBytes, stats.serialRxOverflow);
    res.sendChunk(buf);

    // 7. Active Network Sessions metrics
    snprintf(buf, sizeof(buf),
             "# HELP oobm_active_ws_clients Currently connected WebSocket terminal sessions\n"
             "# TYPE oobm_active_ws_clients gauge\n"
             "oobm_active_ws_clients %u\n\n"
             "# HELP oobm_active_telnet_clients Currently connected Telnet console sessions\n"
             "# TYPE oobm_active_telnet_clients gauge\n"
             "oobm_active_telnet_clients %u\n\n",
             stats.activeWsClients, stats.activeTelnetClients);
    res.sendChunk(buf);

    // 8. SNTP Time Synchronization metrics
    snprintf(buf, sizeof(buf),
             "# HELP oobm_ntp_synced Whether network time synchronization is active\n"
             "# TYPE oobm_ntp_synced gauge\n"
             "oobm_ntp_synced %d\n\n"
             "# HELP oobm_ntp_last_sync_timestamp Timestamp of last successful SNTP synchronization\n"
             "# TYPE oobm_ntp_last_sync_timestamp gauge\n"
             "oobm_ntp_last_sync_timestamp %ld\n",
             stats.ntpSynced ? 1 : 0, (long)g_lastNtpSyncTimestamp);
    res.sendChunk(buf);

    res.end();
}

#include "PrometheusExporter.h"
#include "Config.h"
#include <stdio.h>

extern time_t g_lastNtpSyncTimestamp;

void PrometheusExporter::generateMetrics(WebServer &server, const SystemStatsData &stats) {
    server.setContentLength(CONTENT_LENGTH_UNKNOWN);
    server.send(200, "text/plain; version=0.0.4; charset=utf-8", "");

    char buf[512];

    // Info metadata
    snprintf(buf, sizeof(buf),
             "# HELP esp_oobm_info Firmware and device build metadata\n"
             "# TYPE esp_oobm_info gauge\n"
             "esp_oobm_info{version=\"%s\",build_date=\"%s\",build_time=\"%s\",chip=\"ESP32-PICO-D4\",mac=\"%s\"} 1\n\n",
             FIRMWARE_VERSION, FIRMWARE_BUILD_DATE, FIRMWARE_BUILD_TIME, stats.macAddress);
    server.sendContent(buf);

    // System uptime & CPU
    snprintf(buf, sizeof(buf),
             "# HELP esp_uptime_seconds Total system uptime in seconds\n"
             "# TYPE esp_uptime_seconds gauge\n"
             "esp_uptime_seconds %u\n\n"
             "# HELP esp_cpu_freq_mhz Current CPU frequency in megahertz\n"
             "# TYPE esp_cpu_freq_mhz gauge\n"
             "esp_cpu_freq_mhz %u\n\n",
             stats.uptimeSeconds, stats.cpuFreqMhz);
    server.sendContent(buf);

    // Heap metrics
    snprintf(buf, sizeof(buf),
             "# HELP esp_free_heap_bytes Current available free heap in bytes\n"
             "# TYPE esp_free_heap_bytes gauge\n"
             "esp_free_heap_bytes %u\n\n"
             "# HELP esp_min_free_heap_bytes Minimum free heap observed since boot in bytes\n"
             "# TYPE esp_min_free_heap_bytes gauge\n"
             "esp_min_free_heap_bytes %u\n\n"
             "# HELP esp_max_alloc_heap_bytes Largest contiguous free block in heap\n"
             "# TYPE esp_max_alloc_heap_bytes gauge\n"
             "esp_max_alloc_heap_bytes %u\n\n"
             "# HELP esp_heap_fragmentation_percent Heap fragmentation percentage\n"
             "# TYPE esp_heap_fragmentation_percent gauge\n"
             "esp_heap_fragmentation_percent %u\n\n",
             stats.freeHeapBytes, stats.minFreeHeapBytes, stats.maxAllocHeapBytes, stats.heapFragPercent);
    server.sendContent(buf);

    // Flash & Sketch metrics
    snprintf(buf, sizeof(buf),
             "# HELP esp_flash_size_bytes Total embedded flash size in bytes\n"
             "# TYPE esp_flash_size_bytes gauge\n"
             "esp_flash_size_bytes %u\n\n"
             "# HELP esp_sketch_size_bytes Compiled sketch size in bytes\n"
             "# TYPE esp_sketch_size_bytes gauge\n"
             "esp_sketch_size_bytes %u\n\n"
             "# HELP esp_free_sketch_space_bytes Available space for OTA updates in bytes\n"
             "# TYPE esp_free_sketch_space_bytes gauge\n"
             "esp_free_sketch_space_bytes %u\n\n",
             stats.flashSizeBytes, stats.sketchSizeBytes, stats.freeSketchSpaceBytes);
    server.sendContent(buf);

    // WiFi metrics
    snprintf(buf, sizeof(buf),
             "# HELP esp_wifi_rssi_dbm WiFi received signal strength in dBm\n"
             "# TYPE esp_wifi_rssi_dbm gauge\n"
             "esp_wifi_rssi_dbm %d\n\n"
             "# HELP esp_wifi_connected WiFi station connection state (1 = connected, 0 = disconnected)\n"
             "# TYPE esp_wifi_connected gauge\n"
             "esp_wifi_connected %d\n\n"
             "# HELP esp_wifi_ap_active Access point state (1 = active, 0 = inactive)\n"
             "# TYPE esp_wifi_ap_active gauge\n"
             "esp_wifi_ap_active %d\n\n"
             "# HELP esp_wifi_ap_clients Number of connected clients to Access Point\n"
             "# TYPE esp_wifi_ap_clients gauge\n"
             "esp_wifi_ap_clients %u\n\n",
             stats.wifiRssi, stats.wifiStaConnected ? 1 : 0, stats.wifiApActive ? 1 : 0, stats.wifiApClients);
    server.sendContent(buf);

    // Serial Bridge metrics
    snprintf(buf, sizeof(buf),
             "# HELP esp_serial_baud_rate Active hardware UART baud rate\n"
             "# TYPE esp_serial_baud_rate gauge\n"
             "esp_serial_baud_rate %u\n\n"
             "# HELP esp_serial_rx_bytes_total Total bytes received from hardware UART\n"
             "# TYPE esp_serial_rx_bytes_total counter\n"
             "esp_serial_rx_bytes_total %u\n\n"
             "# HELP esp_serial_tx_bytes_total Total bytes transmitted to hardware UART\n"
             "# TYPE esp_serial_tx_bytes_total counter\n"
             "esp_serial_tx_bytes_total %u\n\n"
             "# HELP esp_serial_rx_overflow_total Total bytes dropped due to RX ring buffer overflow\n"
             "# TYPE esp_serial_rx_overflow_total counter\n"
             "esp_serial_rx_overflow_total %u\n\n",
             stats.baudRate, stats.serialRxBytes, stats.serialTxBytes, stats.serialRxOverflow);
    server.sendContent(buf);

    // Active Sessions metrics
    snprintf(buf, sizeof(buf),
             "# HELP esp_serial_active_ws_sessions Currently connected WebSocket terminal clients\n"
             "# TYPE esp_serial_active_ws_sessions gauge\n"
             "esp_serial_active_ws_sessions %u\n\n"
             "# HELP esp_serial_active_telnet_sessions Currently connected Telnet clients\n"
             "# TYPE esp_serial_active_telnet_sessions gauge\n"
             "esp_serial_active_telnet_sessions %u\n\n",
             stats.activeWsClients, stats.activeTelnetClients);
    server.sendContent(buf);

    // NTP metrics
    snprintf(buf, sizeof(buf),
             "# HELP esp_ntp_sync_success Whether SNTP time sync is active\n"
             "# TYPE esp_ntp_sync_success gauge\n"
             "esp_ntp_sync_success %d\n\n"
             "# HELP esp_ntp_last_sync_timestamp Timestamp of last successful NTP synchronization\n"
             "# TYPE esp_ntp_last_sync_timestamp gauge\n"
             "esp_ntp_last_sync_timestamp %ld\n",
             stats.ntpSynced ? 1 : 0, (long)g_lastNtpSyncTimestamp);
    server.sendContent(buf);

    server.sendContent(""); // End chunked response
}

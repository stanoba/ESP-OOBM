#pragma once
#include <Arduino.h>
#include <WebServer.h>
#include "SystemStats.h"

class PrometheusExporter {
public:
    static void generateMetrics(WebServer &server, const SystemStatsData &stats);
};

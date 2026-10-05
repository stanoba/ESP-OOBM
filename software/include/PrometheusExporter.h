#pragma once
#include <Arduino.h>
#include "SystemStats.h"
#include "WebPortal.h"

class PrometheusExporter {
public:
    static void generateMetrics(ResponseWriter &res, const SystemStatsData &stats);
};

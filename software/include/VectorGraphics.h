#pragma once
#include <Arduino.h>

// =============================================================================
// ESP-OOBM Original Vector SVG Logo (Horizontal USB Key + 3 Arrows + WiFi Wave)
// Stored in Flash ROM (PROGMEM) - Zero Dynamic RAM Allocation
// =============================================================================
static const char OOBM_LOGO_SVG[] PROGMEM = 
R"rawliteral(<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 340 46" style="height:48px;width:auto;display:block;">
  <defs>
    <linearGradient id="oobmGrad" x1="0%" y1="0%" x2="100%" y2="100%">
      <stop offset="0%" stop-color="#00c0d8"/>
      <stop offset="40%" stop-color="#0284c7"/>
      <stop offset="100%" stop-color="#10b981"/>
    </linearGradient>
    <linearGradient id="oobmGradCyan" x1="0%" y1="0%" x2="100%" y2="0%">
      <stop offset="0%" stop-color="#38bdf8"/>
      <stop offset="100%" stop-color="#00c0d8"/>
    </linearGradient>
  </defs>
  <g transform="translate(1, 0)">
    <!-- USB Plug Tip -->
    <rect x="2" y="16" width="9" height="14" rx="2" fill="none" stroke="url(#oobmGrad)" stroke-width="2.6"/>
    <!-- USB Dongle Body (Longer Horizontal Rounded Rectangle) -->
    <rect x="11" y="8" width="58" height="30" rx="6.5" fill="none" stroke="url(#oobmGrad)" stroke-width="2.8"/>
    <!-- 3 Arrows '>>>' Inside USB Dongle -->
    <path d="M23 16 L31 23 L23 30" fill="none" stroke="url(#oobmGradCyan)" stroke-width="2.8" stroke-linecap="round" stroke-linejoin="round"/>
    <path d="M36 16 L44 23 L36 30" fill="none" stroke="url(#oobmGradCyan)" stroke-width="2.8" stroke-linecap="round" stroke-linejoin="round"/>
    <path d="M49 16 L57 23 L49 30" fill="none" stroke="url(#oobmGradCyan)" stroke-width="2.8" stroke-linecap="round" stroke-linejoin="round"/>
    <!-- WiFi Radiance Arcs on Right -->
    <path d="M75 14 A14 14 0 0 1 75 32" fill="none" stroke="url(#oobmGrad)" stroke-width="2.6" stroke-linecap="round"/>
    <path d="M81 9 A22 22 0 0 1 81 37" fill="none" stroke="url(#oobmGrad)" stroke-width="2.6" stroke-linecap="round"/>
  </g>
  <!-- Wordmark: ESP -OOBM -->
  <text class="brand-title-prefix" x="96" y="27" font-family="system-ui, -apple-system, 'Segoe UI', Roboto, sans-serif" font-weight="800" font-size="21" fill="#0284c7" letter-spacing="1">ESP</text>
  <text class="brand-title-main" x="144" y="27" font-family="system-ui, -apple-system, 'Segoe UI', Roboto, sans-serif" font-weight="900" font-size="21" fill="#0f172a" letter-spacing="1.5"> -OOBM</text>
  <!-- Subtitle: OUT-OF-BAND MANAGEMENT -->
  <text class="brand-sub" x="97" y="40" font-family="system-ui, -apple-system, 'Segoe UI', Roboto, sans-serif" font-weight="700" font-size="9.5" fill="#009aa0" letter-spacing="1.8">OUT-OF-BAND MANAGEMENT</text>
</svg>)rawliteral";

// =============================================================================
// ESP-OOBM Original Vector SVG Favicon
// Stored in Flash ROM (PROGMEM) - Zero Dynamic RAM Allocation
// =============================================================================
static const char OOBM_FAVICON_SVG[] PROGMEM = 
R"rawliteral(<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 48 48">
  <defs>
    <linearGradient id="favOobmGrad" x1="0%" y1="0%" x2="100%" y2="100%">
      <stop offset="0%" stop-color="#0284c7"/>
      <stop offset="50%" stop-color="#00c0d8"/>
      <stop offset="100%" stop-color="#10b981"/>
    </linearGradient>
  </defs>
  <g transform="translate(1, 1)">
    <!-- USB Plug Tip -->
    <rect x="1" y="17" width="5" height="12" rx="1.5" fill="none" stroke="url(#favOobmGrad)" stroke-width="2.2"/>
    <!-- USB Dongle Body -->
    <rect x="6" y="10" width="26" height="26" rx="5" fill="none" stroke="url(#favOobmGrad)" stroke-width="2.5"/>
    <!-- 2 Chevrons Inside -->
    <path d="M12 17 L18 23 L12 29" fill="none" stroke="#38bdf8" stroke-width="2.5" stroke-linecap="round" stroke-linejoin="round"/>
    <path d="M20 17 L26 23 L20 29" fill="none" stroke="#38bdf8" stroke-width="2.5" stroke-linecap="round" stroke-linejoin="round"/>
    <!-- WiFi Radiance Arcs on Right -->
    <path d="M35 15 A11 11 0 0 1 35 31" fill="none" stroke="url(#favOobmGrad)" stroke-width="2.4" stroke-linecap="round"/>
    <path d="M40 10 A18 18 0 0 1 40 36" fill="none" stroke="url(#favOobmGrad)" stroke-width="2.4" stroke-linecap="round"/>
  </g>
</svg>)rawliteral";

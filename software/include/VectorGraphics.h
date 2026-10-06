#pragma once
#include <Arduino.h>

// =============================================================================
// ESP-OOBM 100% Vector SVG Logo (Pure Paths - Offline AP Mode / Zero Font Dependency)
// Stored in Flash ROM (PROGMEM) - Zero Dynamic RAM Allocation
// =============================================================================
static const char OOBM_LOGO_SVG[] PROGMEM = 
R"rawliteral(<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 330 46" style="height:46px;width:auto;display:block;">
  <defs>
    <linearGradient id="oobmGrad" x1="0%" y1="0%" x2="100%" y2="100%">
      <stop offset="0%" stop-color="#0284c7"/>
      <stop offset="50%" stop-color="#00c0d8"/>
      <stop offset="100%" stop-color="#10b981"/>
    </linearGradient>
    <linearGradient id="oobmGradCyan" x1="0%" y1="0%" x2="100%" y2="0%">
      <stop offset="0%" stop-color="#0284c7"/>
      <stop offset="100%" stop-color="#00c0d8"/>
    </linearGradient>
  </defs>
  <g transform="translate(1, 0)">
    <!-- USB-A Plug Collar (Terminates flush at x=15 with butt cap) -->
    <path d="M15 14.5 L4.5 14.5 A2.5 2.5 0 0 0 2 17 L2 27 A2.5 2.5 0 0 0 4.5 29.5 L15 29.5" fill="none" stroke="url(#oobmGrad)" stroke-width="2.8" stroke-linecap="butt" stroke-linejoin="round"/>
    <!-- USB Dongle Body (Full closed rounded rectangle) -->
    <rect x="15" y="7" width="56" height="30" rx="7" fill="none" stroke="url(#oobmGrad)" stroke-width="2.8"/>
    <!-- 3 Chevrons '>>>' -->
    <path d="M27 16 L34.5 22 L27 28 M39 16 L46.5 22 L39 28 M51 16 L58.5 22 L51 28" fill="none" stroke="url(#oobmGradCyan)" stroke-width="3.0" stroke-linecap="round" stroke-linejoin="round"/>
    <!-- WiFi Radiance Arcs on Right -->
    <path d="M78 13.5 A14 14 0 0 1 78 30.5 M84 8.5 A21 21 0 0 1 84 35.5" fill="none" stroke="url(#oobmGrad)" stroke-width="2.8" stroke-linecap="round"/>
  </g>
  <!-- Pure Vector Typography (100% Offline / Zero Font Dependency) -->
  <path class="brand-title-prefix" d="M100.6,9.8L113.0,9.8L113.0,12.7L103.9,12.7L103.9,17.0L112.3,17.0L112.3,19.8L103.9,19.8L103.9,24.2L113.0,24.2L113.0,27.0L100.6,27.0L100.6,9.8Z M116.1,24.4L116.1,21.9L119.4,21.9L119.4,23.3L120.2,24.2L125.2,24.2L126.1,23.3L126.1,20.5L125.3,19.7L118.8,19.7L116.1,17.0L116.1,12.5L118.8,9.8L126.5,9.8L129.2,12.5L129.2,15.0L125.9,15.0L125.9,13.5L125.1,12.7L120.2,12.7L119.4,13.5L119.4,16.0L120.2,16.8L126.7,16.8L129.4,19.5L129.4,24.3L126.7,27.0L118.7,27.0L116.1,24.4Z M133.0,9.8L143.8,9.8L146.4,12.5L146.4,18.5L143.8,21.2L136.4,21.2L136.4,27.0L133.0,27.0L133.0,9.8ZM142.3,18.4L143.1,17.5L143.1,13.5L142.3,12.6L136.4,12.6L136.4,18.4L142.3,18.4Z" fill="#0284c7" stroke="#0284c7" stroke-width="0.9" style="paint-order:stroke fill;fill-rule:evenodd;"/>
  <path class="brand-title-main" d="M149.3,19.3L157.9,19.3L157.9,22.1L149.3,22.1L149.3,19.3Z M161.0,24.2L161.0,12.7L163.8,9.8L172.6,9.8L175.5,12.7L175.5,24.2L172.6,27.0L163.8,27.0L161.0,24.2ZM170.9,24.2L172.1,22.9L172.1,13.9L170.9,12.7L165.6,12.7L164.3,13.9L164.3,22.9L165.6,24.2L170.9,24.2Z M179.0,24.2L179.0,12.7L181.8,9.8L190.6,9.8L193.4,12.7L193.4,24.2L190.6,27.0L181.8,27.0L179.0,24.2ZM188.8,24.2L190.1,22.9L190.1,13.9L188.8,12.7L183.5,12.7L182.3,13.9L182.3,22.9L183.5,24.2L188.8,24.2Z M197.2,9.8L207.9,9.8L210.4,12.3L210.4,17.1L209.4,18.1L211.1,19.9L211.1,24.4L208.5,27.0L197.2,27.0L197.2,9.8ZM206.2,17.0L207.1,16.1L207.1,13.5L206.2,12.6L200.4,12.6L200.4,17.0L206.2,17.0ZM206.8,24.2L207.8,23.2L207.8,20.8L206.8,19.7L200.4,19.7L200.4,24.2L206.8,24.2Z M214.4,9.8L217.5,9.8L222.6,20.9L222.6,20.9L227.7,9.8L230.8,9.8L230.8,27.0L227.6,27.0L227.6,16.6L227.6,16.6L223.6,24.6L221.6,24.6L217.6,16.6L217.6,16.6L217.6,27.0L214.4,27.0L214.4,9.8Z" fill="#091a36" style="fill-rule:evenodd;"/>
  <path class="brand-sub" d="M100.5,39.9L100.5,35.5L101.6,34.4L105.0,34.4L106.1,35.5L106.1,39.9L105.0,41.0L101.6,41.0L100.5,39.9ZM104.3,39.9L104.8,39.4L104.8,36.0L104.3,35.5L102.3,35.5L101.8,36.0L101.8,39.4L102.3,39.9L104.3,39.9Z M109.0,39.9L109.0,34.4L110.3,34.4L110.3,39.4L110.7,39.9L112.6,39.9L113.1,39.4L113.1,34.4L114.3,34.4L114.3,39.9L113.2,41.0L110.1,41.0L109.0,39.9Z M118.9,35.5L116.9,35.5L116.9,34.4L122.1,34.4L122.1,35.5L120.1,35.5L120.1,41.0L118.9,41.0L118.9,35.5Z M124.4,38.0L127.7,38.0L127.7,39.1L124.4,39.1L124.4,38.0Z M130.4,39.9L130.4,35.5L131.5,34.4L134.9,34.4L136.0,35.5L136.0,39.9L134.9,41.0L131.5,41.0L130.4,39.9ZM134.2,39.9L134.7,39.4L134.7,36.0L134.2,35.5L132.2,35.5L131.7,36.0L131.7,39.4L132.2,39.9L134.2,39.9Z M138.9,34.4L143.6,34.4L143.6,35.5L140.2,35.5L140.2,37.3L143.1,37.3L143.1,38.4L140.2,38.4L140.2,41.0L138.9,41.0L138.9,34.4Z M145.9,38.0L149.2,38.0L149.2,39.1L145.9,39.1L145.9,38.0Z M152.0,34.4L156.1,34.4L157.0,35.4L157.0,37.2L156.7,37.6L157.3,38.3L157.3,40.0L156.3,41.0L152.0,41.0L152.0,34.4ZM155.4,37.2L155.8,36.8L155.8,35.8L155.5,35.5L153.2,35.5L153.2,37.2L155.4,37.2ZM155.7,39.9L156.1,39.5L156.1,38.6L155.7,38.2L153.2,38.2L153.2,39.9L155.7,39.9Z M161.9,34.4L163.1,34.4L165.5,41.0L164.2,41.0L163.7,39.5L161.3,39.5L160.8,41.0L159.5,41.0L161.9,34.4ZM163.4,38.5L162.5,36.0L162.5,36.0L161.6,38.5L163.4,38.5Z M167.9,34.4L169.1,34.4L172.0,38.9L172.0,38.9L172.0,34.4L173.3,34.4L173.3,41.0L172.1,41.0L169.2,36.5L169.2,36.5L169.2,41.0L167.9,41.0L167.9,34.4Z M176.3,34.4L180.5,34.4L181.6,35.5L181.6,39.9L180.5,41.0L176.3,41.0L176.3,34.4ZM179.9,39.9L180.4,39.4L180.4,36.0L179.9,35.5L177.6,35.5L177.6,39.9L179.9,39.9Z M188.9,34.4L190.1,34.4L192.1,38.7L192.1,38.7L194.0,34.4L195.2,34.4L195.2,41.0L194.0,41.0L194.0,37.0L194.0,37.0L192.5,40.1L191.7,40.1L190.2,37.0L190.2,37.0L190.2,41.0L188.9,41.0L188.9,34.4Z M200.1,34.4L201.3,34.4L203.7,41.0L202.4,41.0L201.8,39.5L199.5,39.5L199.0,41.0L197.7,41.0L200.1,34.4ZM201.6,38.5L200.7,36.0L200.7,36.0L199.8,38.5L201.6,38.5Z M206.1,34.4L207.3,34.4L210.2,38.9L210.2,38.9L210.2,34.4L211.5,34.4L211.5,41.0L210.3,41.0L207.4,36.5L207.4,36.5L207.4,41.0L206.1,41.0L206.1,34.4Z M216.3,34.4L217.5,34.4L219.9,41.0L218.6,41.0L218.1,39.5L215.7,39.5L215.2,41.0L213.9,41.0L216.3,34.4ZM217.8,38.5L216.9,36.0L216.9,36.0L216.0,38.5L217.8,38.5Z M222.3,39.9L222.3,35.5L223.3,34.4L226.6,34.4L227.7,35.5L227.7,36.5L226.4,36.5L226.4,36.0L225.9,35.5L224.0,35.5L223.5,36.0L223.5,39.4L224.0,39.9L226.0,39.9L226.4,39.4L226.4,38.4L225.0,38.4L225.0,37.3L227.7,37.3L227.7,39.9L226.6,41.0L223.3,41.0L222.3,39.9Z M230.5,34.4L235.3,34.4L235.3,35.5L231.8,35.5L231.8,37.2L235.0,37.2L235.0,38.2L231.8,38.2L231.8,39.9L235.3,39.9L235.3,41.0L230.5,41.0L230.5,34.4Z M238.0,34.4L239.2,34.4L241.1,38.7L241.2,38.7L243.1,34.4L244.3,34.4L244.3,41.0L243.1,41.0L243.1,37.0L243.1,37.0L241.5,40.1L240.8,40.1L239.2,37.0L239.2,37.0L239.2,41.0L238.0,41.0L238.0,34.4Z M247.3,34.4L252.1,34.4L252.1,35.5L248.6,35.5L248.6,37.2L251.8,37.2L251.8,38.2L248.6,38.2L248.6,39.9L252.1,39.9L252.1,41.0L247.3,41.0L247.3,34.4Z M254.8,34.4L256.0,34.4L258.9,38.9L258.9,38.9L258.9,34.4L260.2,34.4L260.2,41.0L259.0,41.0L256.1,36.5L256.1,36.5L256.1,41.0L254.8,41.0L254.8,34.4Z M264.7,35.5L262.7,35.5L262.7,34.4L267.9,34.4L267.9,35.5L266.0,35.5L266.0,41.0L264.7,41.0L264.7,35.5Z" fill="#009aa0" style="fill-rule:evenodd;"/>
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
    <!-- USB-A Plug Collar (Terminates flush at x=7 with butt cap) -->
    <path d="M7 16 L2.5 16 A1.5 1.5 0 0 0 1 17.5 L1 28.5 A1.5 1.5 0 0 0 2.5 30 L7 30" fill="none" stroke="url(#favOobmGrad)" stroke-width="2.2" stroke-linecap="butt" stroke-linejoin="round"/>
    <!-- USB Dongle Body (Full closed rounded rectangle) -->
    <rect x="7" y="9" width="28" height="28" rx="6" fill="none" stroke="url(#favOobmGrad)" stroke-width="2.4"/>
    <!-- 3 Chevrons Inside -->
    <path d="M13 18 L17.5 23 L13 28" fill="none" stroke="#38bdf8" stroke-width="2.2" stroke-linecap="round" stroke-linejoin="round"/>
    <path d="M19 18 L23.5 23 L19 28" fill="none" stroke="#38bdf8" stroke-width="2.2" stroke-linecap="round" stroke-linejoin="round"/>
    <path d="M25 18 L29.5 23 L25 28" fill="none" stroke="#38bdf8" stroke-width="2.2" stroke-linecap="round" stroke-linejoin="round"/>
    <!-- WiFi Radiance Arcs on Right -->
    <path d="M38 15 A11 11 0 0 1 38 31" fill="none" stroke="url(#favOobmGrad)" stroke-width="2.4" stroke-linecap="round"/>
    <path d="M43 10 A18 18 0 0 1 43 36" fill="none" stroke="url(#favOobmGrad)" stroke-width="2.4" stroke-linecap="round"/>
  </g>
</svg>)rawliteral";

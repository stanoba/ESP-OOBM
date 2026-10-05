#pragma once
#include <Arduino.h>

// =============================================================================
// ESP-OOBM Default 25-Year Built-in TLS Wildcard Certificate
// Auto-generated build asset stored in Flash ROM (PROGMEM) - Zero Dynamic RAM
// Common Name: *.local (SAN: *.local, *.lan, *.internal, esp-oobm.local, 192.168.4.1)
// Validity: 2026-10-04 to 2051-10-05 (25 Years)
// Key Algorithm: ECDSA secp256r1 (256-bit ECC)
// =============================================================================

#define DEFAULT_TLS_CERT_CN         "*.local"
#define DEFAULT_TLS_CERT_ISSUER     "ESP-OOBM Embedded CA"
#define DEFAULT_TLS_CERT_EXPIRY     "2051-10-05"
#define DEFAULT_TLS_CERT_KEY_TYPE   "ECDSA (secp256r1 256-bit)"
#define DEFAULT_TLS_CERT_SANS       "*.local, *.lan, *.internal, *.home.arpa, esp-oobm.local, 192.168.4.1"

static const char DEFAULT_TLS_CERT_PEM[] PROGMEM = 
R"rawliteral(-----BEGIN CERTIFICATE-----
MIICPjCCAeSgAwIBAgIUe+MKHuOlTF43yrXoxmMkUyso1y4wCgYIKoZIzj0EAwIw
TjELMAkGA1UEBhMCU0sxETAPBgNVBAoMCEVTUC1PT0JNMRowGAYDVQQLDBFFbWJl
ZGRlZCBTZWN1cml0eTEQMA4GA1UEAwwHKi5sb2NhbDAgFw0yNjEwMDQxMzA2MDla
GA8yMDUxMTAwNTEzMDYwOVowTjELMAkGA1UEBhMCU0sxETAPBgNVBAoMCEVTUC1P
T0JNMRowGAYDVQQLDBFFbWJlZGRlZCBTZWN1cml0eTEQMA4GA1UEAwwHKi5sb2Nh
bDBZMBMGByqGSM49AgEGCCqGSM49AwEHA0IABL8NElTaIlNV67nT6Marjo0lWHyo
1v2U0IAL7qmXhJdVv+atUCiGHfih1AGapPUgreXMsiRz9XD3ClAClobe3OCjgZ0w
gZowWAYDVR0RBFEwT4IHKi5sb2NhbIIFKi5sYW6CCiouaW50ZXJuYWyCCyouaG9t
ZS5hcnBhgg5lc3Atb29ibS5sb2NhbIIIZXNwLW9vYm2HBMCoBAGHBH8AAAEwDwYD
VR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMCAaYwHQYDVR0lBBYwFAYIKwYBBQUH
AwEGCCsGAQUFBwMCMAoGCCqGSM49BAMCA0gAMEUCIDjPipi/7U7LZcrCZhtYYrsn
GtUhVIdz+t71yGUpCteDAiEA0AeIHaMIDw431b84119mrcbQV1ot34iueUMT1/v9
qAk=
-----END CERTIFICATE-----
)rawliteral";

static const char DEFAULT_TLS_KEY_PEM[] PROGMEM = 
R"rawliteral(-----BEGIN PRIVATE KEY-----
MIGHAgEAMBMGByqGSM49AgEGCCqGSM49AwEHBG0wawIBAQQgoEFh6jI0xhXdmun8
UqTaVz6qB9SwTdocGWC5SxSESf6hRANCAAS/DRJU2iJTVeu50+jGq46NJVh8qNb9
lNCAC+6pl4SXVb/mrVAohh34odQBmqT1IK3lzLIkc/Vw9wpQApaG3tzg
-----END PRIVATE KEY-----
)rawliteral";

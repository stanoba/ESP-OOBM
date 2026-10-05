#!/usr/bin/env python3
"""
ESP-OOBM Default 25-Year ECDSA Wildcard Certificate Generator
Generates software/include/DefaultCert.h with embedded X.509 cert and private key in Flash ROM.
"""
import datetime
import ipaddress
import os
import sys

try:
    from cryptography import x509
    from cryptography.x509.oid import NameOID
    from cryptography.hazmat.primitives import hashes, serialization
    from cryptography.hazmat.primitives.asymmetric import ec
except ImportError:
    print("Error: 'cryptography' Python package is required. Install with: pip install cryptography")
    sys.exit(1)

def generate_default_certificate(output_header_path):
    print("[TLS] Generating 25-Year ECDSA secp256r1 Wildcard Certificate...")
    
    # 1. Generate ECC secp256r1 private key
    key = ec.generate_private_key(ec.SECP256R1())
    
    # 2. Subject & Issuer (Self-Signed Root/Leaf)
    subject = issuer = x509.Name([
        x509.NameAttribute(NameOID.COUNTRY_NAME, "SK"),
        x509.NameAttribute(NameOID.ORGANIZATION_NAME, "ESP-OOBM"),
        x509.NameAttribute(NameOID.ORGANIZATIONAL_UNIT_NAME, "Embedded Security"),
        x509.NameAttribute(NameOID.COMMON_NAME, "*.local"),
    ])
    
    # 3. Subject Alternative Names (SAN)
    san = x509.SubjectAlternativeName([
        x509.DNSName("*.local"),
        x509.DNSName("*.lan"),
        x509.DNSName("*.internal"),
        x509.DNSName("*.home.arpa"),
        x509.DNSName("esp-oobm.local"),
        x509.DNSName("esp-oobm"),
        x509.IPAddress(ipaddress.IPv4Address("192.168.4.1")),
        x509.IPAddress(ipaddress.IPv4Address("127.0.0.1")),
    ])
    
    # 4. Validity: 25 Years (from yesterday to avoid timezone offset edge cases)
    now = datetime.datetime.now(datetime.timezone.utc)
    not_before = now - datetime.timedelta(days=1)
    not_after = now + datetime.timedelta(days=365 * 25 + 6) # 25 years with leap days
    
    # 5. Build X.509 Certificate
    cert = (
        x509.CertificateBuilder()
        .subject_name(subject)
        .issuer_name(issuer)
        .public_key(key.public_key())
        .serial_number(x509.random_serial_number())
        .not_valid_before(not_before)
        .not_valid_after(not_after)
        .add_extension(san, critical=False)
        .add_extension(x509.BasicConstraints(ca=True, path_length=None), critical=True)
        .add_extension(
            x509.KeyUsage(
                digital_signature=True,
                content_commitment=False,
                key_encipherment=True,
                data_encipherment=False,
                key_agreement=False,
                key_cert_sign=True,
                crl_sign=True,
                encipher_only=False,
                decipher_only=False,
            ),
            critical=True,
        )
        .add_extension(
            x509.ExtendedKeyUsage([
                x509.ExtendedKeyUsageOID.SERVER_AUTH,
                x509.ExtendedKeyUsageOID.CLIENT_AUTH,
            ]),
            critical=False,
        )
        .sign(key, hashes.SHA256())
    )
    
    cert_pem = cert.public_bytes(serialization.Encoding.PEM).decode("utf-8").strip()
    key_pem = key.private_bytes(
        encoding=serialization.Encoding.PEM,
        format=serialization.PrivateFormat.PKCS8,
        encryption_algorithm=serialization.NoEncryption(),
    ).decode("utf-8").strip()
    
    expiry_str = cert.not_valid_after_utc.strftime("%Y-%m-%d")
    
    header_content = f"""#pragma once
#include <Arduino.h>

// =============================================================================
// ESP-OOBM Default 25-Year Built-in TLS Wildcard Certificate
// Auto-generated build asset stored in Flash ROM (PROGMEM) - Zero Dynamic RAM
// Common Name: *.local (SAN: *.local, *.lan, *.internal, esp-oobm.local, 192.168.4.1)
// Validity: {not_before.strftime('%Y-%m-%d')} to {expiry_str} (25 Years)
// Key Algorithm: ECDSA secp256r1 (256-bit ECC)
// =============================================================================

#define DEFAULT_TLS_CERT_CN         "*.local"
#define DEFAULT_TLS_CERT_ISSUER     "ESP-OOBM Embedded CA"
#define DEFAULT_TLS_CERT_EXPIRY     "{expiry_str}"
#define DEFAULT_TLS_CERT_KEY_TYPE   "ECDSA (secp256r1 256-bit)"
#define DEFAULT_TLS_CERT_SANS       "*.local, *.lan, *.internal, *.home.arpa, esp-oobm.local, 192.168.4.1"

static const char DEFAULT_TLS_CERT_PEM[] PROGMEM = 
R"rawliteral({cert_pem}
)rawliteral";

static const char DEFAULT_TLS_KEY_PEM[] PROGMEM = 
R"rawliteral({key_pem}
)rawliteral";
"""

    os.makedirs(os.path.dirname(output_header_path), exist_ok=True)
    with open(output_header_path, "w", encoding="utf-8") as f:
        f.write(header_content)
        
    print(f"[TLS] Certificate successfully written to: {output_header_path}")
    print(f"[TLS] Cert Size: {len(cert_pem)} bytes, Key Size: {len(key_pem)} bytes, Expires: {expiry_str}")

if __name__ == "__main__":
    script_dir = os.path.dirname(os.path.abspath(__file__))
    target_path = os.path.abspath(os.path.join(script_dir, "..", "include", "DefaultCert.h"))
    if len(sys.argv) > 1:
        target_path = sys.argv[1]
    generate_default_certificate(target_path)

#include "TlsManager.h"
#include "ConsoleLogger.h"
#include <mbedtls/x509_crt.h>
#include <mbedtls/pk.h>

const char* TlsManager::CERT_FILE_PATH = "/cert.pem";
const char* TlsManager::KEY_FILE_PATH  = "/key.pem";

TlsManager tlsManager;

TlsManager::TlsManager() : _isCustom(false), _customCertCache(""), _customKeyCache("") {}

bool TlsManager::begin() {
    if (!SPIFFS.begin(true)) {
        logger.logError("[TLS] Failed to mount SPIFFS partition for certificates.");
        loadActiveCertificate();
        return false;
    }

    loadActiveCertificate();
    TlsCertInfo info = getCertInfo();
    logger.logInfo("[TLS] Active Certificate: %s (CN: %s, Expiry: %s)", 
                   info.isCustom ? "Custom (SPIFFS)" : "Built-in Wildcard (Flash)",
                   info.subjectCn.c_str(),
                   info.validTo.c_str());
    return true;
}

void TlsManager::loadActiveCertificate() {
    _isCustom = false;
    _customCertCache = "";
    _customKeyCache = "";

    if (SPIFFS.exists(CERT_FILE_PATH) && SPIFFS.exists(KEY_FILE_PATH)) {
        File certFile = SPIFFS.open(CERT_FILE_PATH, "r");
        File keyFile  = SPIFFS.open(KEY_FILE_PATH, "r");
        if (certFile && keyFile) {
            String certContent = certFile.readString();
            String keyContent  = keyFile.readString();
            certFile.close();
            keyFile.close();

            certContent.trim();
            keyContent.trim();

            if (certContent.indexOf("-----BEGIN CERTIFICATE-----") != -1 &&
                (keyContent.indexOf("-----BEGIN ") != -1 && keyContent.indexOf("KEY-----") != -1)) {
                _customCertCache = certContent;
                _customKeyCache  = keyContent;
                _isCustom = true;
                return;
            }
        }
        if (certFile) certFile.close();
        if (keyFile)  keyFile.close();
    }
}

const char* TlsManager::getCertPem() const {
    if (_isCustom && _customCertCache.length() > 0) {
        return _customCertCache.c_str();
    }
    return DEFAULT_TLS_CERT_PEM;
}

size_t TlsManager::getCertLen() const {
    if (_isCustom && _customCertCache.length() > 0) {
        return _customCertCache.length() + 1;
    }
    return strlen_P(DEFAULT_TLS_CERT_PEM) + 1;
}

const char* TlsManager::getKeyPem() const {
    if (_isCustom && _customKeyCache.length() > 0) {
        return _customKeyCache.c_str();
    }
    return DEFAULT_TLS_KEY_PEM;
}

size_t TlsManager::getKeyLen() const {
    if (_isCustom && _customKeyCache.length() > 0) {
        return _customKeyCache.length() + 1;
    }
    return strlen_P(DEFAULT_TLS_KEY_PEM) + 1;
}

TlsCertInfo TlsManager::getCertInfo() const {
    TlsCertInfo info;
    info.isCustom = _isCustom;

    if (!_isCustom) {
        info.subjectCn = DEFAULT_TLS_CERT_CN;
        info.issuer    = DEFAULT_TLS_CERT_ISSUER;
        info.validTo   = DEFAULT_TLS_CERT_EXPIRY;
        info.keyType   = DEFAULT_TLS_CERT_KEY_TYPE;
        info.sans      = DEFAULT_TLS_CERT_SANS;
        info.certSizeBytes = strlen_P(DEFAULT_TLS_CERT_PEM);
        info.keySizeBytes  = strlen_P(DEFAULT_TLS_KEY_PEM);
        return info;
    }

    info.certSizeBytes = _customCertCache.length();
    info.keySizeBytes  = _customKeyCache.length();
    parseCertMetadata(_customCertCache, info);
    return info;
}

void TlsManager::parseCertMetadata(const String &certPem, TlsCertInfo &info) const {
    mbedtls_x509_crt crt;
    mbedtls_x509_crt_init(&crt);

    int ret = mbedtls_x509_crt_parse(&crt, (const unsigned char*)certPem.c_str(), certPem.length() + 1);
    if (ret != 0) {
        info.subjectCn = "Parse Error";
        info.issuer    = "Unknown";
        info.validTo   = "Unknown";
        info.keyType   = "Unknown";
        info.sans      = "";
        mbedtls_x509_crt_free(&crt);
        return;
    }

    char buf[256];
    
    // Subject DN
    int len = mbedtls_x509_dn_gets(buf, sizeof(buf), &crt.subject);
    if (len > 0) {
        String subj(buf);
        int cnIdx = subj.indexOf("CN=");
        if (cnIdx != -1) {
            int endIdx = subj.indexOf(',', cnIdx);
            info.subjectCn = (endIdx != -1) ? subj.substring(cnIdx + 3, endIdx) : subj.substring(cnIdx + 3);
            info.subjectCn.trim();
        } else {
            info.subjectCn = subj;
        }
    } else {
        info.subjectCn = "Unknown";
    }

    // Issuer DN
    len = mbedtls_x509_dn_gets(buf, sizeof(buf), &crt.issuer);
    if (len > 0) {
        String iss(buf);
        int oIdx = iss.indexOf("O=");
        int cnIdx = iss.indexOf("CN=");
        if (cnIdx != -1) {
            int endIdx = iss.indexOf(',', cnIdx);
            info.issuer = (endIdx != -1) ? iss.substring(cnIdx + 3, endIdx) : iss.substring(cnIdx + 3);
            info.issuer.trim();
        } else if (oIdx != -1) {
            int endIdx = iss.indexOf(',', oIdx);
            info.issuer = (endIdx != -1) ? iss.substring(oIdx + 2, endIdx) : iss.substring(oIdx + 2);
            info.issuer.trim();
        } else {
            info.issuer = iss;
        }
    } else {
        info.issuer = "Unknown";
    }

    // Expiration
    char dateBuf[32];
    snprintf(dateBuf, sizeof(dateBuf), "%04d-%02d-%02d", crt.valid_to.year, crt.valid_to.mon, crt.valid_to.day);
    info.validTo = String(dateBuf);

    // Key Type
    mbedtls_pk_type_t pkType = mbedtls_pk_get_type(&crt.pk);
    if (pkType == MBEDTLS_PK_ECDSA || pkType == MBEDTLS_PK_ECKEY || pkType == MBEDTLS_PK_ECKEY_DH) {
        info.keyType = "ECDSA (ECC " + String(mbedtls_pk_get_bitlen(&crt.pk)) + "-bit)";
    } else if (pkType == MBEDTLS_PK_RSA) {
        info.keyType = "RSA (" + String(mbedtls_pk_get_bitlen(&crt.pk)) + "-bit)";
    } else {
        info.keyType = "Custom (" + String(mbedtls_pk_get_bitlen(&crt.pk)) + "-bit)";
    }

    // SAN
    info.sans = "";

    mbedtls_x509_crt_free(&crt);
}

bool TlsManager::saveCustomCert(const String &certPem, const String &keyPem) {
    String c = certPem;
    String k = keyPem;
    c.trim();
    k.trim();

    if (c.indexOf("-----BEGIN CERTIFICATE-----") == -1) {
        logger.logError("[TLS] Invalid certificate format (missing BEGIN CERTIFICATE).");
        return false;
    }
    if (k.indexOf("-----BEGIN ") == -1 || k.indexOf("KEY-----") == -1) {
        logger.logError("[TLS] Invalid private key format (missing BEGIN ... KEY).");
        return false;
    }

    // Validate parsing with mbedTLS before saving to disk
    mbedtls_x509_crt crt;
    mbedtls_x509_crt_init(&crt);
    int ret = mbedtls_x509_crt_parse(&crt, (const unsigned char*)c.c_str(), c.length() + 1);
    mbedtls_x509_crt_free(&crt);

    if (ret != 0) {
        logger.logError("[TLS] X.509 certificate validation failed (error code -0x%04X).", -ret);
        return false;
    }

    File certFile = SPIFFS.open(CERT_FILE_PATH, "w");
    if (!certFile) {
        logger.logError("[TLS] Failed to open /cert.pem for writing.");
        return false;
    }
    certFile.print(c);
    certFile.close();

    File keyFile = SPIFFS.open(KEY_FILE_PATH, "w");
    if (!keyFile) {
        logger.logError("[TLS] Failed to open /key.pem for writing.");
        return false;
    }
    keyFile.print(k);
    keyFile.close();

    _customCertCache = c;
    _customKeyCache  = k;
    _isCustom = true;

    TlsCertInfo info = getCertInfo();
    logger.logInfo("[TLS] Custom certificate saved successfully (CN: %s, Expiry: %s, %s).",
                   info.subjectCn.c_str(), info.validTo.c_str(), info.keyType.c_str());
    return true;
}

bool TlsManager::deleteCustomCert() {
    bool removed = false;
    if (SPIFFS.exists(CERT_FILE_PATH)) {
        SPIFFS.remove(CERT_FILE_PATH);
        removed = true;
    }
    if (SPIFFS.exists(KEY_FILE_PATH)) {
        SPIFFS.remove(KEY_FILE_PATH);
        removed = true;
    }

    _isCustom = false;
    _customCertCache = "";
    _customKeyCache = "";

    logger.logInfo("[TLS] Custom certificate deleted. Reverted to built-in firmware wildcard certificate.");
    return removed;
}

#pragma once
#include <Arduino.h>
#include <SPIFFS.h>
#include "DefaultCert.h"

struct TlsCertInfo {
    bool isCustom;
    String subjectCn;
    String issuer;
    String validTo;
    String keyType;
    String sans;
    size_t certSizeBytes;
    size_t keySizeBytes;
};

class TlsManager {
public:
    static const char* CERT_FILE_PATH;
    static const char* KEY_FILE_PATH;

    TlsManager();

    bool begin();
    
    // Active Certificate & Key Data
    const char* getCertPem() const;
    size_t getCertLen() const;
    const char* getKeyPem() const;
    size_t getKeyLen() const;

    bool isCustomActive() const { return _isCustom; }
    
    // Metadata inspection
    TlsCertInfo getCertInfo() const;

    // Management Actions
    bool saveCustomCert(const String &certPem, const String &keyPem);
    bool deleteCustomCert();

private:
    bool   _isCustom;
    String _customCertCache;
    String _customKeyCache;

    void loadActiveCertificate();
    void parseCertMetadata(const String &certPem, TlsCertInfo &info) const;
};

extern TlsManager tlsManager;

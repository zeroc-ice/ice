// **********************************************************************
//
// Copyright (c) 2002
// ZeroC, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICESSL_CONTEXT_H
#define ICESSL_CONTEXT_H

#include <Ice/LoggerF.h>
#include <Ice/PropertiesF.h>
#include <Ice/BuiltinSequences.h>
#include <IceSSL/OpenSSL.h>
#include <IceSSL/TraceLevelsF.h>
#include <IceSSL/CertificateVerifierOpenSSL.h>
#include <IceSSL/GeneralConfig.h>
#include <IceSSL/CertificateAuthority.h>
#include <IceSSL/BaseCerts.h>
#include <IceSSL/TempCerts.h>
#include <IceSSL/SslTransceiver.h>
#include <IceSSL/ContextF.h>
#include <IceSSL/RSAPublicKey.h>
#include <IceSSL/RSAKeyPairF.h>

namespace IceSSL
{

class Context : public IceUtil::Shared
{
public:

    virtual ~Context();

    bool isConfigured();

    virtual void setCertificateVerifier(const CertificateVerifierPtr&);

    virtual void addTrustedCertificateBase64(const std::string&);

    virtual void addTrustedCertificate(const Ice::ByteSeq&);

    virtual void setRSAKeysBase64(const std::string&, const std::string&);

    virtual void setRSAKeys(const Ice::ByteSeq&, const Ice::ByteSeq&);

    virtual void configure(const GeneralConfig&,
                           const CertificateAuthority&,
                           const BaseCertificates&);

    // Takes a socket fd as the first parameter.
    virtual SslTransceiverPtr createTransceiver(int, const OpenSSLPluginIPtr&) = 0;

protected:

    Context(const TraceLevelsPtr&, const Ice::LoggerPtr&, const Ice::PropertiesPtr&);

    SSL_METHOD* getSslMethod(SslProtocol);
    void createContext(SslProtocol);

    virtual void loadCertificateAuthority(const CertificateAuthority&);

    void setKeyCert(const CertificateDesc&, const std::string&, const std::string&);

    void checkKeyCert();

    void addTrustedCertificate(const RSAPublicKey&);

    void addKeyCert(const CertificateFile&, const CertificateFile&);

    void addKeyCert(const RSAKeyPair&);

    void addKeyCert(const Ice::ByteSeq&, const Ice::ByteSeq&);

    void addKeyCert(const std::string&, const std::string&);

    SSL* createSSLConnection(int);

    void transceiverSetup(const SslTransceiverPtr&);

    void setCipherList(const std::string&);

    void setDHParams(const BaseCertificates&);

    TraceLevelsPtr _traceLevels;
    Ice::LoggerPtr _logger;
    Ice::PropertiesPtr _properties;

    std::string _rsaPrivateKeyProperty;
    std::string _rsaPublicKeyProperty;
    std::string _dsaPrivateKeyProperty;
    std::string _dsaPublicKeyProperty;
    std::string _caCertificateProperty;
    std::string _handshakeTimeoutProperty;
    std::string _passphraseRetriesProperty;
    std::string _maxPassphraseRetriesDefault;

    CertificateVerifierPtr _certificateVerifier;

    SSL_CTX* _sslContext;

    int _maxPassphraseTries;
};

}

#endif

// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_SSL_CONTEXT_OPENSSL_H
#define ICE_SSL_CONTEXT_OPENSSL_H

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
#include <IceSSL/SslConnectionF.h>
#include <IceSSL/SslConnectionOpenSSLF.h>
#include <IceSSL/ContextOpenSSLF.h>
#include <IceSSL/RSAPublicKey.h>
#include <IceSSL/RSAKeyPairF.h>

namespace IceSSL
{

namespace OpenSSL
{

class PluginI;

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

    virtual void configure(const IceSSL::GeneralConfig&,
                           const IceSSL::CertificateAuthority&,
                           const IceSSL::BaseCertificates&);

    // Takes a socket fd as the first parameter.
    virtual ::IceSSL::ConnectionPtr createConnection(int, const IceSSL::PluginBaseIPtr&) = 0;

protected:

    Context(const IceSSL::TraceLevelsPtr&, const Ice::LoggerPtr&, const Ice::PropertiesPtr&);

    SSL_METHOD* getSslMethod(SslProtocol);
    void createContext(SslProtocol);

    virtual void loadCertificateAuthority(const CertificateAuthority&);

    void setKeyCert(const IceSSL::CertificateDesc&, const std::string&, const std::string&);

    void checkKeyCert();

    void addTrustedCertificate(const IceSSL::OpenSSL::RSAPublicKey&);

    void addKeyCert(const IceSSL::CertificateFile&, const IceSSL::CertificateFile&);

    void addKeyCert(const RSAKeyPair&);

    void addKeyCert(const Ice::ByteSeq&, const Ice::ByteSeq&);

    void addKeyCert(const std::string&, const std::string&);

    SSL* createSSLConnection(int);

    void connectionSetup(const IceSSL::OpenSSL::ConnectionPtr& connection);

    void setCipherList(const std::string&);

    void setDHParams(const IceSSL::BaseCertificates&);

    IceSSL::TraceLevelsPtr _traceLevels;
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

    IceSSL::CertificateVerifierPtr _certificateVerifier;

    SSL_CTX* _sslContext;

    int _maxPassphraseTries;

    friend class IceSSL::OpenSSL::PluginI;
};

}

}

#endif

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

#include <IceUtil/Config.h>
#include <IceUtil/Shared.h>
#include <Ice/InstanceF.h>
#include <Ice/TraceLevelsF.h>
#include <Ice/LoggerF.h>
#include <Ice/PropertiesF.h>
#include <Ice/BuiltinSequences.h>

#include <Ice/OpenSSL.h>

#include <Ice/SslCertificateVerifierOpenSSL.h>

#include <Ice/SslGeneralConfig.h>
#include <Ice/SslCertificateAuthority.h>
#include <Ice/SslBaseCerts.h>
#include <Ice/SslTempCerts.h>

#include <Ice/SslConnectionF.h>
#include <Ice/SslConnectionOpenSSLF.h>
#include <Ice/SslContextOpenSSLF.h>

namespace IceSSL
{

namespace OpenSSL
{

class System;
class RSAKeyPair;

class Context : public IceUtil::Shared
{

public:
    virtual ~Context();

    bool isConfigured();

    virtual void setCertificateVerifier(const CertificateVerifierPtr&);

    virtual void addTrustedCertificate(const std::string&);

    virtual void setRSAKeysBase64(const std::string&, const std::string&);

    virtual void setRSAKeys(const Ice::ByteSeq&, const Ice::ByteSeq&);

    virtual void configure(const IceSSL::GeneralConfig&,
                           const IceSSL::CertificateAuthority&,
                           const IceSSL::BaseCertificates&);

    // Takes a socket fd.
    virtual ::IceSSL::ConnectionPtr createConnection(int, const IceSSL::SystemInternalPtr&) = 0;

protected:
    Context(const IceInternal::InstancePtr&);

    SSL_METHOD* getSslMethod(SslProtocol);
    void createContext(SslProtocol);

    virtual void loadCertificateAuthority(const CertificateAuthority&);

    void setKeyCert(const IceSSL::CertificateDesc&, const std::string&, const std::string&);

    void checkKeyCert();

    void addKeyCert(const IceSSL::CertificateFile&, const IceSSL::CertificateFile&);

    void addKeyCert(const RSAKeyPair&);

    void addKeyCert(const Ice::ByteSeq&, const Ice::ByteSeq&);

    void addKeyCert(const std::string&, const std::string&);

    SSL* createSSLConnection(int);

    void connectionSetup(const IceSSL::OpenSSL::ConnectionPtr& connection);

    void setCipherList(const std::string&);

    void setDHParams(const IceSSL::BaseCertificates&);

    IceInternal::TraceLevelsPtr _traceLevels;
    Ice::LoggerPtr _logger;
    Ice::PropertiesPtr _properties;

    std::string _rsaPrivateKeyProperty;
    std::string _rsaPublicKeyProperty;
    std::string _dsaPrivateKeyProperty;
    std::string _dsaPublicKeyProperty;
    std::string _caCertificateProperty;
    std::string _handshakeTimeoutProperty;

    IceSSL::CertificateVerifierPtr _certificateVerifier;

    SSL_CTX* _sslContext;

    friend class IceSSL::OpenSSL::System;

};

}

}

#endif

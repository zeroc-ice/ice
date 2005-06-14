// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICESSL_CONTEXT_H
#define ICESSL_CONTEXT_H

#include <Ice/CommunicatorF.h>
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

    void cleanUp();

    virtual void setCertificateVerifier(const CertificateVerifierPtr&);

    virtual void addTrustedCertificateBase64(const std::string&);

    virtual void addTrustedCertificate(const Ice::ByteSeq&);

    virtual void setRSAKeysBase64(const std::string&, const std::string&);

    virtual void setRSAKeys(const Ice::ByteSeq&, const Ice::ByteSeq&);

    virtual void configure(const GeneralConfig&,
                           const CertificateAuthority&,
                           const BaseCertificates&);

    // Takes a socket fd as the first parameter, and the initial handshake timeout as the final.
    virtual SslTransceiverPtr createTransceiver(int, const OpenSSLPluginIPtr&, int) = 0;

protected:

    Context(const TraceLevelsPtr&, const Ice::CommunicatorPtr&, const ContextType&);

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

    void setCipherList(const std::string&);

    void setDHParams(const BaseCertificates&);

    TraceLevelsPtr _traceLevels;
    Ice::CommunicatorPtr _communicator;
    ContextType _contextType;

    std::string _rsaPrivateKeyProperty;
    std::string _rsaPublicKeyProperty;
    std::string _dsaPrivateKeyProperty;
    std::string _dsaPublicKeyProperty;
    std::string _caCertificateProperty;
    std::string _passphraseRetriesProperty;
    std::string _maxPassphraseRetriesDefault;
    std::string _connectionHandshakeRetries;

    CertificateVerifierPtr _certificateVerifier;

    SSL_CTX* _sslContext;

    int _maxPassphraseTries;
};

}

#endif

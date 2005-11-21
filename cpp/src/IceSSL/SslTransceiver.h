// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_SSL_TRANSCEIVER_H
#define ICE_SSL_TRANSCEIVER_H

#include <IceUtil/Mutex.h>
#include <Ice/LoggerF.h>
#include <Ice/StatsF.h>
#include <IceSSL/SslTransceiverF.h>
#include <IceSSL/OpenSSLPluginIF.h>
#include <IceSSL/TraceLevelsF.h>
#include <IceSSL/CertificateVerifierF.h>

#include <IceUtil/StaticMutex.h>
#include <Ice/Transceiver.h>
#include <Ice/Buffer.h>
#include <IceSSL/CertificateVerifierOpenSSL.h>

#include <openssl/ssl.h>
#include <map>

namespace IceSSL
{

// NOTE: This is a mapping from SSL* to SslTransceiver*, for use with
// the verifyCallback.  I have purposely not used SslTransceiverPtr
// here, as connections register themselves with this map on
// construction and unregister themselves in the destructor.  If this
// map used SslTransceiverPtr, SslTransceiver instances would never
// destruct as there would always be a reference to them from the map.
class SslTransceiver;
typedef std::map<SSL*, SslTransceiver*> SslTransceiverMap;

class SslTransceiver : public IceInternal::Transceiver
{
public:

    virtual SOCKET fd();
    virtual void close();
    virtual void shutdownWrite();
    virtual void shutdownReadWrite();
    virtual void write(IceInternal::Buffer&, int);
    virtual void read(IceInternal::Buffer&, int);
    virtual std::string type() const;
    virtual std::string toString() const;
    virtual void initialize(int timeout);

    static SslTransceiverPtr getTransceiver(SSL*);

    // Callback from OpenSSL for purposes of certificate verification
    int verifyCertificate(int, X509_STORE_CTX*);

private:

    void internalShutdownWrite(int timeout);

    void select(int, bool);
    void readSelect(int);
    void writeSelect(int);

    static void addTransceiver(SSL*, SslTransceiver*);
    static void removeTransceiver(SSL*);

#ifdef ICE_SSL_EXTRA_TRACING
    void showCertificateChain(BIO*);
    void showPeerCertificate(BIO*, const char*);
    void showSharedCiphers(BIO*);
    void showSessionInfo(BIO*);
    void showSelectedCipherInfo(BIO*);
    void showHandshakeStats(BIO*);
    void showClientCAList(BIO*, const char*);
#endif

    static SslTransceiverMap _transceiverMap;
    static IceUtil::StaticMutex _transceiverRepositoryMutex;

    SslTransceiver(ContextType, const OpenSSLPluginIPtr&, SOCKET, const IceSSL::CertificateVerifierPtr&, SSL*, int);
    virtual ~SslTransceiver();
    friend class ClientContext;
    friend class ServerContext;

    // Pointer to the OpenSSL Connection structure.
    const ContextType _contextType;
    const OpenSSLPluginIPtr _plugin;
    SOCKET _fd;
    const IceSSL::CertificateVerifierPtr _certificateVerifier;
    /*const*/ SSL* _sslConnection;
    const Ice::LoggerPtr _logger;
    const TraceLevelsPtr _traceLevels;
    const Ice::StatsPtr _stats;
};

}

#endif

// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_SSL_CONNECTION_OPENSSL_H
#define ICE_SSL_CONNECTION_OPENSSL_H

#include <openssl/ssl.h>
#include <Ice/SslConnection.h>
#include <Ice/SslSystem.h>
#include <Ice/Buffer.h>
#include <Ice/TraceLevels.h>
#include <Ice/Logger.h>

namespace IceSecurity
{

namespace Ssl
{

namespace OpenSSL
{

using namespace Ice;

class Connection : public IceSecurity::Ssl::Connection
{

public:

    Connection(SSL*, string&);
    virtual ~Connection();

    virtual void shutdown();

    virtual int read(Buffer&, int) = 0;
    virtual int write(Buffer&, int) = 0;

    virtual int init(int timeout = 0) = 0;

    void setTrace(TraceLevelsPtr traceLevels) { _traceLevels = traceLevels; };
    void setLogger(LoggerPtr traceLevels) { _logger = traceLevels; };


protected:

    int connect();
    int accept();
    int renegotiate();

    inline int pending() { return SSL_pending(_sslConnection); };
    inline int getLastError() const { return SSL_get_error(_sslConnection, _lastError); };

    int sslRead(char*, int);
    int sslWrite(char*, int);

    void printGetError(int);

    void protocolWrite();

    int readInBuffer(Buffer&);

    int readSelect(int);
    int writeSelect(int);

    int readSSL(Buffer&, int);

    virtual void showConnectionInfo() = 0;

    void showCertificateChain(BIO*);
    void showPeerCertificate(BIO*, const char*);
    void showSharedCiphers(BIO*);
    void showSessionInfo(BIO*);
    void showSelectedCipherInfo(BIO*);
    void showHandshakeStats(BIO*);
    void showClientCAList(BIO*, const char*);

    void setLastError(int errorCode) { _lastError = errorCode; };

    // Pointer to the OpenSSL Connection structure.
    SSL* _sslConnection;

    int _lastError;

    // TODO: Review this after a healthy stint of testing
    // Buffer for application data that may be returned during handshake
    // (probably won't contain anything, may be removed later).
    Buffer _inBuffer;
    JTCMutex _inBufferMutex;

    TraceLevelsPtr _traceLevels;
    LoggerPtr _logger;

    System* _system;

    JTCMutex _initMutex;
    int initWantRead;
    int initWantWrite;
};

}

}

}

#endif

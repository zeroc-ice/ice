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

#include <map>
#include <openssl/ssl.h>
#include <IceUtil/Mutex.h>
#include <Ice/SslConnection.h>
#include <Ice/SystemInternalF.h>
#include <Ice/SslConnectionOpenSSLF.h>
#include <Ice/CertificateVerifierOpenSSL.h>

namespace IceSSL
{

namespace OpenSSL
{

class SafeFlag
{
public:

    SafeFlag(bool flagVal = false)
    {
        _flag = flagVal;
    }

    ~SafeFlag()
    {
    }

    bool checkAndSet()
    {
        ::IceUtil::Mutex::Lock sync(_mutex);

        if (_flag)
        {
            return false;
        }
        else
        {
            _flag = true;
            return true;
        }
    }

    bool check()
    {
        ::IceUtil::Mutex::Lock sync(_mutex);
        return _flag;
    }

    void set()
    {
        ::IceUtil::Mutex::Lock sync(_mutex);
        _flag = true;
    }

    void unset()
    {
        ::IceUtil::Mutex::Lock sync(_mutex);
        _flag = false;
    }

private:

    ::IceUtil::Mutex _mutex;
    bool _flag;
};

class HandshakeSentinel
{
public:

    HandshakeSentinel(SafeFlag& handshakeFlag) :
	_flag(handshakeFlag)
    {
        _ownHandshake = _flag.checkAndSet();
    }
    
    ~HandshakeSentinel()
    { 
        if (_ownHandshake)
        {
            _flag.unset();
        }
    }

    bool ownHandshake()
    {
        return _ownHandshake;
    }

private:

    bool _ownHandshake;
    SafeFlag& _flag;
};

// NOTE: This is a mapping from SSL* to Connection*, for use with the verifyCallback.
//       I have purposely not used ConnectionPtr here, as connections register themselves
//       with this map on construction and unregister themselves in the destructor.  If
//       this map used ConnectionPtr, Connection instances would never destruct as there
//       would always be a reference to them from the map.
typedef std::map<SSL*, Connection*> SslConnectionMap;

class Connection : public IceSSL::Connection
{
public:

    Connection(const IceInternal::TraceLevelsPtr&,
               const Ice::LoggerPtr&,
               const IceSSL::CertificateVerifierPtr&,
               SSL*,
               const IceSSL::SystemInternalPtr&);
    virtual ~Connection();

    virtual void shutdown();

    virtual int read(IceInternal::Buffer&, int);
    virtual int write(IceInternal::Buffer&, int) = 0;

    virtual int init(int timeout = 0) = 0;

    void setHandshakeReadTimeout(int timeout);

    static ConnectionPtr getConnection(SSL*);

    // Callback from OpenSSL for purposes of certificate verification
    int verifyCertificate(int, X509_STORE_CTX*);

protected:

    int connect();
    int accept();
    int renegotiate();
    int initialize(int timeout);

    int pending();
    int getLastError() const;

    int sslRead(char*, int);
    int sslWrite(char*, int);

    int select(int, bool);
    int readSelect(int);
    int writeSelect(int);

    int readSSL(IceInternal::Buffer&, int);

    static void addConnection(SSL*, Connection*);
    static void removeConnection(SSL*);

    virtual void showConnectionInfo() = 0;

    void showCertificateChain(BIO*);
    void showPeerCertificate(BIO*, const char*);
    void showSharedCiphers(BIO*);
    void showSessionInfo(BIO*);
    void showSelectedCipherInfo(BIO*);
    void showHandshakeStats(BIO*);
    void showClientCAList(BIO*, const char*);

    void setLastError(int errorCode) { _lastError = errorCode; };

    static SslConnectionMap _connectionMap;
    static ::IceUtil::Mutex _connectionRepositoryMutex;

    // Pointer to the OpenSSL Connection structure.
    SSL* _sslConnection;

    int _lastError;

    ::IceUtil::Mutex _handshakeWaitMutex;

    // IceInternal::TraceLevelsPtr _traceLevels;
    // Ice::LoggerPtr _logger;

    SafeFlag _handshakeFlag;
    int _initWantRead;
    int _initWantWrite;
    int _handshakeReadTimeout;
    int _readTimeout;
};

}

}

#endif

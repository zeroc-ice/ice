// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_SSL_TRANSCEIVER_H
#define ICE_SSL_TRANSCEIVER_H

#ifdef SSL_EXTENSION
#include <Ice/SslConnection.h>
#endif

#include <Ice/InstanceF.h>
#include <Ice/TraceLevelsF.h>
#include <Ice/LoggerF.h>
#include <Ice/Transceiver.h>

namespace IceInternal
{

#ifdef SSL_EXTENSION
using IceSecurity::Ssl::Connection;
#endif

class SslConnector;
class SslAcceptor;

class SslTransceiver : public Transceiver
{
public:

    virtual int fd();
    virtual void close();
    virtual void shutdown();
    virtual void write(Buffer&, int);
    virtual void read(Buffer&, int);
    virtual std::string toString() const;

private:

#ifdef SSL_EXTENSION
    SslTransceiver(const InstancePtr&, int, Connection*);
#else
    SslTransceiver(const InstancePtr&, int);
#endif

    virtual ~SslTransceiver();
    void cleanUpSSL();
    friend class SslConnector;
    friend class SslAcceptor;

    InstancePtr _instance;
    int _fd;
    TraceLevelsPtr _traceLevels;
    ::Ice::LoggerPtr _logger;
    fd_set _rFdSet;
    fd_set _wFdSet;

#ifdef SSL_EXTENSION
    Connection* _sslConnection;
#endif
};

}

#endif

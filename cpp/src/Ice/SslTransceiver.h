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

#include <Ice/SslConnectionF.h>
#include <Ice/InstanceF.h>
#include <Ice/TraceLevelsF.h>
#include <Ice/LoggerF.h>
#include <Ice/Transceiver.h>

namespace IceInternal
{

class SslConnector;
class SslAcceptor;

class SslTransceiver : public Transceiver
{
public:

    virtual SOCKET fd();
    virtual void close();
    virtual void shutdown();
    virtual void write(Buffer&, int);
    virtual void read(Buffer&, int);
    virtual std::string toString() const;

private:

    SslTransceiver(const InstancePtr&, SOCKET, const ::IceSSL::ConnectionPtr&);

    virtual ~SslTransceiver();
    friend class SslConnector;
    friend class SslAcceptor;

    InstancePtr _instance;
    SOCKET _fd;
    TraceLevelsPtr _traceLevels;
    ::Ice::LoggerPtr _logger;
    fd_set _rFdSet;
    fd_set _wFdSet;

    ::IceSSL::ConnectionPtr _sslConnection;
};

}

#endif

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

#include <IceSSL/SslConnectionF.h>
#include <Ice/InstanceF.h>
#include <Ice/TraceLevelsF.h>
#include <Ice/LoggerF.h>
#include <Ice/Transceiver.h>

namespace IceSSL
{

class SslConnector;
class SslAcceptor;

class SslTransceiver : public IceInternal::Transceiver
{
public:

    virtual SOCKET fd();
    virtual void close();
    virtual void shutdown();
    virtual void write(IceInternal::Buffer&, int);
    virtual void read(IceInternal::Buffer&, int);
    virtual std::string toString() const;

private:

    SslTransceiver(const IceInternal::InstancePtr&, SOCKET, const ::IceSSL::ConnectionPtr&);

    virtual ~SslTransceiver();
    friend class SslConnector;
    friend class SslAcceptor;

    IceInternal::InstancePtr _instance;
    IceInternal::TraceLevelsPtr _traceLevels;
    ::Ice::LoggerPtr _logger;
    SOCKET _fd;
    fd_set _rFdSet;
    fd_set _wFdSet;

    ::IceSSL::ConnectionPtr _sslConnection;
};

}

#endif

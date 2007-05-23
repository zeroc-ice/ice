// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_SSL_TRANSCEIVER_I_H
#define ICE_SSL_TRANSCEIVER_I_H

#include <IceSSL/InstanceF.h>
#include <IceSSL/Plugin.h>

#include <Ice/LoggerF.h>
#include <Ice/StatsF.h>
#include <Ice/Transceiver.h>

typedef struct ssl_st SSL;

namespace IceSSL
{

class ConnectorI;
class AcceptorI;

class TransceiverI : public IceInternal::Transceiver
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
    virtual void initialize(int);
    virtual void checkSendSize(const IceInternal::Buffer&, size_t);

    ConnectionInfo getConnectionInfo() const;

private:

    TransceiverI(const InstancePtr&, SSL*, SOCKET, bool, const std::string& = "");
    virtual ~TransceiverI();
    friend class ConnectorI;
    friend class AcceptorI;

    void shutdown();

    const InstancePtr _instance;
    const Ice::LoggerPtr _logger;
    const Ice::StatsPtr _stats;
    
    SSL* _ssl;
    SOCKET _fd;

    const std::string _adapterName;
    const bool _incoming;

    const std::string _desc;
#ifdef _WIN32
    int _maxPacketSize;
#endif
};
typedef IceUtil::Handle<TransceiverI> TransceiverIPtr;

}

#endif

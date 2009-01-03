// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
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

#include <IceUtil/Mutex.h>

typedef struct ssl_st SSL;

namespace IceSSL
{

class ConnectorI;
class AcceptorI;

class TransceiverI : public IceInternal::Transceiver
{
    enum State
    {
        StateNeedConnect,
        StateConnectPending,
        StateConnected
    };

public:

    virtual SOCKET fd();
    virtual void close();
    virtual bool write(IceInternal::Buffer&);
    virtual bool read(IceInternal::Buffer&);
    virtual std::string type() const;
    virtual std::string toString() const;
    virtual IceInternal::SocketStatus initialize();
    virtual void checkSendSize(const IceInternal::Buffer&, size_t);

    ConnectionInfo getConnectionInfo() const;

private:

    TransceiverI(const InstancePtr&, SSL*, SOCKET, const std::string&, bool, bool, const std::string& = "");
    virtual ~TransceiverI();
    friend class ConnectorI;
    friend class AcceptorI;

    const InstancePtr _instance;
    const Ice::LoggerPtr _logger;
    const Ice::StatsPtr _stats;
    
    SSL* _ssl;
    IceUtil::Mutex _sslMutex; // Access to the SSL data structure must be synchronized.

    SOCKET _fd;

    const std::string _host;

    const bool _incoming;
    const std::string _adapterName;

    State _state;
    std::string _desc;
#ifdef _WIN32
    int _maxPacketSize;
#endif
};
typedef IceUtil::Handle<TransceiverI> TransceiverIPtr;

}

#endif

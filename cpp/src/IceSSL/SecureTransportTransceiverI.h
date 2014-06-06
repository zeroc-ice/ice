// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_SSL_SECURE_TRANSPORT_TRANSCEIVER_I_H
#define ICE_SSL_SECURE_TRANSPORT_TRANSCEIVER_I_H

#include <IceSSL/Config.h>
#include <IceSSL/InstanceF.h>
#include <IceSSL/SSLEngineF.h>
#include <IceSSL/Plugin.h>

#include <Ice/Transceiver.h>
#include <Ice/Network.h>

#ifdef ICE_USE_SECURE_TRANSPORT

#include <Security/Security.h>
#include <CoreFoundation/CoreFoundation.h>

namespace IceSSL
{

class ConnectorI;
class AcceptorI;

class TransceiverI : public IceInternal::Transceiver, public IceInternal::NativeInfo
{
    enum State
    {
        StateNeedConnect,
        StateConnectPending,
        StateProxyConnectRequest,
        StateProxyConnectRequestPending,
        StateConnected,
        StateHandshakeComplete
    };

public:

    virtual IceInternal::NativeInfoPtr getNativeInfo();

    virtual IceInternal::SocketOperation initialize(IceInternal::Buffer&, IceInternal::Buffer&, bool&);
    virtual IceInternal::SocketOperation closing(bool, const Ice::LocalException&);
    virtual void close();
    virtual IceInternal::SocketOperation write(IceInternal::Buffer&);
    virtual IceInternal::SocketOperation read(IceInternal::Buffer&, bool&);

    virtual std::string protocol() const;
    virtual std::string toString() const;
    virtual Ice::ConnectionInfoPtr getInfo() const;
    virtual void checkSendSize(const IceInternal::Buffer&, size_t);

    ContextRef context() const;
    SecTrustRef trust() const;
    OSStatus writeRaw(const char*, size_t*) const;
    OSStatus readRaw(char*, size_t*) const;
    
private:

    TransceiverI(const InstancePtr&, SOCKET, const IceInternal::NetworkProxyPtr&, const std::string&,
                 const IceInternal::Address&);
    TransceiverI(const InstancePtr&, SOCKET, const std::string&);
    virtual ~TransceiverI();

    virtual NativeConnectionInfoPtr getNativeConnectionInfo() const;
    
    void traceConnection();
    
    bool writeRaw(IceInternal::Buffer&);
    bool readRaw(IceInternal::Buffer&);

    friend class ConnectorI;
    friend class AcceptorI;

    const InstancePtr _instance;
    const SecureTransportEnginePtr _engine;

    const IceInternal::NetworkProxyPtr _proxy;
    const std::string _host;
    const IceInternal::Address _addr;

    const std::string _adapterName;
    const bool _incoming;

    ContextRef _ssl;
    SecTrustRef _trust;
    
    size_t _buffered;
    enum SSLWantFlags
    {
        SSLWantRead = 0x1,
        SSLWantWrite = 0x2
    };
    
    mutable Ice::Byte _flags;

    State _state;
    std::string _desc;
    size_t _maxSendPacketSize;
    size_t _maxReceivePacketSize;
};
typedef IceUtil::Handle<TransceiverI> TransceiverIPtr;

}

#endif

#endif

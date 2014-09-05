// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_SSL_SCHANNELTRANSCEIVER_I_H
#define ICE_SSL_SCHANNELTRANSCEIVER_I_H

#include <IceSSL/Config.h>
#include <IceSSL/InstanceF.h>
#include <IceSSL/Plugin.h>
#include <IceSSL/SSLEngineF.h>

#include <Ice/Transceiver.h>
#include <Ice/Network.h>
#include <Ice/Buffer.h>

#ifdef ICE_USE_SCHANNEL

#ifdef SECURITY_WIN32
#  undef SECURITY_WIN32
#endif

#ifdef SECURITY_KERNEL
#  undef SECURITY_KERNEL
#endif

#define SECURITY_WIN32 1
#include <security.h>
#include <sspi.h>
#include <schannel.h>
#undef SECURITY_WIN32

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
        StateHandshakeReadContinue,
        StateHandshakeWriteContinue,
        StateHandshakeComplete
    };

public:

    virtual IceInternal::NativeInfoPtr getNativeInfo();
    virtual IceInternal::AsyncInfo* getAsyncInfo(IceInternal::SocketOperation);

    virtual IceInternal::SocketOperation initialize(IceInternal::Buffer&, IceInternal::Buffer&, bool&);
    virtual IceInternal::SocketOperation closing(bool, const Ice::LocalException&);
    virtual void close();
    virtual IceInternal::SocketOperation write(IceInternal::Buffer&);
    virtual IceInternal::SocketOperation read(IceInternal::Buffer&, bool&);
    virtual bool startWrite(IceInternal::Buffer&);
    virtual void finishWrite(IceInternal::Buffer&);
    virtual void startRead(IceInternal::Buffer&);
    virtual void finishRead(IceInternal::Buffer&, bool&);
    virtual std::string protocol() const;
    virtual std::string toString() const;
    virtual std::string toDetailedString() const;
    virtual Ice::ConnectionInfoPtr getInfo() const;
    virtual void checkSendSize(const IceInternal::Buffer&, size_t);

private:

    TransceiverI(const InstancePtr&, SOCKET, const IceInternal::NetworkProxyPtr&, const std::string&,
                 const IceInternal::Address&, const IceInternal::Address&);
    TransceiverI(const InstancePtr&, SOCKET, const std::string&);
    virtual ~TransceiverI();

    virtual NativeConnectionInfoPtr getNativeConnectionInfo() const;

    IceInternal::SocketOperation sslHandshake();

    size_t decryptMessage(IceInternal::Buffer&);
    size_t encryptMessage(IceInternal::Buffer&);

    bool writeRaw(IceInternal::Buffer&);
    bool readRaw(IceInternal::Buffer&);

    friend class ConnectorI;
    friend class AcceptorI;

    const InstancePtr _instance;
    const SChannelEnginePtr _engine;

    const IceInternal::NetworkProxyPtr _proxy;
    const std::string _host;
    const IceInternal::Address _addr;
    const IceInternal::Address _sourceAddr;

    const std::string _adapterName;
    const bool _incoming;

    State _state;
    std::string _desc;

    //
    // Buffered encrypted data that has not been written.
    //
    IceInternal::Buffer _writeBuffer;
    size_t _bufferedW;

    //
    // Buffered data that has not been decrypted.
    //
    IceInternal::Buffer _readBuffer;

    //
    // Buffered data that was decrypted but not yet processed.
    //
    IceInternal::Buffer _readUnprocessed;

    CtxtHandle _ssl;
    bool _sslInitialized;
    CredHandle _credentials;
    bool _credentialsInitialized;

#ifdef ICE_USE_IOCP
    IceInternal::AsyncInfo _read;
    IceInternal::AsyncInfo _write;
    int _maxSendPacketSize;
    int _maxReceivePacketSize;
#endif
    SecPkgContext_StreamSizes _sizes;
};
typedef IceUtil::Handle<TransceiverI> TransceiverIPtr;

}

#endif

#endif

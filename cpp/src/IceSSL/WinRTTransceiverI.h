// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_SSL_WINRT_TRANSCEIVER_I_H
#define ICE_SSL_WINRT_TRANSCEIVER_I_H

#include <IceSSL/Config.h>

#ifdef ICE_OS_WINRT

#include <IceSSL/InstanceF.h>
#include <IceSSL/SSLEngineF.h>
#include <IceSSL/Plugin.h>

#include <Ice/Transceiver.h>
#include <Ice/Network.h>

namespace IceSSL
{

class ConnectorI;
class AcceptorI;

class TransceiverI : public IceInternal::Transceiver
{
public:

    virtual IceInternal::NativeInfoPtr getNativeInfo();

    virtual IceInternal::SocketOperation initialize(IceInternal::Buffer&, IceInternal::Buffer&);
    virtual IceInternal::SocketOperation closing(bool, const Ice::LocalException&);
    virtual void close();
    virtual IceInternal::SocketOperation write(IceInternal::Buffer&);
    virtual IceInternal::SocketOperation read(IceInternal::Buffer&);
    virtual bool startWrite(IceInternal::Buffer&);
    virtual void finishWrite(IceInternal::Buffer&);
    virtual void startRead(IceInternal::Buffer&);
    virtual void finishRead(IceInternal::Buffer&);

    virtual std::string protocol() const;
    virtual std::string toString() const;
    virtual std::string toDetailedString() const;
    virtual Ice::ConnectionInfoPtr getInfo() const;
    virtual void checkSendSize(const IceInternal::Buffer&);
    virtual void setBufferSize(int rcvSize, int sndSize);

private:

    TransceiverI(const InstancePtr&, const IceInternal::TransceiverPtr&, const std::string&, bool);
    virtual ~TransceiverI();

    friend class ConnectorI;
    friend class AcceptorI;

    const InstancePtr _instance;
    const WinRTEnginePtr _engine;
    const std::string _host;
    const std::string _adapterName;
    const bool _incoming;
    const IceInternal::TransceiverPtr _delegate;

    bool _connected;
    bool _upgraded;
    bool _verified;
    Windows::Security::Cryptography::Certificates::CertificateChain^ _chain;
};
typedef IceUtil::Handle<TransceiverI> TransceiverIPtr;

}

#endif

#endif

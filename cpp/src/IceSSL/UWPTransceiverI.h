// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICESSL_UWP_TRANSCEIVER_I_H
#define ICESSL_UWP_TRANSCEIVER_I_H

#include <IceSSL/Config.h>
#include <IceSSL/InstanceF.h>
#include <IceSSL/UWPEngineF.h>
#include <IceSSL/Plugin.h>

#include <Ice/Transceiver.h>
#include <Ice/Network.h>

namespace IceSSL
{

namespace UWP
{

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

    friend class SSLEngine;

    const InstancePtr _instance;
    const SSLEnginePtr _engine;
    const std::string _host;
    const std::string _adapterName;
    const bool _incoming;
    const IceInternal::TransceiverPtr _delegate;

    bool _connected;
    bool _upgraded;
    std::vector<std::string> _certs;
    bool _verified;
    std::vector<IceSSL::CertificatePtr> _nativeCerts;
    Windows::Security::Cryptography::Certificates::CertificateChain^ _chain;
};
typedef IceUtil::Handle<TransceiverI> TransceiverIPtr;

} // UWP namespace end

} // IceSSL namespace end

#endif

// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICESSL_SECURE_TRANSPORT_TRANSCEIVER_I_H
#define ICESSL_SECURE_TRANSPORT_TRANSCEIVER_I_H

#ifdef __APPLE__

#include <IceSSL/Config.h>
#include <IceSSL/InstanceF.h>
#include <IceSSL/SecureTransportEngineF.h>
#include <IceSSL/Plugin.h>

#include <Ice/Transceiver.h>
#include <Ice/UniqueRef.h>
#include <Ice/Network.h>

#include <Security/Security.h>
#include <Security/SecureTransport.h>
#include <CoreFoundation/CoreFoundation.h>

namespace IceSSL
{

namespace SecureTransport
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

    virtual std::string protocol() const;
    virtual std::string toString() const;
    virtual std::string toDetailedString() const;
    virtual Ice::ConnectionInfoPtr getInfo() const;
    virtual void checkSendSize(const IceInternal::Buffer&);
    virtual void setBufferSize(int rcvSize, int sndSize);

    OSStatus writeRaw(const char*, size_t*) const;
    OSStatus readRaw(char*, size_t*) const;

private:

    TransceiverI(const InstancePtr&, const IceInternal::TransceiverPtr&, const std::string&, bool);
    virtual ~TransceiverI();

    friend class IceSSL::SecureTransport::SSLEngine;

    const InstancePtr _instance;
    const SSLEnginePtr _engine;
    const std::string _host;
    const std::string _adapterName;
    const bool _incoming;
    const IceInternal::TransceiverPtr _delegate;

    IceInternal::UniqueRef<SSLContextRef> _ssl;
    IceInternal::UniqueRef<SecTrustRef> _trust;
    bool _connected;

    enum SSLWantFlags
    {
        SSLWantRead = 0x1,
        SSLWantWrite = 0x2
    };

    mutable Ice::Byte _tflags;
    size_t _maxSendPacketSize;
    size_t _maxRecvPacketSize;
    std::string _cipher;
    std::vector<CertificatePtr> _certs;
    bool _verified;
    size_t _buffered;
};
typedef IceUtil::Handle<TransceiverI> TransceiverIPtr;

} // SecureTransport namespace end

} // IceSSL namespace end

#endif

#endif

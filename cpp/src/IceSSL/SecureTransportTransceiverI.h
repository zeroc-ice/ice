// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
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
#include <Ice/StreamSocket.h>
#include <Ice/WSTransceiver.h>

#ifdef ICE_USE_SECURE_TRANSPORT

#include <Security/Security.h>
#include <CoreFoundation/CoreFoundation.h>

namespace IceSSL
{

class ConnectorI;
class AcceptorI;

class TransceiverI : public IceInternal::Transceiver, public IceInternal::WSTransceiverDelegate
{
public:

    virtual IceInternal::NativeInfoPtr getNativeInfo();

    virtual IceInternal::SocketOperation initialize(IceInternal::Buffer&, IceInternal::Buffer&, bool&);
    virtual IceInternal::SocketOperation closing(bool, const Ice::LocalException&);
    virtual void close();
    virtual IceInternal::SocketOperation write(IceInternal::Buffer&);
    virtual IceInternal::SocketOperation read(IceInternal::Buffer&, bool&);

    virtual std::string protocol() const;
    virtual std::string toString() const;
    virtual std::string toDetailedString() const;
    virtual Ice::ConnectionInfoPtr getInfo() const;
    virtual Ice::ConnectionInfoPtr getWSInfo(const Ice::HeaderDict&) const;
    virtual void checkSendSize(const IceInternal::Buffer&);
    virtual void setBufferSize(int rcvSize, int sndSize);

    OSStatus writeRaw(const char*, size_t*) const;
    OSStatus readRaw(char*, size_t*) const;

private:

    TransceiverI(const InstancePtr&, const IceInternal::StreamSocketPtr&, const std::string&, bool);
    virtual ~TransceiverI();

    void fillConnectionInfo(const ConnectionInfoPtr&, std::vector<CertificatePtr>&) const;

    friend class ConnectorI;
    friend class AcceptorI;

    const InstancePtr _instance;
    const SecureTransportEnginePtr _engine;
    const std::string _host;
    const std::string _adapterName;
    const bool _incoming;
    const IceInternal::StreamSocketPtr _stream;

    SSLContextRef _ssl;
    SecTrustRef _trust;
    bool _verified;
    
    size_t _buffered;
    enum SSLWantFlags
    {
        SSLWantRead = 0x1,
        SSLWantWrite = 0x2
    };

    mutable Ice::Byte _flags;
    size_t _maxSendPacketSize;
    size_t _maxRecvPacketSize;
};
typedef IceUtil::Handle<TransceiverI> TransceiverIPtr;

}

#endif

#endif

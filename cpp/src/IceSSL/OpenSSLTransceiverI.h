//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICESSL_OPENSSL_TRANSCEIVER_I_H
#define ICESSL_OPENSSL_TRANSCEIVER_I_H

#include <IceSSL/Config.h>
#include <IceSSL/Util.h>
#include <IceSSL/InstanceF.h>
#include <IceSSL/Plugin.h>
#include <IceSSL/OpenSSLEngineF.h>

#include <Ice/Transceiver.h>
#include <Ice/Network.h>
#include <Ice/StreamSocket.h>
#include <Ice/WSTransceiver.h>

typedef struct ssl_st SSL;
typedef struct bio_st BIO;

namespace IceSSL
{

namespace OpenSSL
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
#ifdef ICE_USE_IOCP
    virtual bool startWrite(IceInternal::Buffer&);
    virtual void finishWrite(IceInternal::Buffer&);
    virtual void startRead(IceInternal::Buffer&);
    virtual void finishRead(IceInternal::Buffer&);
#endif
    virtual std::string protocol() const;
    virtual std::string toString() const;
    virtual std::string toDetailedString() const;
    virtual Ice::ConnectionInfoPtr getInfo() const;
    virtual void checkSendSize(const IceInternal::Buffer&);
    virtual void setBufferSize(int rcvSize, int sndSize);

    int verifyCallback(int , X509_STORE_CTX*);

private:

    TransceiverI(const InstancePtr&, const IceInternal::TransceiverPtr&, const std::string&, bool);
    virtual ~TransceiverI();

    bool receive();
    bool send();

    friend class IceSSL::OpenSSL::SSLEngine;

    const InstancePtr _instance;
    const IceSSL::OpenSSL::SSLEnginePtr _engine;
    const std::string _host;
    const std::string _adapterName;
    const bool _incoming;
    const IceInternal::TransceiverPtr _delegate;
    bool _connected;
    std::string _cipher;
    std::vector<IceSSL::CertificatePtr> _certs;
    bool _verified;

    SSL* _ssl;
    BIO* _memBio;
    IceInternal::Buffer _writeBuffer;
    IceInternal::Buffer _readBuffer;
    int _sentBytes;
    size_t _maxSendPacketSize;
    size_t _maxRecvPacketSize;
};
typedef IceUtil::Handle<TransceiverI> TransceiverIPtr;

} // OpenSSL namespace end

} // IceSSL namespace end

#endif

// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICESSL_OPENSSL_ENGINE_H
#define ICESSL_OPENSSL_ENGINE_H

#include <IceSSL/SSLEngine.h>
#include <IceSSL/InstanceF.h>
#include <IceSSL/OpenSSLUtil.h>

#include <Ice/BuiltinSequences.h>

namespace IceSSL
{

namespace OpenSSL
{

class SSLEngine : public IceSSL::SSLEngine
{
public:

    SSLEngine(const Ice::CommunicatorPtr&);
    ~SSLEngine();

    virtual void initialize();
    virtual void destroy();
    virtual void verifyPeer(const std::string&, const IceSSL::ConnectionInfoPtr&, const std::string&);
    virtual IceInternal::TransceiverPtr
    createTransceiver(const IceSSL::InstancePtr&, const IceInternal::TransceiverPtr&, const std::string&, bool);

#ifndef OPENSSL_NO_DH
    DH* dhParams(int);
#endif

    SSL_CTX* context() const;
    void context(SSL_CTX*);
    std::string sslErrors() const;

private:

    void cleanup();
    SSL_METHOD* getMethod(int);
    void setOptions(int);
    enum Protocols { SSLv3 = 0x01, TLSv1_0 = 0x02, TLSv1_1 = 0x04, TLSv1_2 = 0x08 };
    int parseProtocols(const Ice::StringSeq&) const;

    SSL_CTX* _ctx;

#ifndef OPENSSL_NO_DH
    IceSSL::OpenSSL::DHParamsPtr _dhParams;
#endif
};

} // OpenSSL namespace end

} // IceSSL namespace endif

#endif

// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_SSL_CONTEXT_H
#define ICE_SSL_CONTEXT_H

#include <InstanceF.h>
#include <UtilF.h>
#include <Ice/LoggerF.h>
#include <Ice/Network.h>

namespace IceSSL
{

class Context : public IceUtil::Shared
{
public:

    Context(const InstancePtr&, SSL_CTX*);
    ~Context();

    SSL_CTX* ctx() const;

    void verifyPeer(SSL*, SOCKET, const std::string&, bool);

    std::string password(bool);

#ifndef OPENSSL_NO_DH
    DH* dhParams(int);
#endif

    int verifyCallback(int, SSL*, X509_STORE_CTX*);

    void traceConnection(SSL*, bool);

protected:

    void parseProtocols(const std::string&);

    InstancePtr _instance;
    Ice::LoggerPtr _logger;
    SSL_CTX* _ctx;
    std::string _defaultDir;
    bool _checkCertName;
    std::string _password;
#ifndef OPENSSL_NO_DH
    DHParamsPtr _dhParams;
#endif
};
typedef IceUtil::Handle<Context> ContextPtr;

}

#endif

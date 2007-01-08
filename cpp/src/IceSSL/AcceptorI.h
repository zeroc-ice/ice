// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_SSL_ACCEPTOR_I_H
#define ICE_SSL_ACCEPTOR_I_H

#include <Ice/LoggerF.h>
#include <Ice/TransceiverF.h>
#include <Ice/Acceptor.h>
#include <IceSSL/InstanceF.h>

#ifndef _WIN32
#   include <netinet/in.h> // For struct sockaddr_in
#endif

namespace IceSSL
{

class EndpointI;

class AcceptorI : public IceInternal::Acceptor
{
public:

    virtual SOCKET fd();
    virtual void close();
    virtual void listen();
    virtual IceInternal::TransceiverPtr accept(int);
    virtual void connectToSelf();
    virtual std::string toString() const;

    bool equivalent(const std::string&, int) const;
    int effectivePort();

private:

    AcceptorI(const InstancePtr&, const std::string&, const std::string&, int);
    virtual ~AcceptorI();
    friend class EndpointI;

    const InstancePtr _instance;
    const std::string _adapterName;
    Ice::LoggerPtr _logger;
    SOCKET _fd;
    int _backlog;
    struct sockaddr_in _addr;
};

}

#endif

// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
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
#include <Ice/Protocol.h>
#include <IceSSL/InstanceF.h>

#ifndef _WIN32
#   include <sys/socket.h> // For struct sockaddr_storage
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
    virtual IceInternal::TransceiverPtr accept();
    virtual std::string toString() const;

    int effectivePort() const;

private:

    AcceptorI(const InstancePtr&, const std::string&, const std::string&, int);
    virtual ~AcceptorI();
    friend class EndpointI;

    const InstancePtr _instance;
    const std::string _adapterName;
    Ice::LoggerPtr _logger;
    SOCKET _fd;
    int _backlog;
    struct sockaddr_storage _addr;
};

}

#endif

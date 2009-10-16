// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_TCP_ACCEPTOR_H
#define ICE_TCP_ACCEPTOR_H

#include <Ice/TransceiverF.h>
#include <Ice/InstanceF.h>
#include <Ice/TraceLevelsF.h>
#include <Ice/LoggerF.h>
#include <Ice/Acceptor.h>

#ifndef _WIN32
#   include <sys/socket.h> // For struct sockaddr_storage
#endif

namespace IceInternal
{

class TcpAcceptor : public Acceptor, public NativeInfo
{
public:

    virtual NativeInfoPtr getNativeInfo();
#ifdef ICE_USE_IOCP
    virtual AsyncInfo* getAsyncInfo(SocketOperation);
#endif

    virtual void close();
    virtual void listen();
#ifdef ICE_USE_IOCP
    virtual void startAccept();
    virtual void finishAccept();
#endif
    virtual TransceiverPtr accept();
    virtual std::string toString() const;

    int effectivePort() const;

private:

    TcpAcceptor(const InstancePtr&, const Ice::TcpEndpointInfoPtr&);
    virtual ~TcpAcceptor();
    friend class TcpEndpointI;

    const InstancePtr _instance;
    const Ice::TcpEndpointInfoPtr _endpointInfo;
    const TraceLevelsPtr _traceLevels;
    const ::Ice::LoggerPtr _logger;
    const struct sockaddr_storage _addr;

    int _backlog;
#ifdef ICE_USE_IOCP
    SOCKET _acceptFd;
    int _acceptError;
    std::vector<char> _acceptBuf;
    AsyncInfo _info;
#endif
};

}

#endif

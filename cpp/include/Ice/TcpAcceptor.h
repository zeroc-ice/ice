// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_TCP_ACCEPTOR_H
#define ICE_TCP_ACCEPTOR_H

#include <Ice/TransceiverF.h>
#include <Ice/InstanceF.h>
#include <Ice/TraceLevelsF.h>
#include <Ice/LoggerF.h>
#include <Ice/Acceptor.h>

#ifndef WIN32
#   include <netinet/in.h> // For struct sockaddr_in
#endif

namespace __Ice
{

class TcpEndpoint;

class TcpAcceptor : public Acceptor
{
public:

    virtual int fd();
    virtual void close();
    virtual void shutdown();
    virtual void listen();
    virtual Transceiver_ptr accept(int);
    virtual std::string toString() const;

private:

    TcpAcceptor(Instance_ptr, int);
    virtual ~TcpAcceptor();
    friend class TcpEndpoint;

    Instance_ptr instance_;
    int fd_;
    int backlog_;
    struct sockaddr_in addr_;
#ifndef ICE_NO_TRACE
    TraceLevels_ptr traceLevels_;
    ::Ice::Logger_ptr logger_;
#endif
};

}

#endif

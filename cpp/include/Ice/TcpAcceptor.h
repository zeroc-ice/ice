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

#include <Ice/TcpAcceptorF.h>
#include <Ice/InstanceF.h>
#include <Ice/TcpTransceiverF.h>
#include <Ice/TraceLevelsF.h>
#include <Ice/LoggerF.h>
#include <Ice/Shared.h>

#ifndef WIN32
#   include <netinet/in.h> // For struct sockaddr_in
#endif

namespace __Ice
{

class CollectorFactoryI;

class TcpAcceptorI : public Shared
{
public:

    int fd();
    void close();
    void shutdown();
    void listen();
    TcpTransceiver accept(int);
    std::string toString() const;

private:

    TcpAcceptorI(const TcpAcceptorI&);
    void operator=(const TcpAcceptorI&);

    TcpAcceptorI(Instance, int);
    virtual ~TcpAcceptorI();
    friend class CollectorFactoryI; // May create TcpAcceptorIs

    void block(bool);

    Instance instance_;
    int fd_;
    bool block_;
    int backlog_;
    struct sockaddr_in addr_;
#ifndef ICE_NO_TRACE
    TraceLevels traceLevels_;
    ::Ice::Logger logger_;
#endif
};

}

#endif

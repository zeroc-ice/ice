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

class CollectorFactoryI;

class TcpAcceptorI : public AcceptorI
{
public:

    virtual int fd();
    virtual void close();
    virtual void shutdown();
    virtual Transceiver initialize();
    virtual Transceiver accept(int);
    virtual std::string toString() const;

private:

    TcpAcceptorI(const TcpAcceptorI&);
    void operator=(const TcpAcceptorI&);

    TcpAcceptorI(Instance, int);
    virtual ~TcpAcceptorI();
    friend class CollectorFactoryI; // May create TcpAcceptorIs

    Instance instance_;
    int fd_;
    int backlog_;
    struct sockaddr_in addr_;
#ifndef ICE_NO_TRACE
    TraceLevels traceLevels_;
    ::Ice::Logger logger_;
#endif
};

}

#endif

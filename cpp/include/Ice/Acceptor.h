// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_ACCEPTOR_H
#define ICE_ACCEPTOR_H

#include <Ice/AcceptorF.h>
#include <Ice/InstanceF.h>
#include <Ice/TransceiverF.h>
#include <Ice/Shared.h>
#include <netinet/in.h>

namespace _Ice
{

class CollectorFactoryI;

class AcceptorI : public Shared
{
public:
    
    int fd() { return fd_; }
    void shutdown();
    void listen();
    Transceiver accept();
    std::string toString() const;

private:

    AcceptorI(const AcceptorI&);
    void operator=(const AcceptorI&);

    AcceptorI(Instance, int);
    virtual ~AcceptorI();
    void destroy();
    friend class CollectorFactoryI; // May create and destroy AcceptorIs

    Instance instance_;
    int fd_;
    int backlog_;
    struct sockaddr_in addr_;
};

}

#endif

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
#include <Ice/TransceiverF.h>
#include <Ice/Shared.h>

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
    
private:

    AcceptorI(const AcceptorI&);
    void operator=(const AcceptorI&);

    AcceptorI(int);
    virtual ~AcceptorI();
    void destroy();
    friend class CollectorFactoryI; // May create and destroy AcceptorIs

    int fd_;
    int port_;
    int backlog_;
};

}

#endif

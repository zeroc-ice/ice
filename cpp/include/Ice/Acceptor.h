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
#include <Ice/TraceLevelsF.h>
#include <Ice/LoggerF.h>
#include <Ice/Shared.h>

#ifndef WIN32
#   include <netinet/in.h> // For struct sockaddr_in
#endif

namespace __Ice
{

class CollectorFactoryI;

class AcceptorI : public Shared
{
public:

    int fd();
    void close();
    void shutdown();
    void listen();
    Transceiver accept(int);
    std::string toString() const;

private:

    AcceptorI(const AcceptorI&);
    void operator=(const AcceptorI&);

    AcceptorI(Instance, int);
    virtual ~AcceptorI();
    friend class CollectorFactoryI; // May create AcceptorIs

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

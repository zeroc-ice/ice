// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_UDP_ACCEPTOR_H
#define ICE_UDP_ACCEPTOR_H

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

class UdpAcceptorI : public AcceptorI
{
public:

    int fd();
    void close();
    void shutdown();
    Transceiver initialize();
    Transceiver accept(int);
    std::string toString() const;

private:

    UdpAcceptorI(const UdpAcceptorI&);
    void operator=(const UdpAcceptorI&);

    UdpAcceptorI(Instance, int);
    virtual ~UdpAcceptorI();
    friend class CollectorFactoryI; // May create UdpAcceptorIs

    Instance instance_;
    struct sockaddr_in addr_;
#ifndef ICE_NO_TRACE
    TraceLevels traceLevels_;
    ::Ice::Logger logger_;
#endif
};

}

#endif

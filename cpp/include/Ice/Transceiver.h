// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_TRANSCEIVER_H
#define ICE_TRANSCEIVER_H

#include <Ice/TransceiverF.h>
#include <Ice/InstanceF.h>
#include <Ice/TraceLevelsF.h>
#include <Ice/LoggerF.h>
#include <Ice/Shared.h>

#ifndef WIN32
#   include <netinet/in.h> // For struct sockaddr_in
#endif

namespace __Ice
{

class Buffer;

class TransceiverI : public Shared
{
public:

    int fd();
    void close();
    void shutdown();
    void write(Buffer&);
    void read(Buffer&);
    std::string toString() const;
    
private:

    TransceiverI(const TransceiverI&);
    void operator=(const TransceiverI&);

    TransceiverI(Instance, int);
    virtual ~TransceiverI();
    friend class ConnectorI; // May create TransceiverIs
    friend class AcceptorI; // May create TransceiverIs

    Instance instance_;
    int fd_;
#ifndef ICE_NO_TRACE
    TraceLevels traceLevels_;
    ::Ice::Logger logger_;
#endif
};

}

#endif

// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_UDP_TRANSCEIVER_H
#define ICE_UDP_TRANSCEIVER_H

#include <Ice/InstanceF.h>
#include <Ice/TraceLevelsF.h>
#include <Ice/LoggerF.h>
#include <Ice/Transceiver.h>

#ifndef WIN32
#   include <netinet/in.h> // For struct sockaddr_in
#endif

namespace __Ice
{

class UdpEndpointI;

class UdpTransceiverI : public TransceiverI
{
public:

    virtual int fd();
    virtual void close();
    virtual void shutdown();
    virtual void write(Buffer&, int);
    virtual void read(Buffer&, int);
    virtual std::string toString() const;
    
private:

    UdpTransceiverI(Instance, const std::string&, int);
    UdpTransceiverI(Instance, int);
    virtual ~UdpTransceiverI();
    friend class UdpEndpointI;

    Instance instance_;
    bool sender_;
    int fd_;
    struct sockaddr_in addr_;
#ifndef ICE_NO_TRACE
    TraceLevels traceLevels_;
    ::Ice::Logger logger_;
#endif
};

}

#endif

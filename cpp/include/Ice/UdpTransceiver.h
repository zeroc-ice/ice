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

class UdpEndpoint;

class UdpTransceiver : public Transceiver
{
public:

    virtual int fd();
    virtual void close();
    virtual void shutdown();
    virtual void write(Buffer&, int);
    virtual void read(Buffer&, int);
    virtual std::string toString() const;

    virtual bool equivalent(const std::string&, int) const;
    
private:

    UdpTransceiver(Instance_ptr, const std::string&, int);
    UdpTransceiver(Instance_ptr, int);
    virtual ~UdpTransceiver();
    friend class UdpEndpoint;

    Instance_ptr instance_;
    bool sender_;
    int fd_;
    struct sockaddr_in addr_;
#ifndef ICE_NO_TRACE
    TraceLevels_ptr traceLevels_;
    ::Ice::Logger_ptr logger_;
#endif
};

}

#endif

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

namespace IceInternal
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
    
    int effectivePort();

private:

    UdpTransceiver(const InstancePtr&, const std::string&, int);
    UdpTransceiver(const InstancePtr&, int);
    virtual ~UdpTransceiver();
    friend class UdpEndpoint;

    InstancePtr _instance;
    TraceLevelsPtr _traceLevels;
    ::Ice::LoggerPtr _logger;
    bool _sender;
    int _fd;
    struct sockaddr_in _addr;
};

}

#endif

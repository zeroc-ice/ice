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

class SUdpTransceiver;

class UdpTransceiver : public Transceiver
{
public:

    virtual SOCKET fd();
    virtual void close();
    virtual void shutdown();
    virtual void write(Buffer&, int);
    virtual void read(Buffer&, int);
    virtual std::string toString() const;

    bool equivalent(const std::string&, int) const;
    int effectivePort();
    void setProtocolName(const std::string&);

private:

    UdpTransceiver(const InstancePtr&, const std::string&, int, const std::string& protocolName = "udp");
    UdpTransceiver(const InstancePtr&, const std::string&, int, bool, const std::string& protocolName = "udp");
    virtual ~UdpTransceiver();

    friend class UdpEndpoint;
//    friend class SUdpEndpoint; // TODO: Remove as soon as SUdpEndpoint uses SUdpTransceiver.
//    friend class SUdpTransceiver;

    InstancePtr _instance;
    TraceLevelsPtr _traceLevels;
    ::Ice::LoggerPtr _logger;
    bool _incoming;
    bool _connect;
    SOCKET _fd;
    struct sockaddr_in _addr;
    std::string _protocolName;
};

}

#endif

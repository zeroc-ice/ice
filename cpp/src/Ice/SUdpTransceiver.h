// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_SUDP_TRANSCEIVER_H
#define ICE_SUDP_TRANSCEIVER_H

#include <Ice/InstanceF.h>
#include <Ice/TraceLevelsF.h>
#include <Ice/LoggerF.h>
#include <Ice/SUdpControlChannelF.h>
#include <Ice/SecureUdpF.h>
#include <Ice/Stream.h>
#include <Ice/UdpTransceiver.h>
#include <Ice/SUdpTransceiverF.h>
#include <map>

#ifndef WIN32
#   include <netinet/in.h> // For struct sockaddr_in
#endif

namespace IceInternal
{

class SUdpEndpoint;

class SUdpTransceiver : public Transceiver
{
public:

    virtual SOCKET fd();
    virtual void close();
    virtual void shutdown();
    virtual void write(Buffer&, int);
    virtual void read(Buffer&, int);
    virtual std::string toString() const;

    virtual bool equivalent(const std::string&, int) const;

    int effectivePort();

    // Server Channel Implementation methods
    void clientHello(const IceSecurity::SecureUdp::ClientChannelPtr&, const Ice::ByteSeq&);
    void clientKeyAcknowledge(Ice::Long, Ice::Long, const Ice::ByteSeq&);
    void clientKeyRequest(Ice::Long);
    void clientGoodbye(Ice::Long);

    // Client Channel Implementation methods
    void serverHello(Ice::Long, const Ice::ByteSeq&);
    void serverKeyChange(const Ice::ByteSeq&);
    void serverGoodbye();

private:

    SUdpTransceiver(const InstancePtr&, const std::string&, int);
    SUdpTransceiver(const InstancePtr&, int, bool);
    virtual ~SUdpTransceiver();

    friend class SUdpEndpoint;

    void connectControlChannel(const std::string&, int);
    void createControlChannel(int);

    UdpTransceiver _udpTransceiver;
    IceSecurity::SecureUdp::ControlChannelPtr _controlChannel;

    InstancePtr _instance;
    TraceLevelsPtr _traceLevels;
    ::Ice::LoggerPtr _logger;
};

}

#endif


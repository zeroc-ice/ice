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

namespace __Ice
{

class UdpConnectorI;
class UdpAcceptorI;

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

    UdpTransceiverI(const UdpTransceiverI&);
    void operator=(const UdpTransceiverI&);

    UdpTransceiverI(Instance, int);
    virtual ~UdpTransceiverI();
    friend class UdpConnectorI; // May create UdpTransceiverIs
    friend class UdpAcceptorI; // May create UdpTransceiverIs

    Instance instance_;
    int fd_;
#ifndef ICE_NO_TRACE
    TraceLevels traceLevels_;
    ::Ice::Logger logger_;
#endif
};

}

#endif

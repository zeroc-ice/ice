// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_TCP_TRANSCEIVER_H
#define ICE_TCP_TRANSCEIVER_H

#include <Ice/InstanceF.h>
#include <Ice/TraceLevelsF.h>
#include <Ice/LoggerF.h>
#include <Ice/Transceiver.h>

namespace __Ice
{

class TcpConnectorI;
class TcpAcceptorI;

class TcpTransceiverI : public TransceiverI
{
public:

    virtual int fd();
    virtual void close();
    virtual void shutdown();
    virtual void write(Buffer&, int);
    virtual void read(Buffer&, int);
    virtual std::string toString() const;
    
private:

    TcpTransceiverI(Instance, int);
    virtual ~TcpTransceiverI();
    friend class TcpConnectorI;
    friend class TcpAcceptorI;

    Instance instance_;
    int fd_;
    fd_set rFdSet;
    fd_set wFdSet;
#ifndef ICE_NO_TRACE
    TraceLevels traceLevels_;
    ::Ice::Logger logger_;
#endif
};

}

#endif

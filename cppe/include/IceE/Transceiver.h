// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_TRANSPORT_TRANSCEIVER_H
#define ICEE_TRANSPORT_TRANSCEIVER_H

#include <IceE/TransceiverF.h>
#include <IceE/InstanceF.h>
#include <IceE/TraceLevelsF.h>
#include <IceE/LoggerF.h>
#include <IceE/Shared.h>

#ifdef _WIN32
typedef int ssize_t;
#else
#   define SOCKET int
#endif

namespace IceInternal
{

class Connector;
class Acceptor;
class Buffer;

class ICEE_API Transceiver : public ::Ice::Shared
{
public:

    SOCKET fd();
    void close();
    void shutdownWrite();
    void shutdownReadWrite();
    void write(Buffer&, int);
    void read(Buffer&, int);
    std::string type() const;
    std::string toString() const;

private:

    Transceiver(const InstancePtr&, SOCKET);
    virtual ~Transceiver();
    friend class Connector;
    friend class Acceptor;

    const TraceLevelsPtr _traceLevels;
    const Ice::LoggerPtr _logger;
    
    SOCKET _fd;
#ifdef _WIN32_WCE
    WSAEVENT _event;
#else
    fd_set _wFdSet;
    fd_set _rFdSet;
#endif

    const std::string _desc;
};

}

#endif

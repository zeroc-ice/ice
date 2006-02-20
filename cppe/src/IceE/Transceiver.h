// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_TRANSCEIVER_H
#define ICEE_TRANSCEIVER_H

#include <IceE/TransceiverF.h>
#include <IceE/InstanceF.h>
#include <IceE/TraceLevelsF.h>
#include <IceE/LoggerF.h>
#include <IceE/Shared.h>

#ifdef _WIN32
#   include <winsock2.h>
typedef int ssize_t;
#else
#   define SOCKET int
#endif

namespace IceInternal
{

class Connector;
class Acceptor;
class Buffer;

class ICE_API Transceiver : public ::IceUtil::Shared
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

    Transceiver(const InstancePtr&, SOCKET, int);
    virtual ~Transceiver();
    friend class Connector;
    friend class Acceptor;

#ifdef ICEE_USE_SELECT_FOR_TIMEOUTS
    void doSelect(bool, int);
#endif

    const TraceLevelsPtr _traceLevels;
    const Ice::LoggerPtr _logger;
    
    SOCKET _fd;
#ifdef ICEE_USE_SELECT_FOR_TIMEOUTS
#ifdef _WIN32
    WSAEVENT _event;
    WSAEVENT _readEvent;
    WSAEVENT _writeEvent;
#else
    fd_set _wFdSet;
    fd_set _rFdSet;
#endif
#else
    const int _timeout;
#endif

    const std::string _desc;
#ifdef _WIN32
    const bool _isPeerLocal;
#endif
};

}

#endif

// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
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

class Transceiver : public ::IceUtil::Shared
{
public:

    void setTimeouts(int, int);

    SOCKET fd();
    void close();
    void shutdownWrite();
    void shutdownReadWrite();
    void writeWithTimeout(Buffer&, int);
    void readWithTimeout(Buffer&, int);

    void write(Buffer& buf)
    {
	writeWithTimeout(buf, _writeTimeout);
    }

    void read(Buffer& buf)
    {
	readWithTimeout(buf, _readTimeout);
    }
    
    std::string type() const;
    std::string toString() const;

private:

    Transceiver(const InstancePtr&, SOCKET);
    virtual ~Transceiver();
    friend class Connector;
    friend class Acceptor;

#ifdef ICEE_USE_SELECT_OR_POLL_FOR_TIMEOUTS
    void doSelect(bool, int);
#endif

    const TraceLevelsPtr _traceLevels;
    const Ice::LoggerPtr _logger;
    
    SOCKET _fd;
    int _readTimeout;
    int _writeTimeout;

#ifdef ICEE_USE_SELECT_OR_POLL_FOR_TIMEOUTS
#ifdef _WIN32
    WSAEVENT _event;
    WSAEVENT _readEvent;
    WSAEVENT _writeEvent;
#else
    fd_set _wFdSet;
    fd_set _rFdSet;
#endif
#endif

    const std::string _desc;
#ifdef _WIN32
    int _maxPacketSize;
#endif
};

}

#endif

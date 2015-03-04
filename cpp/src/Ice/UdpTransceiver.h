// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_UDP_TRANSCEIVER_H
#define ICE_UDP_TRANSCEIVER_H

#include <Ice/InstanceF.h>
#include <Ice/TraceLevelsF.h>
#include <Ice/LoggerF.h>
#include <Ice/StatsF.h>
#include <Ice/Transceiver.h>
#include <Ice/Protocol.h>
#include <Ice/Network.h>
#include <IceUtil/Mutex.h>

#ifdef ICE_OS_WINRT
#   include <deque>
#endif

namespace IceInternal
{

class UdpEndpoint;

class UdpTransceiver : public Transceiver, public NativeInfo
{
    enum State
    {
        StateNeedConnect,
        StateConnectPending,
        StateConnected,
        StateNotConnected
    };

public:

    virtual NativeInfoPtr getNativeInfo();
#if defined(ICE_USE_IOCP)
    virtual AsyncInfo* getAsyncInfo(SocketOperation);
#elif defined(ICE_OS_WINRT)
    virtual void setCompletedHandler(SocketOperationCompletedHandler^);
#endif

    virtual SocketOperation initialize(Buffer&, Buffer&);
    virtual void close();
    virtual bool write(Buffer&);
    virtual bool read(Buffer&);
#if defined(ICE_USE_IOCP) || defined(ICE_OS_WINRT)
    virtual bool startWrite(Buffer&);
    virtual void finishWrite(Buffer&);
    virtual void startRead(Buffer&);
    virtual void finishRead(Buffer&);
#endif
    virtual std::string type() const;
    virtual std::string toString() const;
    virtual Ice::ConnectionInfoPtr getInfo() const;
    virtual void checkSendSize(const Buffer&, size_t);

    int effectivePort() const;

private:

    UdpTransceiver(const InstancePtr&, const Address&, const std::string&, int);
    UdpTransceiver(const InstancePtr&, const std::string&, int, const std::string&, bool);

    virtual ~UdpTransceiver();

    void setBufSize(const InstancePtr&);

#ifdef ICE_OS_WINRT
    bool checkIfErrorOrCompleted(SocketOperation, Windows::Foundation::IAsyncInfo^);
    void appendMessage(Windows::Networking::Sockets::DatagramSocketMessageReceivedEventArgs^);
    Windows::Networking::Sockets::DatagramSocketMessageReceivedEventArgs^ readMessage();
#endif

    friend class UdpEndpointI;
    friend class UdpConnector;

    const TraceLevelsPtr _traceLevels;
    const Ice::LoggerPtr _logger;
    const Ice::StatsPtr _stats;
    const bool _incoming;

    const Address _addr;
    Address _mcastAddr;
    Address _peerAddr;

    State _state;
    int _rcvSize;
    int _sndSize;
    static const int _udpOverhead;
    static const int _maxPacketSize;

#if defined(ICE_USE_IOCP)
    AsyncInfo _read;
    AsyncInfo _write;
    Address _readAddr;
    socklen_t _readAddrLen;
#elif defined(ICE_OS_WINRT)
    AsyncInfo _write;

    Windows::Storage::Streams::DataWriter^ _writer;

    SocketOperationCompletedHandler^ _completedHandler;
    Windows::Foundation::AsyncOperationCompletedHandler<unsigned int>^ _writeOperationCompletedHandler;

    IceUtil::Mutex _mutex;
    bool _readPending;
    std::deque<Windows::Networking::Sockets::DatagramSocketMessageReceivedEventArgs^> _received;
#endif
};

}
#endif

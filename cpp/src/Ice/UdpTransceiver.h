// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_UDP_TRANSCEIVER_H
#define ICE_UDP_TRANSCEIVER_H

#include <IceUtil/Mutex.h>

#include <Ice/ProtocolInstanceF.h>
#include <Ice/Transceiver.h>
#include <Ice/Network.h>

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

    virtual SocketOperation initialize(Buffer&, Buffer&, bool&);
    virtual SocketOperation closing(bool, const Ice::LocalException&);
    virtual void close();
    virtual EndpointIPtr bind();
    virtual SocketOperation write(Buffer&);
    virtual SocketOperation read(Buffer&, bool&);
#if defined(ICE_USE_IOCP) || defined(ICE_OS_WINRT)
    virtual bool startWrite(Buffer&);
    virtual void finishWrite(Buffer&);
    virtual void startRead(Buffer&);
    virtual void finishRead(Buffer&, bool&);
#endif
    virtual std::string protocol() const;
    virtual std::string toString() const;
    virtual std::string toDetailedString() const;
    virtual Ice::ConnectionInfoPtr getInfo() const;
    virtual void checkSendSize(const Buffer&);
    virtual void setBufferSize(int rcvSize, int sndSize);

    int effectivePort() const;

private:

    UdpTransceiver(const ProtocolInstancePtr&, const Address&, const Address&, const std::string&, int);
    UdpTransceiver(const UdpEndpointIPtr&, const ProtocolInstancePtr&, const std::string&, int, const std::string&,
                   bool);

    virtual ~UdpTransceiver();

    void setBufSize(int, int);

#ifdef ICE_OS_WINRT
    bool checkIfErrorOrCompleted(SocketOperation, Windows::Foundation::IAsyncInfo^);
    void appendMessage(Windows::Networking::Sockets::DatagramSocketMessageReceivedEventArgs^);
    Windows::Networking::Sockets::DatagramSocketMessageReceivedEventArgs^ readMessage();
#endif

    friend class UdpEndpointI;
    friend class UdpConnector;

    UdpEndpointIPtr _endpoint;
    const ProtocolInstancePtr _instance;
    const bool _incoming;
    bool _bound;

    const Address _addr;
    Address _mcastAddr;
    const std::string _mcastInterface;
    Address _peerAddr;
    int _port;

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

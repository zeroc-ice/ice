// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_IAP_TRANSCEIVER_H
#define ICE_IAP_TRANSCEIVER_H

#include <Ice/ProtocolInstanceF.h>
#include <Ice/Transceiver.h>
#include <Ice/Network.h>
#include <Ice/Selector.h>

#import <Foundation/Foundation.h>
#import <ExternalAccessory/ExternalAccessory.h>

@class iAPTransceiverCallback;

namespace IceObjC
{

class iAPTransceiver : public IceInternal::Transceiver, public IceInternal::StreamNativeInfo
{
    enum State
    {
        StateNeedConnect,
        StateConnectPending,
        StateConnected
    };

public:

    iAPTransceiver(const IceInternal::ProtocolInstancePtr&, EASession*);
    virtual ~iAPTransceiver();

    virtual void initStreams(IceInternal::SelectorReadyCallback*);
    virtual IceInternal::SocketOperation registerWithRunLoop(IceInternal::SocketOperation);
    virtual IceInternal::SocketOperation unregisterFromRunLoop(IceInternal::SocketOperation, bool);
    virtual void closeStreams();

    virtual IceInternal::NativeInfoPtr getNativeInfo();

    virtual IceInternal::SocketOperation initialize(IceInternal::Buffer&, IceInternal::Buffer&);

    virtual IceInternal::SocketOperation closing(bool, const Ice::LocalException&);
    virtual void close();
    virtual IceInternal::SocketOperation write(IceInternal::Buffer&);
    virtual IceInternal::SocketOperation read(IceInternal::Buffer&);

    virtual std::string protocol() const;
    virtual std::string toString() const;
    virtual std::string toDetailedString() const;
    virtual Ice::ConnectionInfoPtr getInfo() const;
    virtual void checkSendSize(const IceInternal::Buffer&);
    virtual void setBufferSize(int, int);

private:

    void checkErrorStatus(NSStream*, const char*, int);

    IceInternal::ProtocolInstancePtr _instance;
    EASession* _session;
    NSInputStream* _readStream;
    NSOutputStream* _writeStream;
    iAPTransceiverCallback* _callback;
    bool _readStreamRegistered;
    bool _writeStreamRegistered;
    bool _opening;

    IceUtil::Mutex _mutex;
    bool _error;

    State _state;
    std::string _desc;
};

}

#endif

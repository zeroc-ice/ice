//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

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
    class iAPTransceiver final : public IceInternal::Transceiver,
                                 public IceInternal::StreamNativeInfo,
                                 public std::enable_shared_from_this<iAPTransceiver>
    {
        enum State
        {
            StateNeedConnect,
            StateConnectPending,
            StateConnected
        };

    public:
        iAPTransceiver(const IceInternal::ProtocolInstancePtr&, EASession*);
        ~iAPTransceiver();

        void initStreams(IceInternal::SelectorReadyCallback*) final;
        IceInternal::SocketOperation registerWithRunLoop(IceInternal::SocketOperation) final;
        IceInternal::SocketOperation unregisterFromRunLoop(IceInternal::SocketOperation, bool) final;
        void closeStreams() final;

        IceInternal::NativeInfoPtr getNativeInfo() final;

        IceInternal::SocketOperation initialize(IceInternal::Buffer&, IceInternal::Buffer&) final;

        IceInternal::SocketOperation closing(bool, std::exception_ptr) final;
        void close() final;
        IceInternal::SocketOperation write(IceInternal::Buffer&) final;
        IceInternal::SocketOperation read(IceInternal::Buffer&) final;

        std::string protocol() const final;
        std::string toString() const final;
        std::string toDetailedString() const final;
        Ice::ConnectionInfoPtr getInfo() const final;
        void checkSendSize(const IceInternal::Buffer&) final;
        void setBufferSize(int, int) final;

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

        std::mutex _mutex;
        bool _error;

        State _state;
        std::string _desc;
    };
}

#endif

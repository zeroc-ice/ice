// Copyright (c) ZeroC, Inc.

#ifndef ICE_IAP_TRANSCEIVER_H
#define ICE_IAP_TRANSCEIVER_H

#include "../Network.h"
#include "../ProtocolInstanceF.h"
#include "../Transceiver.h"

#import <ExternalAccessory/ExternalAccessory.h>
#import <Foundation/Foundation.h>

#include <dispatch/dispatch.h>

#include <exception>
#include <memory>
#include <mutex>
#include <string>

@class iAPTransceiverCallback;

namespace IceObjC
{
    // Completion-based transceiver for the External Accessory streams, following the same model as the
    // Network.framework transceivers: the thread pool starts asynchronous read and write operations, which the
    // transceiver performs on a private dispatch queue where the NSStream events are delivered, and each
    // completion is posted through the NativeInfo.
    class iAPTransceiver final : public IceInternal::Transceiver
    {
    public:
        // State shared between the transceiver, the stream delegate and the blocks dispatched on the stream queue.
        // The delegate and the blocks only use this state, never the transceiver, so they can safely run after the
        // transceiver is destroyed.
        struct StreamState
        {
            std::mutex mutex;
            NSInputStream* readStream;
            NSOutputStream* writeStream;
            IceInternal::NativeInfoPtr nativeInfo;

            // Opening: the connect completes once both streams are open.
            bool connectPending{false};
            bool readOpen{false};
            bool writeOpen{false};
            std::exception_ptr connectError;

            // A write operation performs a single write call; the connection starts another operation for the
            // rest of the buffer.
            bool writePending{false};
            const std::byte* writeData{nullptr};
            size_t writeSize{0};
            size_t writeCount{0};
            std::exception_ptr writeError;

            // A read operation performs a single read call.
            bool readPending{false};
            std::byte* readData{nullptr};
            size_t readSize{0};
            size_t readCount{0};
            std::exception_ptr readError;

            // Set by a stream error, the end of the stream, or close(); pending operations complete with it.
            std::exception_ptr error;
        };

        iAPTransceiver(const IceInternal::ProtocolInstancePtr&, EASession*);
        // Test-only constructor that bypasses EASession. The caller retains ownership of the (unopened) streams
        // and must keep them alive for the lifetime of the transceiver.
        iAPTransceiver(const IceInternal::ProtocolInstancePtr&, NSInputStream*, NSOutputStream*, std::string desc);
        ~iAPTransceiver();

        IceInternal::NativeInfoPtr getNativeInfo() final;

        IceInternal::SocketOperation initialize(IceInternal::Buffer&, IceInternal::Buffer&) final;

        IceInternal::SocketOperation closing(bool, std::exception_ptr) final;
        void close() final;
        IceInternal::SocketOperation write(IceInternal::Buffer&) final;
        IceInternal::SocketOperation read(IceInternal::Buffer&) final;

        bool startWrite(IceInternal::Buffer&) final;
        void finishWrite(IceInternal::Buffer&) final;
        void startRead(IceInternal::Buffer&) final;
        void finishRead(IceInternal::Buffer&) final;

        std::string protocol() const final;
        std::string toString() const final;
        std::string toDetailedString() const final;
        Ice::ConnectionInfoPtr getInfo(bool incoming, std::string adapterName, std::string connectionId) const final;
        void checkSendSize(const IceInternal::Buffer&) final;
        void setBufferSize(int, int) final;

    private:
        enum State
        {
            StateNeedConnect,
            StateConnectPending,
            StateConnected
        };

        const IceInternal::ProtocolInstancePtr _instance;
        EASession* _session;
        NSInputStream* _readStream;
        NSOutputStream* _writeStream;
        iAPTransceiverCallback* _callback;
        dispatch_queue_t _queue;
        IceInternal::NativeInfoPtr _nativeInfo;
        std::shared_ptr<StreamState> _streamState;

        State _state;
        std::string _desc;
    };
}

#endif

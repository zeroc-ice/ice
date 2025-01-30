// Copyright (c) ZeroC, Inc.

#ifndef TEST_TRANSCEIVER_H
#define TEST_TRANSCEIVER_H

#include "Configuration.h"
#include "Ice/Buffer.h"
#include "Ice/Transceiver.h"

class Transceiver final : public IceInternal::Transceiver
{
public:
    Transceiver(IceInternal::TransceiverPtr);
    IceInternal::NativeInfoPtr getNativeInfo() final;

    IceInternal::SocketOperation closing(bool, std::exception_ptr) final;
    void close() override;
    IceInternal::SocketOperation write(IceInternal::Buffer&) final;
    IceInternal::SocketOperation read(IceInternal::Buffer&) final;
#ifdef ICE_USE_IOCP
    bool startWrite(IceInternal::Buffer&) final;
    void finishWrite(IceInternal::Buffer&) final;
    void startRead(IceInternal::Buffer&) final;
    void finishRead(IceInternal::Buffer&) final;
#endif
    [[nodiscard]] std::string protocol() const final;
    [[nodiscard]] std::string toString() const final;
    [[nodiscard]] std::string toDetailedString() const final;
    [[nodiscard]] Ice::ConnectionInfoPtr
    getInfo(bool incoming, std::string adapterName, std::string connectionId) const final;
    IceInternal::SocketOperation initialize(IceInternal::Buffer&, IceInternal::Buffer&) final;
    void checkSendSize(const IceInternal::Buffer&) final;
    void setBufferSize(int rcvSize, int sndSize) final;

    [[nodiscard]] IceInternal::TransceiverPtr delegate() const { return _transceiver; }

private:
    friend class Connector;
    friend class Acceptor;
    friend class EndpointI;

    const IceInternal::TransceiverPtr _transceiver;
    const ConfigurationPtr _configuration;
    bool _initialized{false};

    IceInternal::Buffer _readBuffer;
    IceInternal::Buffer::Container::const_iterator _readBufferPos;
    bool _buffered;
};

#endif

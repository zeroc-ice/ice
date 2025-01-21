// Copyright (c) ZeroC, Inc.

#ifndef TEST_ACCEPTOR_H
#define TEST_ACCEPTOR_H

#include "EndpointI.h"
#include "Ice/Acceptor.h"

class Acceptor : public IceInternal::Acceptor
{
public:
    Acceptor(EndpointIPtr, IceInternal::AcceptorPtr);
    IceInternal::NativeInfoPtr getNativeInfo() override;

    void close() override;
    IceInternal::EndpointIPtr listen() override;
#ifdef ICE_USE_IOCP
    virtual void startAccept();
    virtual void finishAccept();
#endif
    IceInternal::TransceiverPtr accept() override;
    [[nodiscard]] std::string protocol() const override;
    [[nodiscard]] std::string toString() const override;
    [[nodiscard]] std::string toDetailedString() const override;

    [[nodiscard]] IceInternal::AcceptorPtr delegate() const { return _acceptor; }

private:
    EndpointIPtr _endpoint;
    const IceInternal::AcceptorPtr _acceptor;
};

#endif

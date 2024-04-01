//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef TEST_ACCEPTOR_H
#define TEST_ACCEPTOR_H

#include "Ice/Acceptor.h"
#include <EndpointI.h>

class Acceptor : public IceInternal::Acceptor
{
public:
    Acceptor(const EndpointIPtr&, const IceInternal::AcceptorPtr&);
    virtual IceInternal::NativeInfoPtr getNativeInfo();

    virtual void close();
    virtual IceInternal::EndpointIPtr listen();
#ifdef ICE_USE_IOCP
    virtual void startAccept();
    virtual void finishAccept();
#endif
    virtual IceInternal::TransceiverPtr accept();
    virtual std::string protocol() const;
    virtual std::string toString() const;
    virtual std::string toDetailedString() const;

    IceInternal::AcceptorPtr delegate() const { return _acceptor; }

private:
    EndpointIPtr _endpoint;
    const IceInternal::AcceptorPtr _acceptor;
};

#endif

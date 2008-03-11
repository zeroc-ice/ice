// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef TEST_ACCEPTOR_H
#define TEST_ACCEPTOR_H

#include <Ice/Acceptor.h>

class Acceptor : public IceInternal::Acceptor
{
public:

    virtual SOCKET fd();
    virtual void close();
    virtual void listen();
    virtual IceInternal::TransceiverPtr accept();
    virtual std::string toString() const;

private:

    Acceptor(const IceInternal::AcceptorPtr&);
    friend class EndpointI;

    const IceInternal::AcceptorPtr _acceptor;
};

#endif

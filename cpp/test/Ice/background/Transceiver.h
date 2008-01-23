// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef TEST_TRANSCEIVER_H
#define TEST_TRANSCEIVER_H

#include <Ice/Transceiver.h>
#include <Configuration.h>

class Transceiver : public IceInternal::Transceiver
{
public:
    
    virtual SOCKET fd();
    virtual void close();
    virtual void shutdownWrite();
    virtual void shutdownReadWrite();
    virtual bool write(IceInternal::Buffer&, int);
    virtual bool read(IceInternal::Buffer&, int);
    virtual std::string type() const;
    virtual std::string toString() const;
    virtual IceInternal::SocketStatus initialize(int);
    virtual void checkSendSize(const IceInternal::Buffer&, size_t);

private:
    
    Transceiver(const IceInternal::TransceiverPtr&);
    friend class Connector;
    friend class Acceptor;
    friend class EndpointI;

    const IceInternal::TransceiverPtr _transceiver;
    const ConfigurationPtr _configuration;
    bool _initialized;
};

#endif

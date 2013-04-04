// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
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
    
    virtual IceInternal::NativeInfoPtr getNativeInfo();

    virtual void close();
    virtual bool write(IceInternal::Buffer&);
    virtual bool read(IceInternal::Buffer&);
#ifdef ICE_USE_IOCP
    virtual bool startWrite(IceInternal::Buffer&);
    virtual void finishWrite(IceInternal::Buffer&);
    virtual void startRead(IceInternal::Buffer&);
    virtual void finishRead(IceInternal::Buffer&);
#endif
    virtual std::string type() const;
    virtual std::string toString() const;
    virtual Ice::ConnectionInfoPtr getInfo() const;
    virtual IceInternal::SocketOperation initialize(IceInternal::Buffer&, IceInternal::Buffer&);
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

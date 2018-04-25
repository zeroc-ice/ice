// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef TEST_TRANSCEIVER_H
#define TEST_TRANSCEIVER_H

#include <Ice/Buffer.h>
#include <Ice/Transceiver.h>
#include <Configuration.h>

class Transceiver : public IceInternal::Transceiver
{
public:

    virtual IceInternal::NativeInfoPtr getNativeInfo();

    virtual IceInternal::SocketOperation closing(bool, const Ice::LocalException&);
    virtual void close();
    virtual IceInternal::SocketOperation write(IceInternal::Buffer&);
    virtual IceInternal::SocketOperation read(IceInternal::Buffer&);
#ifdef ICE_USE_IOCP
    virtual bool startWrite(IceInternal::Buffer&);
    virtual void finishWrite(IceInternal::Buffer&);
    virtual void startRead(IceInternal::Buffer&);
    virtual void finishRead(IceInternal::Buffer&);
#endif
    virtual std::string protocol() const;
    virtual std::string toString() const;
    virtual std::string toDetailedString() const;
    virtual Ice::ConnectionInfoPtr getInfo() const;
    virtual IceInternal::SocketOperation initialize(IceInternal::Buffer&, IceInternal::Buffer&);
    virtual void checkSendSize(const IceInternal::Buffer&);
    virtual void setBufferSize(int rcvSize, int sndSize);

    IceInternal::TransceiverPtr delegate() const { return _transceiver; }

private:

    Transceiver(const IceInternal::TransceiverPtr&);
    friend class Connector;
    friend class Acceptor;
    friend class EndpointI;

    const IceInternal::TransceiverPtr _transceiver;
    const ConfigurationPtr _configuration;
    bool _initialized;

    IceInternal::Buffer _readBuffer;
    IceInternal::Buffer::Container::const_iterator _readBufferPos;
    bool _buffered;
};

#endif

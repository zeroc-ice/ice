// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#ifndef ICE_STREAM_ACCEPTOR_H
#define ICE_STREAM_ACCEPTOR_H

#include <Ice/TransceiverF.h>
#include <Ice/Acceptor.h>

namespace IceObjC
{

class StreamEndpointI;
#ifdef ICE_CPP11_MAPPING // C++11 mapping
typedef ::std::shared_ptr<StreamEndpointI> StreamEndpointIPtr;
#else
typedef IceUtil::Handle<StreamEndpointI> StreamEndpointIPtr;
#endif

class StreamAcceptor : public IceInternal::Acceptor, public IceInternal::NativeInfo
{
public:

    virtual IceInternal::NativeInfoPtr getNativeInfo();
    virtual void close();
    virtual IceInternal::EndpointIPtr listen();
    virtual IceInternal::TransceiverPtr accept();
    virtual std::string protocol() const;
    virtual std::string toString() const;
    virtual std::string toDetailedString() const;

    int effectivePort() const;

private:

    StreamAcceptor(const StreamEndpointIPtr&, const InstancePtr&, const std::string&, int);
    virtual ~StreamAcceptor();
    friend class StreamEndpointI;

    StreamEndpointIPtr _endpoint;
    InstancePtr _instance;
    int _backlog;
    IceInternal::Address _addr;
};

}

#endif

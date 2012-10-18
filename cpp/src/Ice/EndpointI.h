// **********************************************************************
//
// Copyright (c) 2003-2012 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_ENDPOINT_I_H
#define ICE_ENDPOINT_I_H

#include <IceUtil/Config.h>
#include <IceUtil/Shared.h>
#include <IceUtil/Thread.h>
#include <IceUtil/Monitor.h>
#include <Ice/Endpoint.h>
#include <Ice/EndpointIF.h>
#include <Ice/InstanceF.h>
#include <Ice/TransceiverF.h>
#include <Ice/ConnectorF.h>
#include <Ice/AcceptorF.h>
#include <Ice/Protocol.h>
#include <Ice/Network.h>
#include <Ice/ObserverHelper.h>

#ifndef ICE_OS_WINRT
#   include <deque>
#endif

namespace IceInternal
{

class BasicStream;

class ICE_API EndpointI_connectors : public virtual IceUtil::Shared
{
public:

    virtual ~EndpointI_connectors() { }

    virtual void connectors(const std::vector<ConnectorPtr>&) = 0;
    virtual void exception(const Ice::LocalException&) = 0;
};
typedef IceUtil::Handle<EndpointI_connectors> EndpointI_connectorsPtr;

class ICE_API EndpointI : public Ice::Endpoint
{
public:

    //
    // Marshal the endpoint.
    //
    virtual void streamWrite(BasicStream*) const = 0;

    //
    // Return the endpoint type.
    //
    virtual Ice::Short type() const = 0;

    //
    // Return the protocol name
    //
    virtual std::string protocol() const = 0;
    
    //
    // Return the timeout for the endpoint in milliseconds. 0 means
    // non-blocking, -1 means no timeout.
    //
    virtual Ice::Int timeout() const = 0;

    //
    // Return a new endpoint with a different timeout value, provided
    // that timeouts are supported by the endpoint. Otherwise the same
    // endpoint is returned.
    //
    virtual EndpointIPtr timeout(Ice::Int) const = 0;

    //
    // Return a new endpoint with a different connection id.
    //
    virtual EndpointIPtr connectionId(const ::std::string&) const = 0;

    //
    // Return true if the endpoints support bzip2 compress, or false
    // otherwise.
    //
    virtual bool compress() const = 0;

    //
    // Return a new endpoint with a different compression value,
    // provided that compression is supported by the
    // endpoint. Otherwise the same endpoint is returned.
    //
    virtual EndpointIPtr compress(bool) const = 0;

    //
    // Return true if the endpoint is datagram-based.
    //
    virtual bool datagram() const = 0;

    //
    // Return true if the endpoint is secure.
    //
    virtual bool secure() const = 0;

    //
    // Returns the encoding version supported by this endpoint.
    //
    const ::Ice::EncodingVersion& encodingVersion() const;

    //
    // Returns the encoding version supported by this endpoint.
    //
    const ::Ice::ProtocolVersion& protocolVersion() const;

    //
    // Return a server side transceiver for this endpoint, or null if a
    // transceiver can only be created by an acceptor. In case a
    // transceiver is created, this operation also returns a new
    // "effective" endpoint, which might differ from this endpoint,
    // for example, if a dynamic port number is assigned.
    //
    virtual TransceiverPtr transceiver(EndpointIPtr&) const = 0;

    //
    // Return connectors for this endpoint, or empty vector if no 
    // connector is available.
    //
    virtual std::vector<ConnectorPtr> connectors() const = 0;
    virtual void connectors_async(const EndpointI_connectorsPtr&) const = 0;

    //
    // Return an acceptor for this endpoint, or null if no acceptors
    // is available. In case an acceptor is created, this operation
    // also returns a new "effective" endpoint, which might differ
    // from this endpoint, for example, if a dynamic port number is
    // assigned.
    //
    virtual AcceptorPtr acceptor(EndpointIPtr&, const std::string&) const = 0;

    //
    // Expand endpoint out in to separate endpoints for each local
    // host if listening on INADDR_ANY on server side.
    //
    virtual std::vector<EndpointIPtr> expand() const = 0;

    //
    // Check whether the endpoint is equivalent to another one.
    //
    virtual bool equivalent(const EndpointIPtr&) const = 0;
    
    //
    // Compare endpoints for sorting purposes.
    //
    virtual bool operator==(const Ice::LocalObject&) const = 0;
    virtual bool operator<(const Ice::LocalObject&) const = 0;

    const std::string& connectionId() const;
    
protected:
    
    virtual ::Ice::Int internal_getHash() const;

    virtual std::vector<ConnectorPtr> connectors(const std::vector<Address>&) const;
    friend class EndpointHostResolver;

    EndpointI(const Ice::ProtocolVersion&, const Ice::EncodingVersion&, const std::string&);
    EndpointI();

    void parseOption(const std::string&, const std::string&, const std::string&, const std::string&);

    virtual ::Ice::Int hashInit() const = 0;

    const Ice::ProtocolVersion _protocol;
    const Ice::EncodingVersion _encoding;
    const std::string _connectionId;

private:

    mutable bool _hashInitialized;
    mutable Ice::Int _hashValue;
};

inline bool operator==(const EndpointI& l, const EndpointI& r)
{
    return static_cast<const ::Ice::LocalObject&>(l) == static_cast<const ::Ice::LocalObject&>(r);
}

inline bool operator<(const EndpointI& l, const EndpointI& r)
{
    return static_cast<const ::Ice::LocalObject&>(l) < static_cast<const ::Ice::LocalObject&>(r);
}

#ifndef ICE_OS_WINRT
class ICE_API EndpointHostResolver : public IceUtil::Thread, public IceUtil::Monitor<IceUtil::Mutex>
#else
class ICE_API EndpointHostResolver : public IceUtil::Shared
#endif
{
public:

    EndpointHostResolver(const InstancePtr&);

    std::vector<ConnectorPtr> resolve(const std::string&, int, const EndpointIPtr&);
    void resolve(const std::string&, int, const EndpointIPtr&, const EndpointI_connectorsPtr&);
    void destroy();

    virtual void run();
    void updateObserver();

private:

#ifndef ICE_OS_WINRT
    struct ResolveEntry
    {
        std::string host;
        int port;
        EndpointIPtr endpoint;
        EndpointI_connectorsPtr callback;
        Ice::Instrumentation::ObserverPtr observer;
    };

    const InstancePtr _instance;
    bool _destroyed;
    std::deque<ResolveEntry> _queue;
    ObserverHelperT<Ice::Instrumentation::ThreadObserver> _observer;
#else
    const InstancePtr _instance;
#endif
};

}

#endif

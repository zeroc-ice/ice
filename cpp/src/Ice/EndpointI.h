// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_ENDPOINT_I_H
#define ICE_ENDPOINT_I_H

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

#ifdef _WIN32
#   include <winsock2.h>
#else
#   include <sys/socket.h> // For struct sockaddr_storage
#endif

#include <deque>

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

class ICE_API EndpointI : public virtual Ice::Endpoint
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
    virtual bool operator==(const EndpointI&) const = 0;
    virtual bool operator!=(const EndpointI&) const = 0;
    virtual bool operator<(const EndpointI&) const = 0;

#if defined(__BCPLUSPLUS__)
    //
    // COMPILERFIX: Avoid warnings about hiding members for C++Builder 2010
    //
    //
    virtual bool operator==(const Ice::LocalObject& rhs) const
    {
        return Ice::LocalObject::operator==(rhs);
    }

    virtual bool operator<(const Ice::LocalObject& rhs) const
    {
        return Ice::LocalObject::operator<(rhs);
    }
#endif

protected:

    virtual std::vector<ConnectorPtr> connectors(const std::vector<struct sockaddr_storage>&) const;
    friend class EndpointHostResolver;

private:

#if defined(__SUNPRO_CC)
    //
    // COMPILERFIX: prevent the compiler from emitting a warning about
    // hidding these operators.
    //
    using LocalObject::operator==;
    using LocalObject::operator<;
#endif
};

class ICE_API EndpointHostResolver : public IceUtil::Thread, public IceUtil::Monitor<IceUtil::Mutex>
{
public:

    EndpointHostResolver(const InstancePtr&);

    void resolve(const std::string&, int, const EndpointIPtr&, const EndpointI_connectorsPtr&);
    void destroy();

    virtual void run();

private:

    struct ResolveEntry
    {
        std::string host;
        int port;
        EndpointIPtr endpoint;
        EndpointI_connectorsPtr callback;
    };

    const InstancePtr _instance;
    bool _destroyed;
    std::deque<ResolveEntry> _queue;
};

}

#endif

//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_ENDPOINT_I_H
#define ICE_ENDPOINT_I_H

#include <Ice/EndpointIF.h>
#include <Ice/Endpoint.h>
#include <Ice/EndpointTypes.h>
#include <Ice/TransceiverF.h>
#include <Ice/ConnectorF.h>
#include <Ice/AcceptorF.h>

namespace Ice
{

class OutputStream;
class InputStream;

}

namespace IceInternal
{

class ICE_API EndpointI_connectors
#ifndef ICE_CPP11_MAPPING
    : public virtual IceUtil::Shared
#endif
{
public:

    virtual ~EndpointI_connectors();

    virtual void connectors(const std::vector<ConnectorPtr>&) = 0;
    virtual void exception(const Ice::LocalException&) = 0;
};

class ICE_API EndpointI : public Ice::Endpoint
{
public:

    //
    // Marshal the endpoint.
    //
    virtual void streamWrite(Ice::OutputStream*) const;
    virtual void streamWriteImpl(Ice::OutputStream*) const = 0;

    //
    // Return the endpoint type.
    //
    virtual Ice::Short type() const = 0;

    //
    // Return the protocol name
    //
    virtual const std::string& protocol() const = 0;

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
    // Returns the endpoint connection id.
    //
    virtual const std::string& connectionId() const = 0;

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
    // transceiver can only be created by an acceptor.
    //
    virtual TransceiverPtr transceiver() const = 0;

    //
    // Return connectors for this endpoint, or empty vector if no
    // connector is available. Implementation is responsible for
    // returning connectors sorted according to the endpoint selection
    // type.
    //
    virtual void connectors_async(Ice::EndpointSelectionType, const EndpointI_connectorsPtr&) const = 0;

    //
    // Return an acceptor for this endpoint, or null if no acceptors
    // is available.
    //
    virtual AcceptorPtr acceptor(const std::string&) const = 0;

    //
    // Expand endpoint out into separate endpoints for each local
    // host if listening on INADDR_ANY on server side.
    //
    virtual std::vector<EndpointIPtr> expandIfWildcard() const = 0;

    //
    // Expand endpoint out into separate endpoints for each IP
    // address returned by the DNS resolver. Also returns the
    // endpoint which can be used to connect to the returned
    // endpoints or null if no specific endpoint can be used to
    // connect to these endpoints (e.g.: with the IP endpoint,
    // it returns this endpoint if it uses a fixed port, null
    // otherwise).
    //
    virtual std::vector<EndpointIPtr> expandHost(IceInternal::EndpointIPtr&) const = 0;

    //
    // Check whether the endpoint is equivalent to another one.
    //
    virtual bool equivalent(const EndpointIPtr&) const = 0;

    //
    // Compare endpoints for sorting purposes.
    //
#ifndef ICE_CPP11_MAPPING
    virtual bool operator==(const Ice::LocalObject&) const = 0;
    virtual bool operator<(const Ice::LocalObject&) const = 0;
#endif

    virtual ::Ice::Int hash() const = 0;

    //
    // Returns the stringified options
    //
    virtual std::string options() const = 0;

    virtual std::string toString() const ICE_NOEXCEPT;
    void initWithOptions(std::vector<std::string>&);

protected:

    virtual bool checkOption(const std::string&, const std::string&, const std::string&);

};

#ifndef ICE_CPP11_MAPPING
inline bool operator==(const EndpointI& l, const EndpointI& r)
{
    return static_cast<const ::Ice::LocalObject&>(l) == static_cast<const ::Ice::LocalObject&>(r);
}

inline bool operator<(const EndpointI& l, const EndpointI& r)
{
    return static_cast<const ::Ice::LocalObject&>(l) < static_cast<const ::Ice::LocalObject&>(r);
}
#endif

template<typename T> class InfoI : public T
{
public:

    InfoI(const EndpointIPtr& endpoint) : _endpoint(endpoint)
    {
        T::compress = _endpoint->compress();
        T::timeout = _endpoint->timeout();
    }

    virtual Ice::Short
    type() const ICE_NOEXCEPT
    {
        return _endpoint->type();
    }

    virtual bool
    datagram() const ICE_NOEXCEPT
    {
        return _endpoint->datagram();
    }

    virtual bool
    secure() const ICE_NOEXCEPT
    {
        return _endpoint->secure();
    }

private:

    const EndpointIPtr _endpoint;
};

}

#endif

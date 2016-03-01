// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_NETWORK_PROXY_H
#define ICE_NETWORK_PROXY_H

#include <Ice/Network.h>

namespace IceInternal
{

class ICE_API NetworkProxy : virtual public IceUtil::Shared
{
public:

    //
    // Write the connection request on the connection established
    // with the network proxy server. This is called right after
    // the connection establishment succeeds.
    //
    virtual void beginWrite(const Address&, Buffer&) = 0;
    virtual SocketOperation endWrite(Buffer&) = 0;

    //
    // Once the connection request has been sent, this is called
    // to prepare and read the response from the proxy server.
    //
    virtual void beginRead(Buffer&) = 0;
    virtual SocketOperation endRead(Buffer&) = 0;

    //
    // This is called when the response from the proxy has been
    // read. The proxy should copy the extra read data (if any) in the
    // given byte vector.
    //
    virtual void finish(Buffer&, Buffer&) = 0;

    //
    // If the proxy host needs to be resolved, this should return
    // a new NetworkProxy containing the IP address of the proxy.
    // This is called from the endpoint host resolver thread, so
    // it's safe if this this method blocks.
    //
    virtual NetworkProxyPtr resolveHost(ProtocolSupport) const = 0;

    //
    // Returns the IP address of the network proxy. This method
    // must not block. It's only called on a network proxy object
    // returned by resolveHost().
    //
    virtual Address getAddress() const = 0;

    //
    // Returns the name of the proxy, used for tracing purposes.
    //
    virtual std::string getName() const = 0;

    //
    // Returns the protocols supported by the proxy.
    //
    virtual ProtocolSupport getProtocolSupport() const = 0;
};

NetworkProxyPtr createNetworkProxy(const Ice::PropertiesPtr&, ProtocolSupport);

}

#endif

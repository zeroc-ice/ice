// Copyright (c) ZeroC, Inc.

#ifndef ICE_NETWORK_PROXY_H
#define ICE_NETWORK_PROXY_H

#include "Ice/Buffer.h"
#include "Network.h"

namespace IceInternal
{
    class ICE_API NetworkProxy
    {
    public:
        virtual ~NetworkProxy();

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
        [[nodiscard]] virtual NetworkProxyPtr resolveHost(ProtocolSupport) const = 0;

        //
        // Returns the IP address of the network proxy. This method
        // must not block. It's only called on a network proxy object
        // returned by resolveHost().
        //
        [[nodiscard]] virtual Address getAddress() const = 0;

        //
        // Returns the name of the proxy, used for tracing purposes.
        //
        [[nodiscard]] virtual std::string getName() const = 0;

        //
        // Returns the protocols supported by the proxy.
        //
        [[nodiscard]] virtual ProtocolSupport getProtocolSupport() const = 0;
    };

    NetworkProxyPtr createNetworkProxy(const Ice::PropertiesPtr&, ProtocolSupport);
}

#endif

// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public interface NetworkProxy
{
    //
    // Write the connection request on the connection established
    // with the network proxy server. This is called right after
    // the connection establishment succeeds.
    //
    void beginWriteConnectRequest(java.net.InetSocketAddress endpoint, Buffer buf);
    void endWriteConnectRequest(Buffer buf);

    //
    // Once the connection request has been sent, this is called
    // to prepare and read the response from the proxy server.
    //
    void beginReadConnectRequestResponse(Buffer buf);
    void endReadConnectRequestResponse(Buffer buf);

    //
    // If the proxy host needs to be resolved, this should return
    // a new NetworkProxy containing the IP address of the proxy.
    // This is called from the endpoint host resolver thread, so
    // it's safe if this this method blocks.
    //
    NetworkProxy resolveHost();

    //
    // Returns the IP address of the network proxy. This method
    // must not block. It's only called on a network proxy object
    // returned by resolveHost().
    //
    java.net.InetSocketAddress getAddress();

    //
    // Returns the name of the proxy, used for tracing purposes.
    //
    String getName();
}

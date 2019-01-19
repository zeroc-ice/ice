//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package IceInternal;

public interface NetworkProxy
{
    //
    // Write the connection request on the connection established
    // with the network proxy server. This is called right after
    // the connection establishment succeeds.
    //
    void beginWrite(java.net.InetSocketAddress endpoint, Buffer buf);
    int endWrite(Buffer buf);

    //
    // Once the connection request has been sent, this is called
    // to prepare and read the response from the proxy server.
    //
    void beginRead(Buffer buf);
    int endRead(Buffer buf);

    //
    // This is called when the response from the proxy has been
    // read. The proxy should copy the extra read data (if any) in the
    // given byte vector.
    //
    void finish(Buffer readBuffer, Buffer writeBuffer);

    //
    // If the proxy host needs to be resolved, this should return
    // a new NetworkProxy containing the IP address of the proxy.
    // This is called from the endpoint host resolver thread, so
    // it's safe if this this method blocks.
    //
    NetworkProxy resolveHost(int protocolSupport);

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

    //
    // Returns the protocols supported by the proxy.
    //
    int getProtocolSupport();
}

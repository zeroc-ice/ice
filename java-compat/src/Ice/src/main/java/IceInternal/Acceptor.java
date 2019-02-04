//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package IceInternal;

public interface Acceptor
{
    java.nio.channels.ServerSocketChannel fd();
    void setReadyCallback(ReadyCallback callback);
    void close();
    EndpointI listen();
    Transceiver accept();
    String protocol();
    @Override
    String toString();
    String toDetailedString();
}

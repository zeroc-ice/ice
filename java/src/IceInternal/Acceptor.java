// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

package IceInternal;

public interface Acceptor
{
    java.nio.channels.ServerSocketChannel fd();
    void close();
    void listen();
    Transceiver accept(int timeout);
    String toString();
}

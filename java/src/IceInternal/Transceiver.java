// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

package IceInternal;

public interface Transceiver
{
    java.nio.channels.SelectableChannel fd();
    void close();
    void shutdown();
    void write(java.nio.ByteBuffer buffer, int timeout);
    void read(java.nio.ByteBuffer buffer, int timeout);
    String toString();
}

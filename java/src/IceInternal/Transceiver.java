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
    void write(BasicStream stream, int timeout);
    boolean tryRead(BasicStream stream);
    void read(BasicStream stream, int timeout);
    String toString();
}

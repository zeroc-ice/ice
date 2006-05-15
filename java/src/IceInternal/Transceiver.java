// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public interface Transceiver
{
    java.nio.channels.SelectableChannel fd();
    void close();
    void shutdownWrite();
    void shutdownReadWrite();
    void write(BasicStream stream, int timeout);
    //
    // NOTE: In Java, read() returns a boolean to indicate whether the transceiver
    //       has read more data than requested.
    //
    boolean read(BasicStream stream, int timeout);
    String type();
    String toString();
}

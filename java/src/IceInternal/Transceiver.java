// **********************************************************************
//
// Copyright (c) 2003 - 2004
// ZeroC, Inc.
// North Palm Beach, FL, USA
//
// All Rights Reserved.
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
    void shutdown();
    void write(BasicStream stream, int timeout);
    void read(BasicStream stream, int timeout);
    String toString();
}

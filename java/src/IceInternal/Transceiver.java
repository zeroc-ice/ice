// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
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

// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public interface Transceiver
{
    void close();
    void shutdownWrite();
    void shutdownReadWrite();
    void write(BasicStream stream, int timeout);
    void read(BasicStream stream, int timeout);
    String type();
    String toString();
}

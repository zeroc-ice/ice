// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
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
    //
    // NOTE: In Java, write() can raise LocalExceptionWrapper to indicate that
    //       retrying may not be safe, which is necessary to address an issue
    //       in the IceSSL implementation for JDK 1.4. We can remove this if
    //       we ever drop support for JDK 1.4 (also see Ice.ConnectionI).
    //
    void write(BasicStream stream, int timeout)
        throws LocalExceptionWrapper;
    //
    // NOTE: In Java, read() returns a boolean to indicate whether the transceiver
    //       has read more data than requested.
    //
    boolean read(BasicStream stream, int timeout);
    String type();
    String toString();
    void checkSendSize(BasicStream stream, int messageSizeMax);
}

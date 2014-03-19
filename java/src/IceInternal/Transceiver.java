// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public interface Transceiver
{
    java.nio.channels.SelectableChannel fd();

    //
    // Initialize the transceiver.
    //
    // Returns the status if the initialize operation.
    //
    int initialize(Buffer readBuffer, Buffer writeBuffer);

    void close();

    //
    // Write data.
    //
    // Returns true if all the data was written, false otherwise.
    //
    boolean write(Buffer buf);

    //
    // Read data.
    //
    // Returns true if all the requested data was read, false otherwise.
    //
    // NOTE: In Java, read() returns a boolean in moreData to indicate
    //       whether the transceiver has read more data than requested.
    //       If moreData is true, read should be called again without
    //       calling select on the FD.
    //
    boolean read(Buffer buf, Ice.BooleanHolder moreData);

    String type();
    String toString();
    Ice.ConnectionInfo getInfo();
    void checkSendSize(Buffer buf, int messageSizeMax);
}

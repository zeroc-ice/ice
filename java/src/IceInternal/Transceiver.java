// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
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
    // Returns the status if the initialize operation. If timeout is != 0,
    // the status will always be SocketStatus.Finished. If timeout is 0, 
    // the operation won't block and will return SocketStatus.NeedRead or
    // SocketStatus.NeedWrite if the initialization couldn't be completed
    // without blocking. This operation should be called again once the 
    // socket is ready for reading or writing and until it returns 
    // SocketStatus.Finished.
    //
    SocketStatus initialize();

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
    void checkSendSize(Buffer buf, int messageSizeMax);
}

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

    int initialize(Buffer readBuffer, Buffer writeBuffer, Ice.BooleanHolder moreData);
    int closing(boolean initiator, Ice.LocalException ex);
    void close();

    int write(Buffer buf);
    int read(Buffer buf, Ice.BooleanHolder moreData);

    String protocol();
    String toString();
    Ice.ConnectionInfo getInfo();
    void checkSendSize(Buffer buf, int messageSizeMax);
}

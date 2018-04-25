// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.IceInternal;

public interface Transceiver
{
    java.nio.channels.SelectableChannel fd();
    void setReadyCallback(ReadyCallback callback);

    int initialize(Buffer readBuffer, Buffer writeBuffer);
    int closing(boolean initiator, com.zeroc.Ice.LocalException ex);
    void close();

    EndpointI bind();
    int write(Buffer buf);
    int read(Buffer buf);

    String protocol();
    @Override
    String toString();
    String toDetailedString();
    com.zeroc.Ice.ConnectionInfo getInfo();
    void checkSendSize(Buffer buf);
    void setBufferSize(int rcvSize, int sndSize);
}

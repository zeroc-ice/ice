// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public interface Transceiver
{
    java.nio.channels.SelectableChannel fd();

    int initialize(Buffer readBuffer, Buffer writeBuffer, Ice.Holder<Boolean> moreData);
    int closing(boolean initiator, Ice.LocalException ex);
    void close();

    EndpointI bind();
    int write(Buffer buf);
    int read(Buffer buf, Ice.Holder<Boolean> moreData);

    String protocol();
    @Override
    String toString();
    String toDetailedString();
    Ice.ConnectionInfo getInfo();
    void checkSendSize(Buffer buf);
    void setBufferSize(int rcvSize, int sndSize);
}

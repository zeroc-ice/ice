// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.IceInternal;

public interface Acceptor
{
    java.nio.channels.ServerSocketChannel fd();
    void setReadyCallback(ReadyCallback callback);
    void close();
    EndpointI listen();
    Transceiver accept();
    String protocol();
    @Override
    String toString();
    String toDetailedString();
}

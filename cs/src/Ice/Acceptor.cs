// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace IceInternal
{

    using System;
    using System.Net.Sockets;

    public interface Acceptor
    {
        Socket fd();
        void close();
        void listen();
        Transceiver accept(int timeout);
        IAsyncResult beginAccept(AsyncCallback callback, object state);
        Transceiver endAccept(IAsyncResult result);
        void connectToSelf();
        string ToString();
    }

}

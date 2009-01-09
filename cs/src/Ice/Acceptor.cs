// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
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
        void close();
        void listen();
        IAsyncResult beginAccept(AsyncCallback callback, object state);
        Transceiver endAccept(IAsyncResult result);
        string ToString();
    }

}

// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace IceInternal
{

    using System.Net.Sockets;

    public interface Acceptor
    {
        Socket fd();
        void close();
        void listen();
        Transceiver accept(int timeout);
        void connectToSelf();
        string ToString();
    }

}

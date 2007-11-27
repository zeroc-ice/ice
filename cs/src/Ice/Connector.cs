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

    public interface Connector
    {
        //
        // Blocking connect. The caller must initialize the new
        // transceiver.
        //
        Transceiver connect(int timeout);

        short type();

        int CompareTo(object obj);
    }

}

// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace IceInternal
{
    public interface Connector
    {
        //
        // Create a transceiver without blocking. The transceiver may not be fully connected
        // until its initialize method is called.
        //
        Transceiver connect();

        short type();
    }

}

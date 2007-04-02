// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

public class ServerMIDlet extends ServerBase
{
    public void
    updateProperties(Ice.Properties properties)
    {
        properties.setProperty("TestAdapter.Endpoints", 
                "default -p " + _port.getString() + " -t " + _timeout.getString());
    }
}


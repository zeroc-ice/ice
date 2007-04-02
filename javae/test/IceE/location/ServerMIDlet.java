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
    protected String
    endpointPropertyName()
    {
        return "ServerManagerAdapter.Endpoints";
    }

    public void
    updateProperties(Ice.Properties properties)
    {
        properties.setProperty("Test.ServerManager", 
                "ServerManager:default -p " + _port.getString() + " -t " + _timeout.getString());
        properties.setProperty("ServerManagerAdapter.Endpoints", 
                "default -p " + _port.getString() + " -t " + _timeout.getString());
    }
}


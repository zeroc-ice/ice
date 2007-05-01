// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

public class ClientMIDlet extends ClientBase
{
    protected String
    testProxyPropertyName()
    {
        return "Ice.Default.Locator";
    }

    public void
    updateProperties(Ice.Properties properties)
    {
        properties.setProperty("Ice.Default.Locator", 
                "locator: default -h " + _addr.getString() + " -p " + _port.getString() + " -t " + 
                _timeout.getString());
        properties.setProperty("Test.ServerManager", 
                "ServerManager:default -h " + _addr.getString() + " -p " + _port.getString() + " -t " +
                _timeout.getString());
    }
}

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
    public void
    updateProperties(Ice.Properties properties)
    {
        String ref = "retry:default -h " + _addr.getString() + " -p " + _port.getString() + " -t " + 
            _timeout.getString();
        properties.setProperty("Retry.Proxy", ref);
    }
}

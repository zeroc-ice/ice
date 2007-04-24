// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

import javax.microedition.lcdui.*;

public class ClientMIDlet extends ClientBase
{
    protected ConfigurationForm
    initConfigurationForm(javax.microedition.midlet.MIDlet parent, Ice.Properties properties)
    {
        _properties = properties;
        ConfigurationForm cf = new ConfigurationForm(parent, properties);
        _addr = new TextField("Addr: ", properties.getPropertyWithDefault("Test.Host", "127.0.0.1"), 128, 
                TextField.ANY);
        _port = new TextField("Port: ", properties.getPropertyWithDefault("Test.FirstPort", "12010"), 128, 
                TextField.NUMERIC);

        String count = properties.getPropertyWithDefault("Test.ServerCount", "13");
        _serverCount = new TextField("Server count:", count.trim(), 128, TextField.NUMERIC);

        cf.append(_addr);
        cf.append(_port);
        cf.append(_serverCount);
        return cf;
    }

    public void
    updateProperties(Ice.Properties properties)
    {
        properties.setProperty("Test.FirstPort", _port.getString());
        properties.setProperty("Test.ServerCount", _serverCount.getString());
        properties.setProperty("Test.Host", _addr.getString());
    }

    private TextField _serverCount;
}

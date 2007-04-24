// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

import javax.microedition.lcdui.*;

abstract public class ServerBase extends TestApplication
{
    protected String
    endpointPropertyName()
    {
        return "TestAdapter.Endpoints";
    }

    protected ConfigurationForm 
    initConfigurationForm(javax.microedition.midlet.MIDlet parent, Ice.Properties properties)
    {
        _properties = properties;
        ConfigurationForm cf = new ConfigurationForm(parent, properties);
        ProxyStringHelper p = new ProxyStringHelper(properties.getProperty(endpointPropertyName()));
        _host = new StringItem("My IP: ", "");
        _port = new TextField("Port", p.port(), 128, TextField.NUMERIC);
        _timeout = new TextField("Timeout", p.timeout(), 128, TextField.NUMERIC);
        cf.append(_host);
        cf.append(_port);
        cf.append(_timeout);
        return cf;
    }

    public void
    setup()
    {
        String host = getHost();
        _host.setText(host);
        _configForm.setHost(host);
        _configForm.enableOk();
    }

    public void
    runTest(Ice.Communicator communicator, java.io.PrintStream ps)
    {
	try
	{
	    if(communicator != null)
	    {
                Ice.InitializationData data = new Ice.InitializationData();
                data.properties = _properties;
		Server.run(new String[0], communicator, data, ps);
	    }
            ps.println("Done");
	}
	catch(Exception ex)
	{
	    ps.println(ex.toString());
	    ex.printStackTrace();
	}
    }

    protected StringItem _host;
    protected TextField _port;
    protected TextField _timeout;
    protected Ice.Properties _properties;
}

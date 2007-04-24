// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

import javax.microedition.lcdui.*;

abstract public class ClientBase 
    extends TestApplication 
{
    protected String
    testProxyPropertyName()
    {
        return "Test.Proxy";
    }

    protected ConfigurationForm
    initConfigurationForm(javax.microedition.midlet.MIDlet parent, Ice.Properties properties)
    {
        _properties = properties;
        ConfigurationForm cf = new ConfigurationForm(parent, properties);

        String proxyString = properties.getProperty(testProxyPropertyName());
        ProxyStringHelper p = new ProxyStringHelper(proxyString);
        _addr = new TextField("Addr: ", p.host(), 128, TextField.ANY);
        _port = new TextField("Port: ", p.port(), 128, TextField.NUMERIC);
        _timeout = new TextField("Timeout: ", p.timeout(), 128, TextField.NUMERIC);
        cf.append(_addr);
        cf.append(_port);
        cf.append(_timeout);
        return cf;
    }

    public void
    setup()
    {
        _configForm.setHost(getHost());
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
		Client.run(new String[0], communicator, data, ps);
	    }
            ps.println("Done!");
	}
	catch(Exception ex)
	{
	    ps.println(ex.toString());
	    ex.printStackTrace();
	}
    }

    protected TextField _addr;
    protected TextField _port;
    protected TextField _timeout;
    protected Ice.Properties _properties;
}

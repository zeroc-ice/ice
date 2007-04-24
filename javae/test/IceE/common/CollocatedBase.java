// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

import javax.microedition.lcdui.*;

abstract public class CollocatedBase extends TestApplication
{
    protected ConfigurationForm 
    initConfigurationForm(javax.microedition.midlet.MIDlet parent, Ice.Properties properties)
    {
        ConfigurationForm cf = new ConfigurationForm(parent, properties);
        _host = new StringItem("My IP:", "");
        cf.append(_host); 
        cf.append(new StringItem("Just press OK to continue", ""));
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
		Collocated.run(new String[0], communicator, null, ps); 
	    }
            ps.println("Done");
	}
	catch(Exception ex)
	{
	    ps.println(ex.toString());
	    ex.printStackTrace();
	}
    }

    StringItem _host;
}

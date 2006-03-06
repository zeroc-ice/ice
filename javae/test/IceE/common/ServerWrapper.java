// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

public class ServerWrapper 
    extends TestApplication
    implements Runnable
{
    protected void
    initIce()
    {
	Ice.Properties properties = Ice.Util.createProperties();
	java.io.InputStream is = getClass().getResourceAsStream("config");
	if(is != null)
	{
	    properties.load(is);
	}

	try
	{
	    _hostnameConnection = 
		(javax.microedition.io.ServerSocketConnection)javax.microedition.io.Connector.open("socket://");
	    properties.setProperty("Ice.Default.Host", _hostnameConnection.getLocalAddress());
	    _out.println("Running on " + _hostnameConnection.getLocalAddress());
	}
	catch(Exception ex)
	{
	    _out.println("Unable to set default host");
	}

	_communicator = Ice.Util.initializeWithProperties(new String[0], properties);
    }

    protected void
    startApp()
	throws javax.microedition.midlet.MIDletStateChangeException
    {
	super.startApp();
	new Thread(this).start();
    }
    
    public void
    run()
    {
	try
	{
	    if(_communicator != null)
	    {
		Server.run(new String[0], _communicator, _out);
	    }
	    done();
	}
	catch(Exception ex)
	{
	    _out.println(ex.toString());
	    ex.printStackTrace();
	}
    }

    protected javax.microedition.io.ServerSocketConnection _hostnameConnection;  
}

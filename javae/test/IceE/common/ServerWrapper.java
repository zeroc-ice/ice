// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

public class ServerWrapper 
    extends TestApplication
    implements Runnable
{
    //
    // Find the IP address once we've connected to the network and display it.
    // 
    class FindHostname 
	extends Thread
    {
	public void
	run()
	{
	    String s = System.getProperty("microedition.hostname");
	    while(s == null || s.length() == 0)
	    {
		try
		{
		    sleep(5000);
	       	}
		catch(InterruptedException ex)
		{
		}
		s = System.getProperty("microedition.hostname");
	    }
	    _out.println("Running with IP " +  s);
	}
    }

    protected void
    startApp()
	throws javax.microedition.midlet.MIDletStateChangeException
    {
	super.startApp();
	new Thread(this).start();
	new FindHostname().start();
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
}

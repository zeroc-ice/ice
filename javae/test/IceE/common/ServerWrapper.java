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
	    message("Running...");
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

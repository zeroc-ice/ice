// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

public class ClientWrapper 
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
	    if(_communicator != null)
	    {
		Client.run(new String[0], _communicator, _out);
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

// **********************************************************************
//
// Copyright (c) 2002
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

package Ice;

public class ShutdownHook extends Thread
{
    ShutdownHook(Thread threadToJoin)
    {
	_threadToJoin = threadToJoin;
	_shutdown = false;
    }

    public void
    run()
    {
        try
        {
	    _shutdown = true;
	    Ice.Application.communicator().shutdown();
	    _threadToJoin.join();
        }
        catch(Exception ex)
        {
            ex.printStackTrace();
        }
    }

    public boolean
    isShutdownFromInterrupt()
    {
	return _shutdown;
    }

    private Thread _threadToJoin;
    private boolean _shutdown;
}

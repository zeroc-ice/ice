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
    }

    public void
    run()
    {
        try
        {
	    Ice.Application.communicator().shutdown();

	    _threadToJoin.join();
        }
        catch(Exception ex)
        {
            ex.printStackTrace();
        }
    }

    Thread _threadToJoin;
}

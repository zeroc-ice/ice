// **********************************************************************
//
// Copyright (c) 2002
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

package Freeze;

public class ShutdownHook extends Thread
{
    ShutdownHook(DBEnvironment env)
    {
        _env = env;
    }

    public void
    run()
    {
        try
        {
            _env.close();
        }
        catch(Exception ex)
        {
            ex.printStackTrace();
        }
    }

    private DBEnvironment _env;
}

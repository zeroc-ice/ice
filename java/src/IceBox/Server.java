// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

package IceBox;

public final class Server
{
    public static void
    main(String[] args)
    {
        Ice.Communicator communicator = null;
        int status = 0;

        Ice.Util.addArgumentPrefix("IceBox");

        try
        {
            Ice.StringSeqHolder argsH = new Ice.StringSeqHolder(args);
            communicator = Ice.Util.initialize(argsH);
            ServiceManagerI serviceManagerImpl = new ServiceManagerI(communicator, argsH.value);
            status = serviceManagerImpl.run();
        }
        catch (Ice.LocalException ex)
        {
            ex.printStackTrace();
            status = 1;
        }

        if (communicator != null)
        {
            try
            {
                communicator.destroy();
            }
            catch (Ice.LocalException ex)
            {
                ex.printStackTrace();
                status = 1;
            }
        }

        System.exit(status);
    }
}

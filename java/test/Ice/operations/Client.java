// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

public class Client
{
    private static int
    run(String[] args, Ice.Communicator communicator)
    {
        Test.MyClassPrx myClass = AllTests.allTests(communicator);

        System.out.print("testing server shutdown... ");
        System.out.flush();
        myClass.shutdown();
        try
        {
            myClass.opVoid();
            throw new RuntimeException();
        }
        catch(Ice.CloseConnectionException ex)
        {
            System.out.println("ok");
        }
        catch(Ice.ConnectFailedException ex)
        {
            System.out.println("ok");
        }
        catch(Ice.ConnectionLostException ex)
        {
            System.out.println("ok");
        }

        return 0;
    }

    public static void
    main(String[] args)
    {
        int status = 0;
        Ice.Communicator communicator = null;

        try
        {
            communicator = Ice.Util.initialize(args);
            status = run(args, communicator);
        }
        catch(Ice.LocalException ex)
        {
            ex.printStackTrace();
            status = 1;
        }

        if(communicator != null)
        {
            try
            {
                communicator.destroy();
            }
            catch(Ice.LocalException ex)
            {
                ex.printStackTrace();
                status = 1;
            }
        }

        System.exit(status);
    }
}

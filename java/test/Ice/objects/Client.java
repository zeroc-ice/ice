// **********************************************************************
//
// Copyright (c) 2003 - 2004
// ZeroC, Inc.
// North Palm Beach, FL, USA
//
// All Rights Reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************


public class Client
{
    private static class MyObjectFactory extends Ice.LocalObjectImpl implements Ice.ObjectFactory
    {
        public Ice.Object
        create(String type)
        {
            if(type.equals("::B"))
            {
                return new BI();
            }
            else if(type.equals("::C"))
            {
                return new CI();
            }
            else if(type.equals("::D"))
            {
                return new DI();
            }
            assert(false); // Should never be reached
            return null;
        }

        public void
        destroy()
        {
            // Nothing to do
        }
    }

    private static int
    run(String[] args, Ice.Communicator communicator)
    {
        Ice.ObjectFactory factory = new MyObjectFactory();
        communicator.addObjectFactory(factory, "::B");
        communicator.addObjectFactory(factory, "::C");
        communicator.addObjectFactory(factory, "::D");

        InitialPrx initial = AllTests.allTests(communicator, false);
        initial.shutdown();
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

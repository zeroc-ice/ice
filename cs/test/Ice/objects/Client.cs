// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System.Diagnostics;

public class Client
{
    private class MyObjectFactory : Ice.LocalObjectImpl, Ice.ObjectFactory
    {
        public Ice.Object create(string type)
        {
            if(type.Equals("::B"))
            {
                return new BI();
            }
            else if(type.Equals("::C"))
            {
                return new CI();
            }
            else if(type.Equals("::D"))
            {
                return new DI();
            }
            Debug.Assert(false); // Should never be reached
            return null;
        }

        public void
        destroy()
        {
            // Nothing to do
        }
    }

    private static int run(string[] args, Ice.Communicator communicator)
    {
	Ice.ObjectFactory factory = new MyObjectFactory();
	communicator.addObjectFactory(factory, "::B");
	communicator.addObjectFactory(factory, "::C");
	communicator.addObjectFactory(factory, "::D");

        InitialPrx initial = AllTests.allTests(communicator, false);
        initial.shutdown();
        return 0;
    }
    
    public static void Main(string[] args)
    {
        int status = 0;
        Ice.Communicator communicator = null;
        
        try
        {
            communicator = Ice.Util.initialize(ref args);
            status = run(args, communicator);
        }
        catch(System.Exception ex)
        {
            System.Console.Error.WriteLine(ex);
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
                System.Console.Error.WriteLine(ex);
                status = 1;
            }
        }
        
        System.Environment.Exit(status);
    }
}

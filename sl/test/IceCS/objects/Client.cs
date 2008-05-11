// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using Test;
using System.Diagnostics;

public class Client
{
    private class MyObjectFactory : Ice.ObjectFactory
    {
        public Ice.Object create(string type)
        {
            if(type.Equals("::Test::B"))
            {
                return new BI();
            }
            else if(type.Equals("::Test::C"))
            {
                return new CI();
            }
            else if(type.Equals("::Test::D"))
            {
                return new DI();
            }
            else if(type.Equals("::Test::E"))
            {
                return new EI();
            }
            else if(type.Equals("::Test::F"))
            {
                return new FI();
            }
            else if(type.Equals("::Test::H"))
            {
                return new HI();
            }
            else if(type.Equals("::Test::I"))
            {
                return new II();
            }
            else if(type.Equals("::Test::J"))
            {
                return new JI();
            }
	    	System.Console.WriteLine(type);
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
        communicator.addObjectFactory(factory, "::Test::B");
        communicator.addObjectFactory(factory, "::Test::C");
        communicator.addObjectFactory(factory, "::Test::D");
        communicator.addObjectFactory(factory, "::Test::E");
        communicator.addObjectFactory(factory, "::Test::F");
        communicator.addObjectFactory(factory, "::Test::I");
        communicator.addObjectFactory(factory, "::Test::J");
        communicator.addObjectFactory(factory, "::Test::H");

        InitialPrx initial = AllTests.allTests(communicator, false);
        initial.shutdown();
        return 0;
    }
    
    public static void Main(string[] args)
    {
        int status = 0;
        Ice.Communicator communicator = null;

#if !SILVERLIGHT
        Debug.Listeners.Add(new ConsoleTraceListener());
#endif

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
        
#if !SILVERLIGHT
        if(status != 0)
        {
            System.Environment.Exit(status);
        }
#endif
    }
}

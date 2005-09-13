// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using Demo;

public class Client
{
    public static int run(string[] args, Ice.Communicator communicator)
    {
        HelloPrx hello = HelloPrxHelper.checkedCast(communicator.stringToProxy("hello:tcp -p 10000"));
        if(hello == null)
        {
            Console.Error.WriteLine("invalid proxy");
            return 1;
        }
        
	hello.sayHello();
        
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
            Console.Error.WriteLine(ex);
            status = 1;
        }

        if(communicator != null)
        {
            try
            {
                communicator.destroy();
            }
            catch(System.Exception ex)
            {
                Console.Error.WriteLine(ex);
                status = 1;
            }
        }

        System.Environment.Exit(status);
    }
}

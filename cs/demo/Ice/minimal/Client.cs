// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using Demo;

public class Client
{
    public static void Main(string[] args)
    {
        int status = 0;
        Ice.Communicator communicator = null;

        try
        {
            communicator = Ice.Util.initialize(ref args);
            HelloPrx hello = HelloPrxHelper.checkedCast(communicator.stringToProxy("hello:tcp -p 10000"));
            if(hello == null)
            {
                Console.Error.WriteLine("invalid proxy");
                status = 1;
            }
            else
            {
                hello.sayHello();
            }
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

        if(status != 0)
        {
            System.Environment.Exit(status);
        }
    }
}

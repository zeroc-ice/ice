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

public class Client : Ice.Application
{
    public override int run(string[] args)
    {
        if(args.Length > 0)
        {
            Console.Error.WriteLine(appName() + ": too many arguments");
            return 1;
        }

        HelloPrx hello = HelloPrxHelper.uncheckedCast(communicator().propertyToProxy("Hello.Proxy"));
        if(hello == null)
        {
            Console.Error.WriteLine("Hello.Proxy not found");
            return 1;
        }

        hello.sayHello();

        return 0;
    }

    public static void Main(string[] args)
    {
        Client app = new Client();
        int status = app.main(args, "config.client");
        if(status != 0)
        {
            System.Environment.Exit(status);
        }
    }
}

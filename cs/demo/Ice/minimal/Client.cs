// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using Demo;

public class Client
{
    public static int Main(string[] args)
    {
        try
        {
            using(Ice.Communicator communicator = Ice.Util.initialize(ref args))
            {
                HelloPrx hello = HelloPrxHelper.checkedCast(
                    communicator.stringToProxy("hello:default -h localhost -p 10000"));
                hello.sayHello();
            }
            return 0;
        }
        catch(System.Exception ex)
        {
            System.Console.Error.WriteLine(ex);
            return 1;
        }
    }
}

// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using Test;

public class AllTests
{
    private static void test(bool b)
    {
        if(!b)
        {
            throw new Exception();
        }
    }

    public static void allTests(Ice.Communicator communicator)
    {
        Console.Out.Write("testing stringToProxy... ");
        Console.Out.Flush();
        String @ref = "hold:default -p 12010 -t 10000";
        Ice.ObjectPrx @base = communicator.stringToProxy(@ref);
        test(@base != null);
        Console.Out.WriteLine("ok");
        
        Console.Out.Write("testing checked cast... ");
        Console.Out.Flush();
        HoldPrx hold = HoldPrxHelper.checkedCast(@base);
        test(hold != null);
        test(hold.Equals(@base));
        Console.Out.WriteLine("ok");
        
        Console.Out.Write("changing state between active and hold rapidly... ");
        Console.Out.Flush();
        for(int i = 0; i < 100; ++i)
        {
            hold.putOnHold(0);
        }
        Console.Out.WriteLine("ok");
        
        Console.Out.Write("changing state to hold and shutting down server... ");
        Console.Out.Flush();
        hold.shutdown();
        Console.Out.WriteLine("ok");
    }
}
        

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


using System;

public class AllTests
{
    private static void test(bool b)
    {
        if (!b)
        {
            throw new System.Exception();
        }
    }
    
    public static TestPrx allTests(Ice.Communicator communicator)
    {
        Console.Out.Write("testing stringToProxy... ");
        Console.Out.Flush();
        System.String ref_Renamed = "test:default -p 12345 -t 2000";
        Ice.ObjectPrx base_Renamed = communicator.stringToProxy(ref_Renamed);
        test(base_Renamed != null);
        Console.Out.WriteLine("ok");
        
        Console.Out.Write("testing checked cast... ");
        Console.Out.Flush();
        TestPrx obj = TestPrxHelper.checkedCast(base_Renamed);
        test(obj != null);
        test(obj.Equals(base_Renamed));
        Console.Out.WriteLine("ok");
        
        Console.Out.Write("deactivating object adapter in the server... ");
        Console.Out.Flush();
        obj.deactivate();
        Console.Out.WriteLine("ok");
        
        return obj;
    }
}

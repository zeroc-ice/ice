// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
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
        if(!b)
        {
            throw new System.Exception();
        }
    }

    public static void
    allTests(Ice.Communicator communicator)
    {
        Ice.ObjectAdapter oa = communicator.createObjectAdapterWithEndpoints("MyOA", "tcp -h localhost");
        oa.activate();

        Ice.Object servant = new MyObjectI();

        //
        // Register default servant with category "foo"
        //
        oa.addDefaultServant(servant, "foo");

        //
        // Start test
        //
        Console.Out.Write("testing single category... ");
        Console.Out.Flush();

        Ice.Object r = oa.findDefaultServant("foo");
        test(r == servant);

        r = oa.findDefaultServant("bar");
        test(r == null);

        Ice.Identity identity = new Ice.Identity();
        identity.category = "foo";

        string[] names = new string[] { "foo", "bar", "x", "y", "abcdefg" };

        Test.MyObjectPrx prx = null;
        for(int idx = 0; idx < 5; ++idx)
        {
            identity.name = names[idx];
            prx = Test.MyObjectPrxHelper.uncheckedCast(oa.createProxy(identity));
            prx.ice_ping();
            test(prx.getName() == names[idx]);
        }

        identity.name = "ObjectNotExist";
        prx = Test.MyObjectPrxHelper.uncheckedCast(oa.createProxy(identity));
        try
        {
            prx.ice_ping();
            test(false);
        }
        catch(Ice.ObjectNotExistException)
        {
            // Expected
        }

        try
        {
            prx.getName();
            test(false);
        }
        catch(Ice.ObjectNotExistException)
        {
            // Expected
        }

        identity.name = "FacetNotExist";
        prx = Test.MyObjectPrxHelper.uncheckedCast(oa.createProxy(identity));
        try
        {
            prx.ice_ping();
            test(false);
        }
        catch(Ice.FacetNotExistException)
        {
            // Expected
        }

        try
        {
            prx.getName();
            test(false);
        }
        catch(Ice.FacetNotExistException)
        {
            // Expected
        }

        identity.category = "bar";
        for(int idx = 0; idx < 5; idx++)
        {
            identity.name = names[idx];
            prx = Test.MyObjectPrxHelper.uncheckedCast(oa.createProxy(identity));

            try
            {
                prx.ice_ping();
                test(false);
            }
            catch(Ice.ObjectNotExistException)
            {
                // Expected
            }

            try
            {
                prx.getName();
                test(false);
            }
            catch(Ice.ObjectNotExistException)
            {
                // Expected
            }
        }

        oa.removeDefaultServant("foo");
        identity.category = "foo";
        prx = Test.MyObjectPrxHelper.uncheckedCast(oa.createProxy(identity));
        try
        {
            prx.ice_ping();
        }
        catch(Ice.ObjectNotExistException)
        {
            // Expected
        }

        Console.Out.WriteLine("ok");

        Console.Out.Write("testing default category... ");
        Console.Out.Flush();

        oa.addDefaultServant(servant, "");

        r = oa.findDefaultServant("bar");
        test(r == null);

        r = oa.findDefaultServant("");
        test(r == servant);

        for(int idx = 0; idx < 5; ++idx)
        {
            identity.name = names[idx];
            prx = Test.MyObjectPrxHelper.uncheckedCast(oa.createProxy(identity));
            prx.ice_ping();
            test(prx.getName() == names[idx]);
        }

        Console.Out.WriteLine("ok");
    }
}

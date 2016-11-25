// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.defaultServant;

import java.io.PrintWriter;
import test.Ice.defaultServant.Test.MyObjectPrx;

public class AllTests
{
    private static void test(boolean b)
    {
        if(!b)
        {
            throw new RuntimeException();
        }
    }

    public static void allTests(test.Util.Application app)
    {
        com.zeroc.Ice.Communicator communicator = app.communicator();
        PrintWriter out = app.getWriter();

        com.zeroc.Ice.ObjectAdapter oa = communicator.createObjectAdapterWithEndpoints("MyOA", "tcp -h localhost");
        oa.activate();

        com.zeroc.Ice.Object servant = new MyObjectI();

        //
        // Register default servant with category "foo"
        //
        oa.addDefaultServant(servant, "foo");

        //
        // Start test
        //
        out.print("testing single category... ");
        out.flush();

        com.zeroc.Ice.Object r = oa.findDefaultServant("foo");
        test(r == servant);

        r = oa.findDefaultServant("bar");
        test(r == null);

        com.zeroc.Ice.Identity identity = new com.zeroc.Ice.Identity();
        identity.category = "foo";

        String names[] = { "foo", "bar", "x", "y", "abcdefg" };

        for(int idx = 0; idx < 5; ++idx)
        {
            identity.name = names[idx];
            MyObjectPrx prx = MyObjectPrx.uncheckedCast(oa.createProxy(identity));
            prx.ice_ping();
            test(prx.getName().equals(names[idx]));
        }

        identity.name = "ObjectNotExist";
        MyObjectPrx prx = MyObjectPrx.uncheckedCast(oa.createProxy(identity));
        try
        {
            prx.ice_ping();
            test(false);
        }
        catch(com.zeroc.Ice.ObjectNotExistException ex)
        {
            // Expected
        }

        try
        {
            prx.getName();
            test(false);
        }
        catch(com.zeroc.Ice.ObjectNotExistException ex)
        {
            // Expected
        }

        identity.name = "FacetNotExist";
        prx = MyObjectPrx.uncheckedCast(oa.createProxy(identity));
        try
        {
            prx.ice_ping();
            test(false);
        }
        catch(com.zeroc.Ice.FacetNotExistException ex)
        {
            // Expected
        }

        try
        {
            prx.getName();
            test(false);
        }
        catch(com.zeroc.Ice.FacetNotExistException ex)
        {
            // Expected
        }

        identity.category = "bar";
        for(int idx = 0; idx < 5; idx++)
        {
            identity.name = names[idx];
            prx = MyObjectPrx.uncheckedCast(oa.createProxy(identity));

            try
            {
                prx.ice_ping();
                test(false);
            }
            catch(com.zeroc.Ice.ObjectNotExistException ex)
            {
                // Expected
            }

            try
            {
                prx.getName();
                test(false);
            }
            catch(com.zeroc.Ice.ObjectNotExistException ex)
            {
                // Expected
            }
        }

        oa.removeDefaultServant("foo");
        identity.category = "foo";
        prx = MyObjectPrx.uncheckedCast(oa.createProxy(identity));
        try
        {
            prx.ice_ping();
        }
        catch(com.zeroc.Ice.ObjectNotExistException ex)
        {
            // Expected
        }

        out.println("ok");

        out.print("testing default category... ");
        out.flush();

        oa.addDefaultServant(servant, "");

        r = oa.findDefaultServant("bar");
        test(r == null);

        r = oa.findDefaultServant("");
        test(r == servant);

        for(int idx = 0; idx < 5; ++idx)
        {
            identity.name = names[idx];
            prx = MyObjectPrx.uncheckedCast(oa.createProxy(identity));
            prx.ice_ping();
            test(prx.getName().equals(names[idx]));
        }

        out.println("ok");
    }
}

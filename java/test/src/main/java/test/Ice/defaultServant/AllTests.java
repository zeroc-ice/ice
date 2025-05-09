// Copyright (c) ZeroC, Inc.

package test.Ice.defaultServant;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.FacetNotExistException;
import com.zeroc.Ice.Identity;
import com.zeroc.Ice.Object;
import com.zeroc.Ice.ObjectAdapter;
import com.zeroc.Ice.ObjectNotExistException;

import test.Ice.defaultServant.Test.MyObjectPrx;
import test.TestHelper;

import java.io.PrintWriter;

public class AllTests {
    private static void test(boolean b) {
        if (!b) {
            throw new RuntimeException();
        }
    }

    public static void allTests(TestHelper helper) {
        Communicator communicator = helper.communicator();
        PrintWriter out = helper.getWriter();

        ObjectAdapter oa =
            communicator.createObjectAdapterWithEndpoints("MyOA", "tcp -h localhost");
        oa.activate();

        Object servant = new MyObjectI();

        //
        // Register default servant with category "foo"
        //
        oa.addDefaultServant(servant, "foo");

        //
        // Start test
        //
        out.print("testing single category... ");
        out.flush();

        Object r = oa.findDefaultServant("foo");
        test(r == servant);

        r = oa.findDefaultServant("bar");
        test(r == null);

        Identity identity = new Identity();
        identity.category = "foo";

        String[] names = {"foo", "bar", "x", "y", "abcdefg"};

        for (int idx = 0; idx < 5; idx++) {
            identity.name = names[idx];
            MyObjectPrx prx = MyObjectPrx.uncheckedCast(oa.createProxy(identity));
            prx.ice_ping();
            test(prx.getName().equals(names[idx]));
        }

        identity.name = "ObjectNotExist";
        MyObjectPrx prx = MyObjectPrx.uncheckedCast(oa.createProxy(identity));
        try {
            prx.ice_ping();
            test(false);
        } catch (ObjectNotExistException ex) {
            // Expected
        }

        try {
            prx.getName();
            test(false);
        } catch (ObjectNotExistException ex) {
            // Expected
        }

        identity.name = "FacetNotExist";
        prx = MyObjectPrx.uncheckedCast(oa.createProxy(identity));
        try {
            prx.ice_ping();
            test(false);
        } catch (FacetNotExistException ex) {
            // Expected
        }

        try {
            prx.getName();
            test(false);
        } catch (FacetNotExistException ex) {
            // Expected
        }

        identity.category = "bar";
        for (int idx = 0; idx < 5; idx++) {
            identity.name = names[idx];
            prx = MyObjectPrx.uncheckedCast(oa.createProxy(identity));

            try {
                prx.ice_ping();
                test(false);
            } catch (ObjectNotExistException ex) {
                // Expected
            }

            try {
                prx.getName();
                test(false);
            } catch (ObjectNotExistException ex) {
                // Expected
            }
        }

        oa.removeDefaultServant("foo");
        identity.category = "foo";
        prx = MyObjectPrx.uncheckedCast(oa.createProxy(identity));
        try {
            prx.ice_ping();
        } catch (ObjectNotExistException ex) {
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

        for (int idx = 0; idx < 5; idx++) {
            identity.name = names[idx];
            prx = MyObjectPrx.uncheckedCast(oa.createProxy(identity));
            prx.ice_ping();
            test(prx.getName().equals(names[idx]));
        }

        out.println("ok");
    }

    private AllTests() {}
}

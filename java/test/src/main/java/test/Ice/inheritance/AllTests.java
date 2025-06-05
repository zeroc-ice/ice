// Copyright (c) ZeroC, Inc.

package test.Ice.inheritance;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.ObjectPrx;

import test.Ice.inheritance.Test.InitialPrx;
import test.Ice.inheritance.Test.MA.IAPrx;
import test.Ice.inheritance.Test.MA.ICPrx;
import test.Ice.inheritance.Test.MB.IB1Prx;
import test.Ice.inheritance.Test.MB.IB2Prx;
import test.TestHelper;

import java.io.PrintWriter;

public class AllTests {
    private static void test(boolean b) {
        if (!b) {
            throw new RuntimeException();
        }
    }

    public static InitialPrx allTests(TestHelper helper) {
        Communicator communicator = helper.communicator();
        PrintWriter out = helper.getWriter();
        out.print("testing stringToProxy... ");
        out.flush();
        String ref = "initial:" + helper.getTestEndpoint(0);
        ObjectPrx base = communicator.stringToProxy(ref);
        test(base != null);
        out.println("ok");

        out.print("testing checked cast... ");
        out.flush();
        InitialPrx initial = InitialPrx.checkedCast(base);
        test(initial != null);
        test(initial.equals(base));
        out.println("ok");

        out.print("getting proxies for interface hierarchy... ");
        out.flush();
        IAPrx ia = initial.iaop();
        IB1Prx ib1 = initial.ib1op();
        IB2Prx ib2 = initial.ib2op();
        ICPrx ic = initial.icop();
        test(ia != ib1);
        test(ia != ib2);
        test(ia != ic);
        test(ib1 != ic);
        test(ib2 != ic);
        out.println("ok");

        out.print("invoking proxy operations on interface hierarchy... ");
        out.flush();

        IAPrx iao = ia.iaop(ia);
        test(iao.equals(ia));
        iao = ia.iaop(ib1);
        test(iao.equals(ib1));
        iao = ia.iaop(ib2);
        test(iao.equals(ib2));
        iao = ia.iaop(ic);
        test(iao.equals(ic));
        iao = ib1.iaop(ia);
        test(iao.equals(ia));
        iao = ib1.iaop(ib1);
        test(iao.equals(ib1));
        iao = ib1.iaop(ib2);
        test(iao.equals(ib2));
        iao = ib1.iaop(ic);
        test(iao.equals(ic));
        iao = ib2.iaop(ia);
        test(iao.equals(ia));
        iao = ib2.iaop(ib1);
        test(iao.equals(ib1));
        iao = ib2.iaop(ib2);
        test(iao.equals(ib2));
        iao = ib2.iaop(ic);
        test(iao.equals(ic));
        iao = ic.iaop(ia);
        test(iao.equals(ia));
        iao = ic.iaop(ib1);
        test(iao.equals(ib1));
        iao = ic.iaop(ib2);
        test(iao.equals(ib2));
        iao = ic.iaop(ic);
        test(iao.equals(ic));

        iao = ib1.ib1op(ib1);
        test(iao.equals(ib1));
        IB1Prx ib1o = ib1.ib1op(ib1);
        test(ib1o.equals(ib1));
        iao = ib1.ib1op(ic);
        test(iao.equals(ic));
        ib1o = ib1.ib1op(ic);
        test(ib1o.equals(ic));
        iao = ic.ib1op(ib1);
        test(iao.equals(ib1));
        ib1o = ic.ib1op(ib1);
        test(ib1o.equals(ib1));
        iao = ic.ib1op(ic);
        test(iao.equals(ic));
        ib1o = ic.ib1op(ic);
        test(ib1o.equals(ic));

        iao = ib2.ib2op(ib2);
        test(iao.equals(ib2));
        IB2Prx ib2o = ib2.ib2op(ib2);
        test(ib2o.equals(ib2));
        iao = ib2.ib2op(ic);
        test(iao.equals(ic));
        ib2o = ib2.ib2op(ic);
        test(ib2o.equals(ic));
        iao = ic.ib2op(ib2);
        test(iao.equals(ib2));
        ib2o = ic.ib2op(ib2);
        test(ib2o.equals(ib2));
        iao = ic.ib2op(ic);
        test(iao.equals(ic));
        ib2o = ic.ib2op(ic);
        test(ib2o.equals(ic));

        iao = ic.icop(ic);
        test(iao.equals(ic));
        ib1o = ic.icop(ic);
        test(ib1o.equals(ic));
        ib2o = ic.icop(ic);
        test(ib2o.equals(ic));
        ICPrx ico = ic.icop(ic);
        test(ico.equals(ic));
        out.println("ok");

        return initial;
    }

    private AllTests() {}
}

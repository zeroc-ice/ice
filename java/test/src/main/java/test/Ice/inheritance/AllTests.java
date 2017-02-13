// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.inheritance;

import java.io.PrintWriter;

import test.Ice.inheritance.Test.InitialPrx;
import test.Ice.inheritance.Test.MA.CAPrx;
import test.Ice.inheritance.Test.MA.CCPrx;
import test.Ice.inheritance.Test.MA.CDPrx;
import test.Ice.inheritance.Test.MA.IAPrx;
import test.Ice.inheritance.Test.MA.ICPrx;
import test.Ice.inheritance.Test.MB.CBPrx;
import test.Ice.inheritance.Test.MB.IB1Prx;
import test.Ice.inheritance.Test.MB.IB2Prx;

public class AllTests
{
    private static void test(boolean b)
    {
        if(!b)
        {
            throw new RuntimeException();
        }
    }

    public static InitialPrx allTests(test.Util.Application app)
    {
        com.zeroc.Ice.Communicator communicator=app.communicator();
        PrintWriter out = app.getWriter();
        out.print("testing stringToProxy... ");
        out.flush();
        String ref = "initial:" + app.getTestEndpoint(0);
        com.zeroc.Ice.ObjectPrx base = communicator.stringToProxy(ref);
        test(base != null);
        out.println("ok");

        out.print("testing checked cast... ");
        out.flush();
        InitialPrx initial = InitialPrx.checkedCast(base);
        test(initial != null);
        test(initial.equals(base));
        out.println("ok");

        out.print("getting proxies for class hierarchy... ");
        out.flush();
        CAPrx ca = initial.caop();
        CBPrx cb = initial.cbop();
        CCPrx cc = initial.ccop();
        CDPrx cd = initial.cdop();
        test(ca != cb);
        test(ca != cc);
        test(ca != cd);
        test(cb != cc);
        test(cb != cd);
        test(cc != cd);
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

        out.print("invoking proxy operations on class hierarchy... ");
        out.flush();
        CAPrx cao;
        CBPrx cbo;
        CCPrx cco;

        cao = ca.caop(ca);
        test(cao.equals(ca));
        cao = ca.caop(cb);
        test(cao.equals(cb));
        cao = ca.caop(cc);
        test(cao.equals(cc));
        cao = cb.caop(ca);
        test(cao.equals(ca));
        cao = cb.caop(cb);
        test(cao.equals(cb));
        cao = cb.caop(cc);
        test(cao.equals(cc));
        cao = cc.caop(ca);
        test(cao.equals(ca));
        cao = cc.caop(cb);
        test(cao.equals(cb));
        cao = cc.caop(cc);
        test(cao.equals(cc));

        cao = cb.cbop(cb);
        test(cao.equals(cb));
        cbo = cb.cbop(cb);
        test(cbo.equals(cb));
        cao = cb.cbop(cc);
        test(cao.equals(cc));
        cbo = cb.cbop(cc);
        test(cbo.equals(cc));
        cao = cc.cbop(cb);
        test(cao.equals(cb));
        cbo = cc.cbop(cb);
        test(cbo.equals(cb));
        cao = cc.cbop(cc);
        test(cao.equals(cc));
        cbo = cc.cbop(cc);
        test(cbo.equals(cc));

        cao = cc.ccop(cc);
        test(cao.equals(cc));
        cbo = cc.ccop(cc);
        test(cbo.equals(cc));
        cco = cc.ccop(cc);
        test(cco.equals(cc));
        out.println("ok");

        out.print("ditto, but for interface hierarchy... ");
        out.flush();
        IAPrx iao;
        IB1Prx ib1o;
        IB2Prx ib2o;
        ICPrx ico;

        iao = ia.iaop(ia);
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
        ib1o = ib1.ib1op(ib1);
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
        ib2o = ib2.ib2op(ib2);
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
        ico = ic.icop(ic);
        test(ico.equals(ic));
        out.println("ok");

        out.print("ditto, but for class implementing interfaces... ");
        out.flush();

        cao = cd.caop(cd);
        test(cao.equals(cd));
        cbo = cd.cbop(cd);
        test(cbo.equals(cd));
        cco = cd.ccop(cd);
        test(cco.equals(cd));

        iao = cd.iaop(cd);
        test(iao.equals(cd));
        ib1o = cd.ib1op(cd);
        test(ib1o.equals(cd));
        ib2o = cd.ib2op(cd);
        test(ib2o.equals(cd));

        cao = cd.cdop(cd);
        test(cao.equals(cd));
        cbo = cd.cdop(cd);
        test(cbo.equals(cd));
        cco = cd.cdop(cd);
        test(cco.equals(cd));

        iao = cd.cdop(cd);
        test(iao.equals(cd));
        ib1o = cd.cdop(cd);
        test(ib1o.equals(cd));
        ib2o = cd.cdop(cd);
        test(ib2o.equals(cd));
        out.println("ok");

        return initial;
    }
}

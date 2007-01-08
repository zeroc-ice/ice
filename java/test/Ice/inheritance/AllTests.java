// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

public class AllTests
{
    private static void
    test(boolean b)
    {
        if(!b)
        {
            throw new RuntimeException();
        }
    }

    public static Test.InitialPrx
    allTests(Ice.Communicator communicator)
    {
        System.out.print("testing stringToProxy... ");
        System.out.flush();
        String ref = "initial:default -p 12010 -t 10000";
        Ice.ObjectPrx base = communicator.stringToProxy(ref);
        test(base != null);
        System.out.println("ok");

        System.out.print("testing checked cast... ");
        System.out.flush();
        Test.InitialPrx initial = Test.InitialPrxHelper.checkedCast(base);
        test(initial != null);
        test(initial.equals(base));
        System.out.println("ok");

        System.out.print("getting proxies for class hierarchy... ");
        System.out.flush();
        Test.MA.CAPrx ca = initial.caop();
        Test.MB.CBPrx cb = initial.cbop();
        Test.MA.CCPrx cc = initial.ccop();
        Test.MA.CDPrx cd = initial.cdop();
        test(ca != cb);
        test(ca != cc);
        test(ca != cd);
        test(cb != cc);
        test(cb != cd);
        test(cc != cd);
        System.out.println("ok");

        System.out.print("getting proxies for interface hierarchy... ");
        System.out.flush();
        Test.MA.IAPrx ia = initial.iaop();
        Test.MB.IB1Prx ib1 = initial.ib1op();
        Test.MB.IB2Prx ib2 = initial.ib2op();
        Test.MA.ICPrx ic = initial.icop();
        test(ia != ib1);
        test(ia != ib2);
        test(ia != ic);
        test(ib1 != ic);
        test(ib2 != ic);
        System.out.println("ok");

        System.out.print("invoking proxy operations on class hierarchy... ");
        System.out.flush();
        Test.MA.CAPrx cao;
        Test.MB.CBPrx cbo;
        Test.MA.CCPrx cco;
        
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
        System.out.println("ok");

        System.out.print("ditto, but for interface hierarchy... ");
        System.out.flush();
        Test.MA.IAPrx iao;
        Test.MB.IB1Prx ib1o;
        Test.MB.IB2Prx ib2o;
        Test.MA.ICPrx ico;

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
        System.out.println("ok");

        System.out.print("ditto, but for class implementing interfaces... ");
        System.out.flush();
        Test.MA.CDPrx cdo;

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
        System.out.println("ok");

        return initial;
    }
}

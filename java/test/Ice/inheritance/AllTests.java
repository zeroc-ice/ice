// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

public class AllTests
{
    private static void
    test(boolean b)
    {
        if (!b)
        {
            throw new RuntimeException();
        }
    }

    public static InitialPrx
    allTests(Ice.Communicator communicator)
    {
        System.out.print("testing stringToProxy... ");
        System.out.flush();
        String ref = "initial:default -p 12345 -t 2000";
        Ice.ObjectPrx base = communicator.stringToProxy(ref);
        test(base != null);
        System.out.println("ok");

        System.out.print("testing checked cast... ");
        System.out.flush();
        InitialPrx initial = InitialPrxHelper.checkedCast(base);
        test(initial != null);
        test(initial.equals(base));
        System.out.println("ok");

        System.out.print("getting proxies for class hierarchy... ");
        System.out.flush();
        M_A.C_APrx ca = initial.c_a();
        M_B.C_BPrx cb = initial.c_b();
        M_A.C_CPrx cc = initial.c_c();
        M_A.C_DPrx cd = initial.c_d();
        test(ca != cb);
        test(ca != cc);
        test(ca != cd);
        test(cb != cc);
        test(cb != cd);
        test(cc != cd);
        System.out.println("ok");

        System.out.print("getting proxies for interface hierarchy... ");
        System.out.flush();
        M_A.I_APrx ia = initial.i_a();
        M_B.I_B1Prx ib1 = initial.i_b1();
        M_B.I_B2Prx ib2 = initial.i_b2();
        M_A.I_CPrx ic = initial.i_c();
        test(ia != ib1);
        test(ia != ib2);
        test(ia != ic);
        test(ib1 != ic);
        test(ib2 != ic);
        System.out.println("ok");

        System.out.print("invoking proxy operations on class hierarchy... ");
        System.out.flush();
        M_A.C_APrx cao;
        M_B.C_BPrx cbo;
        M_A.C_CPrx cco;
        
        cao = ca.ca(ca);
        test(cao.equals(ca));
        cao = ca.ca(cb);
        test(cao.equals(cb));
        cao = ca.ca(cc);
        test(cao.equals(cc));
        cao = cb.ca(ca);
        test(cao.equals(ca));
        cao = cb.ca(cb);
        test(cao.equals(cb));
        cao = cb.ca(cc);
        test(cao.equals(cc));
        cao = cc.ca(ca);
        test(cao.equals(ca));
        cao = cc.ca(cb);
        test(cao.equals(cb));
        cao = cc.ca(cc);
        test(cao.equals(cc));
        
        cao = cb.cb(cb);
        test(cao.equals(cb));
        cbo = cb.cb(cb);
        test(cbo.equals(cb));
        cao = cb.cb(cc);
        test(cao.equals(cc));
        cbo = cb.cb(cc);
        test(cbo.equals(cc));
        cao = cc.cb(cb);
        test(cao.equals(cb));
        cbo = cc.cb(cb);
        test(cbo.equals(cb));
        cao = cc.cb(cc);
        test(cao.equals(cc));
        cbo = cc.cb(cc);
        test(cbo.equals(cc));

        cao = cc.cc(cc);
        test(cao.equals(cc));
        cbo = cc.cc(cc);
        test(cbo.equals(cc));
        cco = cc.cc(cc);
        test(cco.equals(cc));
        System.out.println("ok");

        System.out.print("ditto, but for interface hierarchy... ");
        System.out.flush();
        M_A.I_APrx iao;
        M_B.I_B1Prx ib1o;
        M_B.I_B2Prx ib2o;
        M_A.I_CPrx ico;

        iao = ia.ia(ia);
        test(iao.equals(ia));
        iao = ia.ia(ib1);
        test(iao.equals(ib1));
        iao = ia.ia(ib2);
        test(iao.equals(ib2));
        iao = ia.ia(ic);
        test(iao.equals(ic));
        iao = ib1.ia(ia);
        test(iao.equals(ia));
        iao = ib1.ia(ib1);
        test(iao.equals(ib1));
        iao = ib1.ia(ib2);
        test(iao.equals(ib2));
        iao = ib1.ia(ic);
        test(iao.equals(ic));
        iao = ib2.ia(ia);
        test(iao.equals(ia));
        iao = ib2.ia(ib1);
        test(iao.equals(ib1));
        iao = ib2.ia(ib2);
        test(iao.equals(ib2));
        iao = ib2.ia(ic);
        test(iao.equals(ic));
        iao = ic.ia(ia);
        test(iao.equals(ia));
        iao = ic.ia(ib1);
        test(iao.equals(ib1));
        iao = ic.ia(ib2);
        test(iao.equals(ib2));
        iao = ic.ia(ic);
        test(iao.equals(ic));

        iao = ib1.ib1(ib1);
        test(iao.equals(ib1));
        ib1o = ib1.ib1(ib1);
        test(ib1o.equals(ib1));
        iao = ib1.ib1(ic);
        test(iao.equals(ic));
        ib1o = ib1.ib1(ic);
        test(ib1o.equals(ic));
        iao = ic.ib1(ib1);
        test(iao.equals(ib1));
        ib1o = ic.ib1(ib1);
        test(ib1o.equals(ib1));
        iao = ic.ib1(ic);
        test(iao.equals(ic));
        ib1o = ic.ib1(ic);
        test(ib1o.equals(ic));

        iao = ib2.ib2(ib2);
        test(iao.equals(ib2));
        ib2o = ib2.ib2(ib2);
        test(ib2o.equals(ib2));
        iao = ib2.ib2(ic);
        test(iao.equals(ic));
        ib2o = ib2.ib2(ic);
        test(ib2o.equals(ic));
        iao = ic.ib2(ib2);
        test(iao.equals(ib2));
        ib2o = ic.ib2(ib2);
        test(ib2o.equals(ib2));
        iao = ic.ib2(ic);
        test(iao.equals(ic));
        ib2o = ic.ib2(ic);
        test(ib2o.equals(ic));

        iao = ic.ic(ic);
        test(iao.equals(ic));
        ib1o = ic.ic(ic);
        test(ib1o.equals(ic));
        ib2o = ic.ic(ic);
        test(ib2o.equals(ic));
        ico = ic.ic(ic);
        test(ico.equals(ic));
        System.out.println("ok");

        System.out.print("ditto, but for class implementing interfaces... ");
        System.out.flush();
        M_A.C_DPrx cdo;

        cao = cd.ca(cd);
        test(cao.equals(cd));
        cbo = cd.cb(cd);
        test(cbo.equals(cd));
        cco = cd.cc(cd);
        test(cco.equals(cd));

        iao = cd.ia(cd);
        test(iao.equals(cd));
        ib1o = cd.ib1(cd);
        test(ib1o.equals(cd));
        ib2o = cd.ib2(cd);
        test(ib2o.equals(cd));

        cao = cd.cd(cd);
        test(cao.equals(cd));
        cbo = cd.cd(cd);
        test(cbo.equals(cd));
        cco = cd.cd(cd);
        test(cco.equals(cd));

        iao = cd.cd(cd);
        test(iao.equals(cd));
        ib1o = cd.cd(cd);
        test(ib1o.equals(cd));
        ib2o = cd.cd(cd);
        test(ib2o.equals(cd));
        System.out.println("ok");

        return initial;
    }
}

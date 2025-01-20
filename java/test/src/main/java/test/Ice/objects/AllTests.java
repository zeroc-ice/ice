//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.Ice.objects;

import test.Ice.objects.Test.A1;
import test.Ice.objects.Test.B;
import test.Ice.objects.Test.Base;
import test.Ice.objects.Test.C;
import test.Ice.objects.Test.D;
import test.Ice.objects.Test.D1;
import test.Ice.objects.Test.EDerived;
import test.Ice.objects.Test.F1;
import test.Ice.objects.Test.F2Prx;
import test.Ice.objects.Test.F3;
import test.Ice.objects.Test.G;
import test.Ice.objects.Test.Initial;
import test.Ice.objects.Test.Initial.OpValueMapResult;
import test.Ice.objects.Test.Initial.OpValueResult;
import test.Ice.objects.Test.Initial.OpValueSeqResult;
import test.Ice.objects.Test.InitialPrx;
import test.Ice.objects.Test.K;
import test.Ice.objects.Test.L;
import test.Ice.objects.Test.M;
import test.Ice.objects.Test.Recursive;
import test.Ice.objects.Test.S;
import test.Ice.objects.Test.StructKey;
import test.Ice.objects.Test.UnexpectedObjectExceptionTestPrx;

import java.io.PrintWriter;

public class AllTests {
    private static void test(boolean b) {
        if (!b) {
            throw new RuntimeException();
        }
    }

    public static InitialPrx allTests(test.TestHelper helper) {
        com.zeroc.Ice.Communicator communicator = helper.communicator();
        PrintWriter out = helper.getWriter();
        out.print("testing stringToProxy... ");
        out.flush();
        String ref = "initial:" + helper.getTestEndpoint(0);
        com.zeroc.Ice.ObjectPrx base = communicator.stringToProxy(ref);
        test(base != null);
        out.println("ok");

        out.print("testing checked cast... ");
        out.flush();
        InitialPrx initial = InitialPrx.checkedCast(base);
        test(initial != null);
        test(initial.equals(base));
        out.println("ok");

        out.print("getting B1... ");
        out.flush();
        B b1 = initial.getB1();
        test(b1 != null);
        out.println("ok");

        out.print("getting B2... ");
        out.flush();
        B b2 = initial.getB2();
        test(b2 != null);
        out.println("ok");

        out.print("getting C... ");
        out.flush();
        C c = initial.getC();
        test(c != null);
        out.println("ok");

        out.print("getting D... ");
        out.flush();
        D d = initial.getD();
        test(d != null);
        out.println("ok");

        out.print("checking consistency... ");
        out.flush();
        test(b1 != b2);
        // test(b1 != c);
        // test(b1 != d);
        // test(b2 != c);
        // test(b2 != d);
        // test(c != d);
        test(b1.theB == b1);
        test(b1.theC == null);
        test(b1.theA instanceof B);
        test(((B) b1.theA).theA == b1.theA);
        test(((B) b1.theA).theB == b1);
        test(((B) b1.theA).theC instanceof C);
        test((((B) b1.theA).theC).theB == b1.theA);
        test(b1.preMarshalInvoked);
        test(b1.postUnmarshalInvoked);
        test(b1.theA.preMarshalInvoked);
        test(b1.theA.postUnmarshalInvoked);
        test(((B) b1.theA).theC.preMarshalInvoked);
        test(((B) b1.theA).theC.postUnmarshalInvoked);

        // More tests possible for b2 and d, but I think this is already
        // sufficient.
        test(b2.theA == b2);
        test(d.theC == null);
        out.println("ok");

        out.print("getting B1, B2, C, and D all at once... ");
        out.flush();
        Initial.GetAllResult r;
        r = initial.getAll();
        b1 = r.b1;
        b2 = r.b2;
        c = r.theC;
        d = r.theD;
        test(b1 != null);
        test(b2 != null);
        test(c != null);
        test(d != null);
        out.println("ok");

        out.print("checking consistency... ");
        out.flush();
        test(b1 != b2);
        // test(b1 != c);
        // test(b1 != d);
        // test(b2 != c);
        // test(b2 != d);
        // test(c != d);
        test(b1.theA == b2);
        test(b1.theB == b1);
        test(b1.theC == null);
        test(b2.theA == b2);
        test(b2.theB == b1);
        test(b2.theC == c);
        test(c.theB == b2);
        test(d.theA == b1);
        test(d.theB == b2);
        test(d.theC == null);
        test(d.preMarshalInvoked);
        test(d.postUnmarshalInvoked);
        test(d.theA.preMarshalInvoked);
        test(d.theA.postUnmarshalInvoked);
        test(d.theB.preMarshalInvoked);
        test(d.theB.postUnmarshalInvoked);
        test(d.theB.theC.preMarshalInvoked);
        test(d.theB.theC.postUnmarshalInvoked);

        out.println("ok");

        out.print("getting K... ");
        out.flush();
        {
            K k = initial.getK();
            test(k.value instanceof L);
            L l = (L) k.value;
            test(l.data.equals("l"));
        }
        out.println("ok");

        out.print("testing Value as parameter... ");
        {
            com.zeroc.Ice.Value v1 = new L("l");
            OpValueResult result = initial.opValue(v1);
            test(((L) result.returnValue).data.equals("l"));
            test(((L) result.v2).data.equals("l"));
        }
        {
            L l = new L("l");
            com.zeroc.Ice.Value[] v1 = {l};
            OpValueSeqResult result = initial.opValueSeq(v1);
            test(((L) result.returnValue[0]).data.equals("l"));
            test(((L) result.v2[0]).data.equals("l"));
        }
        {
            L l = new L("l");
            java.util.Map<String, com.zeroc.Ice.Value> v1 =
                    new java.util.HashMap<String, com.zeroc.Ice.Value>();
            v1.put("l", l);
            OpValueMapResult result = initial.opValueMap(v1);
            test(((L) result.returnValue.get("l")).data.equals("l"));
            test(((L) result.v2.get("l")).data.equals("l"));
        }
        out.println("ok");

        out.print("getting D1... ");
        out.flush();
        D1 d1 = new D1(new A1("a1"), new A1("a2"), new A1("a3"), new A1("a4"));
        d1 = initial.getD1(d1);
        test(d1.a1.name.equals("a1"));
        test(d1.a2.name.equals("a2"));
        test(d1.a3.name.equals("a3"));
        test(d1.a4.name.equals("a4"));
        out.println("ok");

        out.print("throw EDerived... ");
        out.flush();
        try {
            initial.throwEDerived();
            test(false);
        } catch (EDerived ederived) {
            test(ederived.a1.name.equals("a1"));
            test(ederived.a2.name.equals("a2"));
            test(ederived.a3.name.equals("a3"));
            test(ederived.a4.name.equals("a4"));
        }
        out.println("ok");

        out.print("setting G... ");
        out.flush();
        try {
            initial.setG(new G(new S("hello"), "g"));
        } catch (com.zeroc.Ice.OperationNotExistException ex) {
        }
        out.println("ok");

        out.print("testing sequences...");
        try {
            out.flush();
            Base[] inS = new Base[0];
            Initial.OpBaseSeqResult sr = initial.opBaseSeq(inS);
            test(sr.returnValue.length == 0 && sr.outSeq.length == 0);

            inS = new Base[1];
            inS[0] = new Base(new S(), "");
            sr = initial.opBaseSeq(inS);
            test(sr.returnValue.length == 1 && sr.outSeq.length == 1);
        } catch (com.zeroc.Ice.OperationNotExistException ex) {
        }
        out.println("ok");

        out.print("testing recursive type... ");
        out.flush();
        Recursive top = new Recursive();
        Recursive bottom = top;
        int maxDepth = 10;
        for (int i = 1; i < maxDepth; ++i) {
            bottom.v = new Recursive();
            bottom = bottom.v;
        }
        initial.setRecursive(top);

        // Adding one more level would exceed the max class graph depth
        bottom.v = new Recursive();
        bottom = bottom.v;

        try {
            initial.setRecursive(top);
            test(false);
        } catch (com.zeroc.Ice.UnknownLocalException ex) {
            // Expected marshal exception from the server (max class graph depth reached)
        }
        out.println("ok");

        out.print("testing compact ID...");
        out.flush();
        try {
            test(initial.getCompact() != null);
        } catch (com.zeroc.Ice.OperationNotExistException ex) {
        }
        out.println("ok");

        out.print("testing marshaled results...");
        out.flush();
        b1 = initial.getMB();
        test(b1 != null && b1.theB == b1);
        b1 = initial.getAMDMBAsync().join();
        test(b1 != null && b1.theB == b1);
        out.println("ok");

        out.print("testing UnexpectedObjectException...");
        out.flush();
        ref = "uoet:" + helper.getTestEndpoint(0);
        base = communicator.stringToProxy(ref);
        test(base != null);
        UnexpectedObjectExceptionTestPrx uoet =
                UnexpectedObjectExceptionTestPrx.uncheckedCast(base);
        test(uoet != null);
        try {
            uoet.op();
            test(false);
        } catch (com.zeroc.Ice.MarshalException ex) {
            test(ex.getMessage().contains("'::Test::AlsoEmpty'"));
            test(ex.getMessage().contains("'::Test::Empty'"));
        } catch (Exception ex) {
            out.println(ex);
            test(false);
        }
        out.println("ok");

        out.print("testing class containing complex dictionary... ");
        out.flush();
        {
            M m = new M();
            m.v = new java.util.HashMap<StructKey, L>();
            StructKey k1 = new StructKey(1, "1");
            m.v.put(k1, new L("one"));
            StructKey k2 = new StructKey(2, "2");
            m.v.put(k2, new L("two"));

            Initial.OpMResult opMResult = initial.opM(m);
            test(opMResult.returnValue.v.size() == 2);
            test(opMResult.v2.v.size() == 2);

            test(opMResult.returnValue.v.get(k1).data.equals("one"));
            test(opMResult.v2.v.get(k1).data.equals("one"));

            test(opMResult.returnValue.v.get(k2).data.equals("two"));
            test(opMResult.v2.v.get(k2).data.equals("two"));
        }
        out.println("ok");

        out.print("testing forward declared types... ");
        out.flush();
        {
            Initial.OpF1Result opF1Result = initial.opF1(new F1("F11"));
            test(opF1Result.returnValue.name.equals("F11"));
            test(opF1Result.f12.name.equals("F12"));

            Initial.OpF2Result opF2Result =
                    initial.opF2(
                            F2Prx.createProxy(communicator, "F21:" + helper.getTestEndpoint()));
            test(opF2Result.returnValue.ice_getIdentity().name.equals("F21"));
            opF2Result.returnValue.op();
            test(opF2Result.f22.ice_getIdentity().name.equals("F22"));

            if (initial.hasF3()) {
                Initial.OpF3Result opF3Result =
                        initial.opF3(new F3(new F1("F11"), F2Prx.createProxy(communicator, "F21")));
                test(opF3Result.returnValue.f1.name.equals("F11"));
                test(opF3Result.returnValue.f2.ice_getIdentity().name.equals("F21"));

                test(opF3Result.f32.f1.name.equals("F12"));
                test(opF3Result.f32.f2.ice_getIdentity().name.equals("F22"));
            }
        }
        out.println("ok");

        out.print("testing sending class cycle... ");
        out.flush();
        {
            Recursive rec = new Recursive();
            rec.v = rec;
            boolean acceptsCycles = initial.acceptsClassCycles();
            try {
                initial.setCycle(rec);
                test(acceptsCycles);
            } catch (com.zeroc.Ice.UnknownLocalException ex) {
                test(!acceptsCycles);
            }
        }
        out.println("ok");

        return initial;
    }
}

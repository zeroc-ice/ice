// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.scope;

import java.io.PrintWriter;

public class AllTests
{
    private static void test(boolean b)
    {
        if(!b)
        {
            throw new RuntimeException();
        }
    }

    public static void allTests(test.TestHelper helper)
    {
        PrintWriter out = helper.getWriter();
        com.zeroc.Ice.Communicator communicator = helper.communicator();

        {
            com.zeroc.Ice.ObjectPrx obj = communicator.stringToProxy("i1:" + helper.getTestEndpoint());
            test.Ice.scope.Test.IPrx i = test.Ice.scope.Test.IPrx.checkedCast(obj);

            test.Ice.scope.Test.S s1 = new test.Ice.scope.Test.S(0);
            test.Ice.scope.Test.I.OpSResult opSResult = i.opS(s1);
            test(s1.equals(opSResult.returnValue));
            test(s1.equals(opSResult.s2));

            test.Ice.scope.Test.S[] sseq1 = new test.Ice.scope.Test.S[]{ s1 };
            test.Ice.scope.Test.I.OpSSeqResult opSSeqResult = i.opSSeq(sseq1);
            test(opSSeqResult.returnValue[0].equals(s1));
            test(opSSeqResult.s2[0].equals(s1));

            java.util.Map<String, test.Ice.scope.Test.S> smap1 = new java.util.HashMap<String, test.Ice.scope.Test.S>();
            smap1.put("a", s1);
            test.Ice.scope.Test.I.OpSMapResult opSMapResult = i.opSMap(smap1);
            test(opSMapResult.returnValue.get("a").equals(s1));
            test(opSMapResult.s2.get("a").equals(s1));

            test.Ice.scope.Test.C c1 = new test.Ice.scope.Test.C(s1);
            test.Ice.scope.Test.I.OpCResult opCResult = i.opC(c1);
            test(c1.s.equals(opCResult.returnValue.s));
            test(c1.s.equals(opCResult.c2.s));

            test.Ice.scope.Test.C[] cseq1 = new test.Ice.scope.Test.C[]{ c1 };
            test.Ice.scope.Test.I.OpCSeqResult opCSeqResult = i.opCSeq(cseq1);
            test(opCSeqResult.returnValue[0].s.equals(s1));
            test(opCSeqResult.c2[0].s.equals(s1));

            java.util.Map<String, test.Ice.scope.Test.C> cmap1 = new java.util.HashMap<String, test.Ice.scope.Test.C>();
            cmap1.put("a", c1);
            test.Ice.scope.Test.I.OpCMapResult opCMapResult = i.opCMap(cmap1);
            test(opCMapResult.returnValue.get("a").s.equals(s1));
            test(opCMapResult.c2.get("a").s.equals(s1));
        }

        {
            com.zeroc.Ice.ObjectPrx obj = communicator.stringToProxy("i1:" + helper.getTestEndpoint());
            test.Ice.scope.Test.IPrx i = test.Ice.scope.Test.IPrx.checkedCast(obj);

            test.Ice.scope.Test.S s1 = new test.Ice.scope.Test.S(0);
            test.Ice.scope.Test.I.OpSResult opSResult = i.opSAsync(s1).join();
            test(s1.equals(opSResult.returnValue));
            test(s1.equals(opSResult.s2));

            test.Ice.scope.Test.S[] sseq1 = new test.Ice.scope.Test.S[]{ s1 };
            test.Ice.scope.Test.I.OpSSeqResult opSSeqResult = i.opSSeqAsync(sseq1).join();
            test(opSSeqResult.returnValue[0].equals(s1));
            test(opSSeqResult.s2[0].equals(s1));

            java.util.Map<String, test.Ice.scope.Test.S> smap1 = new java.util.HashMap<String, test.Ice.scope.Test.S>();
            smap1.put("a", s1);
            test.Ice.scope.Test.I.OpSMapResult opSMapResult = i.opSMapAsync(smap1).join();
            test(opSMapResult.returnValue.get("a").equals(s1));
            test(opSMapResult.s2.get("a").equals(s1));

            test.Ice.scope.Test.C c1 = new test.Ice.scope.Test.C(s1);
            test.Ice.scope.Test.I.OpCResult opCResult = i.opCAsync(c1).join();
            test(c1.s.equals(opCResult.returnValue.s));
            test(c1.s.equals(opCResult.c2.s));

            test.Ice.scope.Test.C[] cseq1 = new test.Ice.scope.Test.C[]{ c1 };
            test.Ice.scope.Test.I.OpCSeqResult opCSeqResult = i.opCSeqAsync(cseq1).join();
            test(opCSeqResult.returnValue[0].s.equals(s1));
            test(opCSeqResult.c2[0].s.equals(s1));

            java.util.Map<String, test.Ice.scope.Test.C> cmap1 = new java.util.HashMap<String, test.Ice.scope.Test.C>();
            cmap1.put("a", c1);
            test.Ice.scope.Test.I.OpCMapResult opCMapResult = i.opCMapAsync(cmap1).join();
            test(opCMapResult.returnValue.get("a").s.equals(s1));
            test(opCMapResult.c2.get("a").s.equals(s1));
        }

        {
            com.zeroc.Ice.ObjectPrx obj = communicator.stringToProxy("i2:" + helper.getTestEndpoint());
            test.Ice.scope.Test.Inner.IPrx i = test.Ice.scope.Test.Inner.IPrx.checkedCast(obj);

            test.Ice.scope.Test.Inner.Inner2.S s1 = new test.Ice.scope.Test.Inner.Inner2.S(0);
            test.Ice.scope.Test.Inner.I.OpSResult opSResult = i.opS(s1);
            test(s1.equals(opSResult.returnValue));
            test(s1.equals(opSResult.s2));

            test.Ice.scope.Test.Inner.Inner2.S[] sseq1 = new test.Ice.scope.Test.Inner.Inner2.S[]{ s1 };
            test.Ice.scope.Test.Inner.I.OpSSeqResult opSSeqResult = i.opSSeq(sseq1);
            test(opSSeqResult.returnValue[0].equals(s1));
            test(opSSeqResult.s2[0].equals(s1));

            java.util.Map<String, test.Ice.scope.Test.Inner.Inner2.S> smap1 =
                new java.util.HashMap<String, test.Ice.scope.Test.Inner.Inner2.S>();
            smap1.put("a", s1);
            test.Ice.scope.Test.Inner.I.OpSMapResult opSMapResult = i.opSMap(smap1);
            test(opSMapResult.returnValue.get("a").equals(s1));
            test(opSMapResult.s2.get("a").equals(s1));

            test.Ice.scope.Test.Inner.Inner2.C c1 = new test.Ice.scope.Test.Inner.Inner2.C(s1);
            test.Ice.scope.Test.Inner.I.OpCResult opCResult = i.opC(c1);
            test(c1.s.equals(opCResult.returnValue.s));
            test(c1.s.equals(opCResult.c2.s));

            test.Ice.scope.Test.Inner.Inner2.C[] cseq1 = new test.Ice.scope.Test.Inner.Inner2.C[]{ c1 };
            test.Ice.scope.Test.Inner.I.OpCSeqResult opCSeqResult = i.opCSeq(cseq1);
            test(opCSeqResult.returnValue[0].s.equals(s1));
            test(opCSeqResult.c2[0].s.equals(s1));

            java.util.Map<String, test.Ice.scope.Test.Inner.Inner2.C> cmap1
                = new java.util.HashMap<String, test.Ice.scope.Test.Inner.Inner2.C>();
            cmap1.put("a", c1);
            test.Ice.scope.Test.Inner.I.OpCMapResult opCMapResult = i.opCMap(cmap1);
            test(opCMapResult.returnValue.get("a").s.equals(s1));
            test(opCMapResult.c2.get("a").s.equals(s1));
        }

        {
            com.zeroc.Ice.ObjectPrx obj = communicator.stringToProxy("i2:" + helper.getTestEndpoint());
            test.Ice.scope.Test.Inner.IPrx i = test.Ice.scope.Test.Inner.IPrx.checkedCast(obj);

            test.Ice.scope.Test.Inner.Inner2.S s1 = new test.Ice.scope.Test.Inner.Inner2.S(0);
            test.Ice.scope.Test.Inner.I.OpSResult opSResult = i.opSAsync(s1).join();
            test(s1.equals(opSResult.returnValue));
            test(s1.equals(opSResult.s2));

            test.Ice.scope.Test.Inner.Inner2.S[] sseq1 = new test.Ice.scope.Test.Inner.Inner2.S[]{ s1 };
            test.Ice.scope.Test.Inner.I.OpSSeqResult opSSeqResult = i.opSSeqAsync(sseq1).join();
            test(opSSeqResult.returnValue[0].equals(s1));
            test(opSSeqResult.s2[0].equals(s1));

            java.util.Map<String, test.Ice.scope.Test.Inner.Inner2.S> smap1 =
                new java.util.HashMap<String, test.Ice.scope.Test.Inner.Inner2.S>();
            smap1.put("a", s1);
            test.Ice.scope.Test.Inner.I.OpSMapResult opSMapResult = i.opSMapAsync(smap1).join();
            test(opSMapResult.returnValue.get("a").equals(s1));
            test(opSMapResult.s2.get("a").equals(s1));

            test.Ice.scope.Test.Inner.Inner2.C c1 = new test.Ice.scope.Test.Inner.Inner2.C(s1);
            test.Ice.scope.Test.Inner.I.OpCResult opCResult = i.opCAsync(c1).join();
            test(c1.s.equals(opCResult.returnValue.s));
            test(c1.s.equals(opCResult.c2.s));

            test.Ice.scope.Test.Inner.Inner2.C[] cseq1 = new test.Ice.scope.Test.Inner.Inner2.C[]{ c1 };
            test.Ice.scope.Test.Inner.I.OpCSeqResult opCSeqResult = i.opCSeqAsync(cseq1).join();
            test(opCSeqResult.returnValue[0].s.equals(s1));
            test(opCSeqResult.c2[0].s.equals(s1));

            java.util.Map<String, test.Ice.scope.Test.Inner.Inner2.C> cmap1 =
                new java.util.HashMap<String, test.Ice.scope.Test.Inner.Inner2.C>();
            cmap1.put("a", c1);
            test.Ice.scope.Test.Inner.I.OpCMapResult opCMapResult = i.opCMapAsync(cmap1).join();
            test(opCMapResult.returnValue.get("a").s.equals(s1));
            test(opCMapResult.c2.get("a").s.equals(s1));
        }

        {
            com.zeroc.Ice.ObjectPrx obj = communicator.stringToProxy("i3:" + helper.getTestEndpoint());
            test.Ice.scope.Test.Inner.Inner2.IPrx i = test.Ice.scope.Test.Inner.Inner2.IPrx.checkedCast(obj);

            test.Ice.scope.Test.Inner.Inner2.S s1 = new test.Ice.scope.Test.Inner.Inner2.S(0);
            test.Ice.scope.Test.Inner.Inner2.I.OpSResult opSResult = i.opS(s1);
            test(s1.equals(opSResult.returnValue));
            test(s1.equals(opSResult.s2));

            test.Ice.scope.Test.Inner.Inner2.S[] sseq1 = new test.Ice.scope.Test.Inner.Inner2.S[]{ s1 };
            test.Ice.scope.Test.Inner.Inner2.I.OpSSeqResult opSSeqResult = i.opSSeq(sseq1);
            test(opSSeqResult.returnValue[0].equals(s1));
            test(opSSeqResult.s2[0].equals(s1));

            java.util.Map<String, test.Ice.scope.Test.Inner.Inner2.S> smap1 =
                new java.util.HashMap<String, test.Ice.scope.Test.Inner.Inner2.S>();
            smap1.put("a", s1);
            test.Ice.scope.Test.Inner.Inner2.I.OpSMapResult opSMapResult = i.opSMap(smap1);
            test(opSMapResult.returnValue.get("a").equals(s1));
            test(opSMapResult.s2.get("a").equals(s1));

            test.Ice.scope.Test.Inner.Inner2.C c1 = new test.Ice.scope.Test.Inner.Inner2.C(s1);
            test.Ice.scope.Test.Inner.Inner2.I.OpCResult opCResult = i.opC(c1);
            test(c1.s.equals(opCResult.returnValue.s));
            test(c1.s.equals(opCResult.c2.s));

            test.Ice.scope.Test.Inner.Inner2.C[] cseq1 = new test.Ice.scope.Test.Inner.Inner2.C[]{ c1 };
            test.Ice.scope.Test.Inner.Inner2.I.OpCSeqResult opCSeqResult = i.opCSeq(cseq1);
            test(opCSeqResult.returnValue[0].s.equals(s1));
            test(opCSeqResult.c2[0].s.equals(s1));

            java.util.Map<String, test.Ice.scope.Test.Inner.Inner2.C> cmap1
                = new java.util.HashMap<String, test.Ice.scope.Test.Inner.Inner2.C>();
            cmap1.put("a", c1);
            test.Ice.scope.Test.Inner.Inner2.I.OpCMapResult opCMapResult = i.opCMap(cmap1);
            test(opCMapResult.returnValue.get("a").s.equals(s1));
            test(opCMapResult.c2.get("a").s.equals(s1));
        }

        {
            com.zeroc.Ice.ObjectPrx obj = communicator.stringToProxy("i3:" + helper.getTestEndpoint());
            test.Ice.scope.Test.Inner.Inner2.IPrx i = test.Ice.scope.Test.Inner.Inner2.IPrx.checkedCast(obj);

            test.Ice.scope.Test.Inner.Inner2.S s1 = new test.Ice.scope.Test.Inner.Inner2.S(0);
            test.Ice.scope.Test.Inner.Inner2.I.OpSResult opSResult = i.opSAsync(s1).join();
            test(s1.equals(opSResult.returnValue));
            test(s1.equals(opSResult.s2));

            test.Ice.scope.Test.Inner.Inner2.S[] sseq1 = new test.Ice.scope.Test.Inner.Inner2.S[]{ s1 };
            test.Ice.scope.Test.Inner.Inner2.I.OpSSeqResult opSSeqResult = i.opSSeqAsync(sseq1).join();
            test(opSSeqResult.returnValue[0].equals(s1));
            test(opSSeqResult.s2[0].equals(s1));

            java.util.Map<String, test.Ice.scope.Test.Inner.Inner2.S> smap1 =
                new java.util.HashMap<String, test.Ice.scope.Test.Inner.Inner2.S>();
            smap1.put("a", s1);
            test.Ice.scope.Test.Inner.Inner2.I.OpSMapResult opSMapResult = i.opSMapAsync(smap1).join();
            test(opSMapResult.returnValue.get("a").equals(s1));
            test(opSMapResult.s2.get("a").equals(s1));

            test.Ice.scope.Test.Inner.Inner2.C c1 = new test.Ice.scope.Test.Inner.Inner2.C(s1);
            test.Ice.scope.Test.Inner.Inner2.I.OpCResult opCResult = i.opCAsync(c1).join();
            test(c1.s.equals(opCResult.returnValue.s));
            test(c1.s.equals(opCResult.c2.s));

            test.Ice.scope.Test.Inner.Inner2.C[] cseq1 = new test.Ice.scope.Test.Inner.Inner2.C[]{ c1 };
            test.Ice.scope.Test.Inner.Inner2.I.OpCSeqResult opCSeqResult = i.opCSeqAsync(cseq1).join();
            test(opCSeqResult.returnValue[0].s.equals(s1));
            test(opCSeqResult.c2[0].s.equals(s1));

            java.util.Map<String, test.Ice.scope.Test.Inner.Inner2.C> cmap1 =
                new java.util.HashMap<String, test.Ice.scope.Test.Inner.Inner2.C>();
            cmap1.put("a", c1);
            test.Ice.scope.Test.Inner.Inner2.I.OpCMapResult opCMapResult = i.opCMapAsync(cmap1).join();
            test(opCMapResult.returnValue.get("a").s.equals(s1));
            test(opCMapResult.c2.get("a").s.equals(s1));
        }

        {
            com.zeroc.Ice.ObjectPrx obj = communicator.stringToProxy("i4:" + helper.getTestEndpoint());
            test.Ice.scope.Inner.Test.Inner2.IPrx i = test.Ice.scope.Inner.Test.Inner2.IPrx.checkedCast(obj);

            test.Ice.scope.Test.S s1 = new test.Ice.scope.Test.S(0);
            test.Ice.scope.Inner.Test.Inner2.I.OpSResult opSResult = i.opSAsync(s1).join();
            test(s1.equals(opSResult.returnValue));
            test(s1.equals(opSResult.s2));

            test.Ice.scope.Test.S[] sseq1 = new test.Ice.scope.Test.S[]{ s1 };
            test.Ice.scope.Inner.Test.Inner2.I.OpSSeqResult opSSeqResult = i.opSSeqAsync(sseq1).join();
            test(opSSeqResult.returnValue[0].equals(s1));
            test(opSSeqResult.s2[0].equals(s1));

            java.util.Map<String, test.Ice.scope.Test.S> smap1 = new java.util.HashMap<String, test.Ice.scope.Test.S>();
            smap1.put("a", s1);
            test.Ice.scope.Inner.Test.Inner2.I.OpSMapResult opSMapResult = i.opSMapAsync(smap1).join();
            test(opSMapResult.returnValue.get("a").equals(s1));
            test(opSMapResult.s2.get("a").equals(s1));

            test.Ice.scope.Test.C c1 = new test.Ice.scope.Test.C(s1);
            test.Ice.scope.Inner.Test.Inner2.I.OpCResult opCResult = i.opCAsync(c1).join();
            test(c1.s.equals(opCResult.returnValue.s));
            test(c1.s.equals(opCResult.c2.s));

            test.Ice.scope.Test.C[] cseq1 = new test.Ice.scope.Test.C[]{ c1 };
            test.Ice.scope.Inner.Test.Inner2.I.OpCSeqResult opCSeqResult = i.opCSeqAsync(cseq1).join();
            test(opCSeqResult.returnValue[0].s.equals(s1));
            test(opCSeqResult.c2[0].s.equals(s1));

            java.util.Map<String, test.Ice.scope.Test.C> cmap1 = new java.util.HashMap<String, test.Ice.scope.Test.C>();
            cmap1.put("a", c1);
            test.Ice.scope.Inner.Test.Inner2.I.OpCMapResult opCMapResult = i.opCMapAsync(cmap1).join();
            test(opCMapResult.returnValue.get("a").s.equals(s1));
            test(opCMapResult.c2.get("a").s.equals(s1));
        }

        {
            test.Ice.scope.Test.IPrx i =
                test.Ice.scope.Test.IPrx.checkedCast(communicator.stringToProxy("i1:" + helper.getTestEndpoint()));
            i.shutdown();
        }
    }
}

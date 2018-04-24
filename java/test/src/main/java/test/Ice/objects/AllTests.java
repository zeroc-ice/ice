// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.objects;

import java.io.PrintWriter;

import test.Ice.objects.Test.B;
import test.Ice.objects.Test.C;
import test.Ice.objects.Test.D;
import test.Ice.objects.Test.E;
import test.Ice.objects.Test.F;
import test.Ice.objects.Test.G;
import test.Ice.objects.Test.H;
import test.Ice.objects.Test.I;
import test.Ice.objects.Test.A1;
import test.Ice.objects.Test.B1;
import test.Ice.objects.Test.D1;
import test.Ice.objects.Test.EDerived;
import test.Ice.objects.Test.Base;
import test.Ice.objects.Test.S;
import test.Ice.objects.Test.Initial;
import test.Ice.objects.Test.InitialPrx;
import test.Ice.objects.Test.J;
import test.Ice.objects.Test.Recursive;
import test.Ice.objects.Test.UnexpectedObjectExceptionTestPrx;

public class AllTests
{
    private static void test(boolean b)
    {
        if(!b)
        {
            throw new RuntimeException();
        }
    }

    @SuppressWarnings("deprecation")
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
        //test(b1 != c);
        //test(b1 != d);
        //test(b2 != c);
        //test(b2 != d);
        //test(c != d);
        test(b1.theB == b1);
        test(b1.theC == null);
        test(b1.theA instanceof B);
        test(((B)b1.theA).theA == b1.theA);
        test(((B)b1.theA).theB == b1);
        test(((B)b1.theA).theC instanceof C);
        test((((B)b1.theA).theC).theB == b1.theA);
        test(b1.preMarshalInvoked);
        test(b1.postUnmarshalInvoked);
        test(b1.theA.preMarshalInvoked);
        test(b1.theA.postUnmarshalInvoked);
        test(((B)b1.theA).theC.preMarshalInvoked);
        test(((B)b1.theA).theC.postUnmarshalInvoked);

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
        //test(b1 != c);
        //test(b1 != d);
        //test(b2 != c);
        //test(b2 != d);
        //test(c != d);
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

        out.print("testing protected members... ");
        out.flush();
        E e = initial.getE();
        test(((EI)e).checkValues());
        try
        {
            test((E.class.getDeclaredField("i").getModifiers() & java.lang.reflect.Modifier.PROTECTED) != 0);
            test((E.class.getDeclaredField("s").getModifiers() & java.lang.reflect.Modifier.PROTECTED) != 0);
        }
        catch(Exception ex)
        {
            test(false);
        }
        F f = initial.getF();
        test(((FI)f).checkValues());
        test(((EI)f.e2).checkValues());
        try
        {
            test((F.class.getDeclaredField("e1").getModifiers() & java.lang.reflect.Modifier.PROTECTED) != 0);
            test((F.class.getDeclaredField("e2").getModifiers() & java.lang.reflect.Modifier.PROTECTED) == 0);
        }
        catch(Exception ex)
        {
            test(false);
        }
        out.println("ok");

        out.print("getting I, J and H... ");
        out.flush();
        com.zeroc.Ice.Value i = initial.getI();
        test(i != null && i.ice_id().equals(I.ice_staticId()));
        com.zeroc.Ice.Value j = initial.getJ();
        test(j != null && j.ice_id().equals(J.ice_staticId()));
        com.zeroc.Ice.Value h = initial.getH();
        test(h != null && ((H)h) != null);
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
        try
        {
            initial.throwEDerived();
            test(false);
        }
        catch(EDerived ederived)
        {
            test(ederived.a1.name.equals("a1"));
            test(ederived.a2.name.equals("a2"));
            test(ederived.a3.name.equals("a3"));
            test(ederived.a4.name.equals("a4"));
        }
        out.println("ok");

        out.print("setting G... ");
        out.flush();
        try
        {
            initial.setG(new G(new S("hello"), "g"));
        }
        catch(com.zeroc.Ice.OperationNotExistException ex)
        {
        }
        out.println("ok");

        out.print("setting I... ");
        out.flush();
        initial.setI(i);
        initial.setI(j);
        initial.setI(h);
        out.println("ok");

        out.print("testing sequences...");
        try
        {
            out.flush();
            Base[] inS = new Base[0];
            Initial.OpBaseSeqResult sr = initial.opBaseSeq(inS);
            test(sr.returnValue.length == 0 && sr.outSeq.length == 0);

            inS = new Base[1];
            inS[0] = new Base(new S(), "");
            sr = initial.opBaseSeq(inS);
            test(sr.returnValue.length == 1 && sr.outSeq.length == 1);
        }
        catch(com.zeroc.Ice.OperationNotExistException ex)
        {
        }
        out.println("ok");

        out.print("testing recursive type... ");
        out.flush();
        Recursive top = new Recursive();
        Recursive p = top;
        int depth = 0;
        try
        {
            for(; depth <= 20000; ++depth)
            {
                p.v = new Recursive();
                p = p.v;
                if((depth < 10 && (depth % 10) == 0) ||
                   (depth < 1000 && (depth % 100) == 0) ||
                   (depth < 10000 && (depth % 1000) == 0) ||
                   (depth % 10000) == 0)
                {
                    initial.setRecursive(top);
                }
            }
            test(!initial.supportsClassGraphDepthMax());
        }
        catch(com.zeroc.Ice.UnknownLocalException ex)
        {
            // Expected marshal exception from the server (max class graph depth reached)
        }
        catch(com.zeroc.Ice.UnknownException ex)
        {
            // Expected stack overflow from the server (Java only)
        }
        catch(java.lang.StackOverflowError ex)
        {
            // Stack overflow while writing instances
        }
        initial.setRecursive(new Recursive());
        out.println("ok");

        out.print("testing compact ID...");
        out.flush();
        try
        {
            test(initial.getCompact() != null);
        }
        catch(com.zeroc.Ice.OperationNotExistException ex)
        {
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
        ref = "uoet:" + app.getTestEndpoint(0);
        base = communicator.stringToProxy(ref);
        test(base != null);
        UnexpectedObjectExceptionTestPrx uoet = UnexpectedObjectExceptionTestPrx.uncheckedCast(base);
        test(uoet != null);
        try
        {
            uoet.op();
            test(false);
        }
        catch(com.zeroc.Ice.UnexpectedObjectException ex)
        {
            test(ex.type.equals("::Test::AlsoEmpty"));
            test(ex.expectedType.equals("::Test::Empty"));
        }
        catch(java.lang.Exception ex)
        {
            out.println(ex);
            test(false);
        }
        out.println("ok");

        out.print("testing getting ObjectFactory...");
        out.flush();
        test(communicator.findObjectFactory("TestOF") != null);
        out.println("ok");
        out.print("testing getting ObjectFactory as ValueFactory...");
        test(communicator.getValueFactoryManager().find("TestOF") != null);
        out.println("ok");

        return initial;
    }
}

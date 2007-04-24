// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import Test.*;

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

    public static InitialPrx
    allTests(Ice.Communicator communicator, boolean collocated)
    {
        System.out.print("testing stringToProxy... ");
        System.out.flush();
        String ref = "initial:default -p 12010 -t 10000";
        Ice.ObjectPrx base = communicator.stringToProxy(ref);
        test(base != null);
        System.out.println("ok");

        System.out.print("testing checked cast... ");
        System.out.flush();
        InitialPrx initial = InitialPrxHelper.checkedCast(base);
        test(initial != null);
        test(initial.equals(base));
        System.out.println("ok");

        System.out.print("getting B1... ");
        System.out.flush();
        B b1 = initial.getB1();
        test(b1 != null);
        System.out.println("ok");

        System.out.print("getting B2... ");
        System.out.flush();
        B b2 = initial.getB2();
        test(b2 != null);
        System.out.println("ok");

        System.out.print("getting C... ");
        System.out.flush();
        C c = initial.getC();
        test(c != null);
        System.out.println("ok");

        System.out.print("getting D... ");
        System.out.flush();
        D d = initial.getD();
        test(d != null);
        System.out.println("ok");

        System.out.print("checking consistency... ");
        System.out.flush();
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
        test(((C)(((B)b1.theA).theC)).theB == b1.theA);
        if(!collocated)
        {   
            test(b1.preMarshalInvoked);
            test(b1.postUnmarshalInvoked(null));
            test(b1.theA.preMarshalInvoked);
            test(b1.theA.postUnmarshalInvoked(null));
            test(((B)b1.theA).theC.preMarshalInvoked);
            test(((B)b1.theA).theC.postUnmarshalInvoked(null));
        }
        // More tests possible for b2 and d, but I think this is already
        // sufficient.
        test(b2.theA == b2);
        test(d.theC == null);
        System.out.println("ok");

        System.out.print("getting B1, B2, C, and D all at once... ");
        System.out.flush();
        BHolder b1H = new BHolder();
        BHolder b2H = new BHolder();
        CHolder cH = new CHolder();
        DHolder dH = new DHolder();
        initial.getAll(b1H, b2H, cH, dH);
        b1 = b1H.value;
        b2 = b2H.value;
        c = cH.value;
        d = dH.value;
        test(b1 != null);
        test(b2 != null);
        test(c != null);
        test(d != null);
        System.out.println("ok");

        System.out.print("checking consistency... ");
        System.out.flush();
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
        if(!collocated)
        {   
            test(d.preMarshalInvoked);
            test(d.postUnmarshalInvoked(null));
            test(d.theA.preMarshalInvoked);
            test(d.theA.postUnmarshalInvoked(null)); 
            test(d.theB.preMarshalInvoked);
            test(d.theB.postUnmarshalInvoked(null));
            test(d.theB.theC.preMarshalInvoked);
            test(d.theB.theC.postUnmarshalInvoked(null));
        }
        System.out.println("ok");

        System.out.print("getting I, J and H... ");
        System.out.flush();
        I i = initial.getI();
        test(i != null);
        I j = initial.getJ();
        test(j != null && ((J)j) != null);
        I h = initial.getH();
        test(h != null && ((H)h) != null);
        System.out.println("ok");

        System.out.print("setting I... ");
        System.out.flush();
        initial.setI(i);
        initial.setI(j);
        initial.setI(h);
        System.out.println("ok");

        if(!collocated)
        {
            System.out.print("testing UnexpectedObjectException...");
            System.out.flush();
            ref = "uoet:default -p 12010 -t 10000";
            base = communicator.stringToProxy(ref);
            test(base != null);
            UnexpectedObjectExceptionTestPrx uoet = UnexpectedObjectExceptionTestPrxHelper.uncheckedCast(base);
            test(uoet != null);
            try
            {
                uoet.op();
                test(false);
            }
            catch(Ice.UnexpectedObjectException ex)
            {
                test(ex.type.equals("::Test::AlsoEmpty"));
                test(ex.expectedType.equals("::Test::Empty"));
            }
            catch(java.lang.Exception ex)
            {
                System.out.println(ex);
                test(false);
            }
            System.out.println("ok");
        }

        return initial;
    }
}

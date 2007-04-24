// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using Test;

public class AllTests
{
    private static void test(bool b)
    {
        if(!b)
        {
            throw new Exception();
        }
    }
    
    public static InitialPrx allTests(Ice.Communicator communicator, bool collocated)
    {
        Console.Out.Write("testing stringToProxy... ");
        Console.Out.Flush();
        String @ref = "initial:default -p 12010 -t 2000";
        Ice.ObjectPrx @base = communicator.stringToProxy(@ref);
        test(@base != null);
        Console.Out.WriteLine("ok");
        
        Console.Out.Write("testing checked cast... ");
        Console.Out.Flush();
        InitialPrx initial = InitialPrxHelper.checkedCast(@base);
        test(initial != null);
        test(initial.Equals(@base));
        Console.Out.WriteLine("ok");
        
        Console.Out.Write("getting B1... ");
        Console.Out.Flush();
        B b1 = initial.getB1();
        test(b1 != null);
        Console.Out.WriteLine("ok");
        
        Console.Out.Write("getting B2... ");
        Console.Out.Flush();
        B b2 = initial.getB2();
        test(b2 != null);
        Console.Out.WriteLine("ok");
        
        Console.Out.Write("getting C... ");
        Console.Out.Flush();
        C c = initial.getC();
        test(c != null);
        Console.Out.WriteLine("ok");
        
        Console.Out.Write("getting D... ");
        Console.Out.Flush();
        D d = initial.getD();
        test(d != null);
        Console.Out.WriteLine("ok");
        
        Console.Out.Write("checking consistency... ");
        Console.Out.Flush();
        test(b1 != b2);
        //test(b1 != c);
        //test(b1 != d);
        //test(b2 != c);
        //test(b2 != d);
        //test(c != d);
        test(b1.theB == b1);
        test(b1.theC == null);
        test(b1.theA is B);
        test(((B) b1.theA).theA == b1.theA);
        test(((B) b1.theA).theB == b1);
        //test(((B)b1.theA).theC is C); // Redundant -- theC is always of type C
        test(((C) (((B) b1.theA).theC)).theB == b1.theA);
        if(!collocated)
        {
            test(b1.preMarshalInvoked);
            test(b1.postUnmarshalInvoked());
            test(b1.theA.preMarshalInvoked);
            test(b1.theA.postUnmarshalInvoked());
            test(((B)b1.theA).theC.preMarshalInvoked);
            test(((B)b1.theA).theC.postUnmarshalInvoked());
        }
        // More tests possible for b2 and d, but I think this is already
        // sufficient.
        test(b2.theA == b2);
        test(d.theC == null);
        Console.Out.WriteLine("ok");
        
        Console.Out.Write("getting B1, B2, C, and D all at once... ");
        Console.Out.Flush();
        B b1out;
        B b2out;
        C cout;
        D dout;
        initial.getAll(out b1out, out b2out, out cout, out dout);
        test(b1out != null);
        test(b2out != null);
        test(cout != null);
        test(dout != null);
        Console.Out.WriteLine("ok");
        
        Console.Out.Write("checking consistency... ");
        Console.Out.Flush();
        test(b1out != b2out);
        test(b1out.theA == b2out);
        test(b1out.theB == b1out);
        test(b1out.theC == null);
        test(b2out.theA == b2out);
        test(b2out.theB == b1out);
        test(b2out.theC == cout);
        test(cout.theB == b2out);
        test(dout.theA == b1out);
        test(dout.theB == b2out);
        test(dout.theC == null);
        if(!collocated)
        {
            test(dout.preMarshalInvoked);
            test(dout.postUnmarshalInvoked());
            test(dout.theA.preMarshalInvoked);
            test(dout.theA.postUnmarshalInvoked()); 
            test(dout.theB.preMarshalInvoked);
            test(dout.theB.postUnmarshalInvoked());
            test(dout.theB.theC.preMarshalInvoked);
            test(dout.theB.theC.postUnmarshalInvoked());
        }
        Console.Out.WriteLine("ok");

        Console.Out.Write("getting I, J and H... ");
        Console.Out.Flush();
        I i = initial.getI();
        test(i != null);
        I j = initial.getJ();
        test(j != null && ((J)j) != null);
        I h = initial.getH();
        test(h != null && ((H)h) != null);
        Console.Out.WriteLine("ok");

        Console.Out.Write("setting I... ");
        Console.Out.Flush();
        initial.setI(i);
        initial.setI(j);
        initial.setI(h);
        Console.Out.WriteLine("ok");

        if(!collocated)
        {
            Console.Out.Write("testing UnexpectedObjectException...");
            Console.Out.Flush();
            @ref = "uoet:default -p 12010 -t 10000";
            @base = communicator.stringToProxy(@ref);
            test(@base != null);
            UnexpectedObjectExceptionTestPrx uoet = UnexpectedObjectExceptionTestPrxHelper.uncheckedCast(@base);
            test(uoet != null);
            try
            {
                uoet.op();
                test(false);
            }
            catch(Ice.UnexpectedObjectException ex)
            {
                test(ex.type.Equals("::Test::AlsoEmpty"));
                test(ex.expectedType.Equals("::Test::Empty"));
            }
            catch(System.Exception ex)
            {
                Console.Out.WriteLine(ex);
                test(false);
            }
            Console.Out.WriteLine("ok");
        }

        return initial;
    }
}

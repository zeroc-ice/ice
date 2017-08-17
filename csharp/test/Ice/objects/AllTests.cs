// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Diagnostics;
using Test;

#if SILVERLIGHT
using System.Windows.Controls;
#endif

public class AllTests : TestCommon.TestApp
{
    private class MyObjectFactory : Ice.ObjectFactory
    {
        public Ice.Object create(string type)
        {
            if (type.Equals("::Test::B"))
            {
                return new BI();
            }
            else if (type.Equals("::Test::C"))
            {
                return new CI();
            }
            else if (type.Equals("::Test::D"))
            {
                return new DI();
            }
            else if (type.Equals("::Test::E"))
            {
                return new EI();
            }
            else if (type.Equals("::Test::F"))
            {
                return new FI();
            }
            else if (type.Equals("::Test::I"))
            {
                return new II();
            }
            else if (type.Equals("::Test::J"))
            {
                return new JI();
            }
            else if (type.Equals("::Test::H"))
            {
                return new HI();
            }
            Debug.Assert(false); // Should never be reached
            return null;
        }

        public void
        destroy()
        {
            // Nothing to do
        }
    }
#if SILVERLIGHT
    public override Ice.InitializationData initData()
    {
        Ice.InitializationData initData = new Ice.InitializationData();
        initData.properties = Ice.Util.createProperties();
        initData.properties.setProperty("Ice.FactoryAssemblies", "objects,version=1.0.0.0");
        return initData;
    }

    override
    public void run(Ice.Communicator communicator)
#else
    public static InitialPrx allTests(Ice.Communicator communicator)
#endif
    {
        Ice.ObjectFactory factory = new MyObjectFactory();
        communicator.addObjectFactory(factory, "::Test::B");
        communicator.addObjectFactory(factory, "::Test::C");
        communicator.addObjectFactory(factory, "::Test::D");
        communicator.addObjectFactory(factory, "::Test::E");
        communicator.addObjectFactory(factory, "::Test::F");
        communicator.addObjectFactory(factory, "::Test::I");
        communicator.addObjectFactory(factory, "::Test::J");
        communicator.addObjectFactory(factory, "::Test::H");

        Write("testing stringToProxy... ");
        Flush();
        String @ref = "initial:default -p 12010";
        Ice.ObjectPrx @base = communicator.stringToProxy(@ref);
        test(@base != null);
        WriteLine("ok");

        Write("testing checked cast... ");
        Flush();
        InitialPrx initial = InitialPrxHelper.checkedCast(@base);
        test(initial != null);
        test(initial.Equals(@base));
        WriteLine("ok");

        Write("getting B1... ");
        Flush();
        B b1 = initial.getB1();
        test(b1 != null);
        WriteLine("ok");

        Write("getting B2... ");
        Flush();
        B b2 = initial.getB2();
        test(b2 != null);
        WriteLine("ok");

        Write("getting C... ");
        Flush();
        C c = initial.getC();
        test(c != null);
        WriteLine("ok");

        Write("getting D... ");
        Flush();
        D d = initial.getD();
        test(d != null);
        WriteLine("ok");

        Write("checking consistency... ");
        Flush();
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
        test(b1.preMarshalInvoked);
        test(b1.postUnmarshalInvoked());
        test(b1.theA.preMarshalInvoked);
        test(b1.theA.postUnmarshalInvoked());
        test(((B)b1.theA).theC.preMarshalInvoked);
        test(((B)b1.theA).theC.postUnmarshalInvoked());

        // More tests possible for b2 and d, but I think this is already
        // sufficient.
        test(b2.theA == b2);
        test(d.theC == null);
        WriteLine("ok");

        Write("getting B1, B2, C, and D all at once... ");
        Flush();
        B b1out;
        B b2out;
        C cout;
        D dout;
        initial.getAll(out b1out, out b2out, out cout, out dout);
        test(b1out != null);
        test(b2out != null);
        test(cout != null);
        test(dout != null);
        WriteLine("ok");

        Write("checking consistency... ");
        Flush();
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
        test(dout.preMarshalInvoked);
        test(dout.postUnmarshalInvoked());
        test(dout.theA.preMarshalInvoked);
        test(dout.theA.postUnmarshalInvoked());
        test(dout.theB.preMarshalInvoked);
        test(dout.theB.postUnmarshalInvoked());
        test(dout.theB.theC.preMarshalInvoked);
        test(dout.theB.theC.postUnmarshalInvoked());

        WriteLine("ok");

        Write("testing protected members... ");
        Flush();
        E e = initial.getE();
        test(e.checkValues());
        System.Reflection.BindingFlags flags = System.Reflection.BindingFlags.NonPublic |
                                               System.Reflection.BindingFlags.Public |
                                               System.Reflection.BindingFlags.Instance;
        test(!typeof(E).GetField("i", flags).IsPublic && !typeof(E).GetField("i", flags).IsPrivate);
        test(!typeof(E).GetField("s", flags).IsPublic && !typeof(E).GetField("s", flags).IsPrivate);
        F f = initial.getF();
        test(f.checkValues());
        test(f.e2.checkValues());
        test(!typeof(F).GetField("e1", flags).IsPublic && !typeof(F).GetField("e1", flags).IsPrivate);
        test(typeof(F).GetField("e2", flags).IsPublic && !typeof(F).GetField("e2", flags).IsPrivate);
        WriteLine("ok");

        Write("getting I, J and H... ");
        Flush();
        I i = initial.getI();
        test(i != null);
        I j = initial.getJ();
        test(j != null && ((J)j) != null);
        I h = initial.getH();
        test(h != null && ((H)h) != null);
        WriteLine("ok");

        Write("getting D1... ");
        Flush();
        D1 d1 = new D1(new A1("a1"), new A1("a2"), new A1("a3"), new A1("a4"));
        d1 = initial.getD1(d1);
        test(d1.a1.name.Equals("a1"));
        test(d1.a2.name.Equals("a2"));
        test(d1.a3.name.Equals("a3"));
        test(d1.a4.name.Equals("a4"));
        WriteLine("ok");

        Write("throw EDerived... ");
        Flush();
        try
        {
            initial.throwEDerived();
            test(false);
        }
        catch(EDerived ederived)
        {
            test(ederived.a1.name.Equals("a1"));
            test(ederived.a2.name.Equals("a2"));
            test(ederived.a3.name.Equals("a3"));
            test(ederived.a4.name.Equals("a4"));
        }
        WriteLine("ok");

        Write("setting I... ");
        Flush();
        initial.setI(i);
        initial.setI(j);
        initial.setI(h);
        WriteLine("ok");

        Write("testing sequences...");
        Flush();
        try
        {
            Base[] inS = new Base[0];
            Base[] outS;
            Base[] retS;
            retS = initial.opBaseSeq(inS, out outS);

            inS = new Base[1];
            inS[0] = new Base(new S(), "");
            retS = initial.opBaseSeq(inS, out outS);
            test(retS.Length == 1 && outS.Length == 1);
        }
        catch(Ice.OperationNotExistException)
        {
        }
        WriteLine("ok");

        Write("testing recursive type... ");
        Flush();
        try
        {
            Recursive top = new Recursive();
            Recursive p = top;
            int depth = 0;
            try
            {
                for(; depth <= 1000; ++depth)
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
            catch(Ice.UnknownLocalException)
            {
                // Expected marshal exception from the server (max class graph depth reached)
            }
            catch(Ice.UnknownException)
            {
                // Expected stack overflow from the server (Java only)
            }
            initial.setRecursive(new Recursive());
        }
        catch(Ice.OperationNotExistException)
        {
            // Expected if running against server that doesn't support this method.
        }
        WriteLine("ok");

        Write("testing compact ID...");
        Flush();
        try
        {
            test(initial.getCompact() != null);
        }
        catch(Ice.OperationNotExistException)
        {
        }
        WriteLine("ok");

        Write("testing UnexpectedObjectException...");
        Flush();
        @ref = "uoet:default -p 12010";
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
            WriteLine(ex.ToString());
            test(false);
        }
        WriteLine("ok");
#if SILVERLIGHT
        initial.shutdown();
#else
        return initial;
#endif
    }
}

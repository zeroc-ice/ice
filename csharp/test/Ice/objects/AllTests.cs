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

public class AllTests : TestCommon.AllTests
{
    public static Ice.Value MyValueFactory(string type)
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

    private class MyObjectFactory : Ice.ObjectFactory
    {
        public MyObjectFactory()
        {
            _destroyed = false;
        }

        ~MyObjectFactory()
        {
            Debug.Assert(_destroyed);
        }

        public Ice.Value create(string type)
        {
            return null;
        }

        public void
        destroy()
        {
            _destroyed = true;
        }

        private bool _destroyed;
    }

    public static InitialPrx allTests(TestCommon.Application app)
    {
        Ice.Communicator communicator = app.communicator();
        communicator.getValueFactoryManager().add(MyValueFactory, "::Test::B");
        communicator.getValueFactoryManager().add(MyValueFactory, "::Test::C");
        communicator.getValueFactoryManager().add(MyValueFactory, "::Test::D");
        communicator.getValueFactoryManager().add(MyValueFactory, "::Test::E");
        communicator.getValueFactoryManager().add(MyValueFactory, "::Test::F");
        communicator.getValueFactoryManager().add(MyValueFactory, "::Test::I");
        communicator.getValueFactoryManager().add(MyValueFactory, "::Test::J");
        communicator.getValueFactoryManager().add(MyValueFactory, "::Test::H");

// Disable Obsolete warning/error
#pragma warning disable 612, 618
        communicator.addObjectFactory(new MyObjectFactory(), "TestOF");
#pragma warning restore 612, 618


        Write("testing stringToProxy... ");
        Flush();
        String @ref = "initial:" + app.getTestEndpoint(0);
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
        test(b1.postUnmarshalInvoked);
        test(b1.theA.preMarshalInvoked);
        test(b1.theA.postUnmarshalInvoked);
        test(((B)b1.theA).theC.preMarshalInvoked);
        test(((B)b1.theA).theC.postUnmarshalInvoked);

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
        test(dout.postUnmarshalInvoked);
        test(dout.theA.preMarshalInvoked);
        test(dout.theA.postUnmarshalInvoked);
        test(dout.theB.preMarshalInvoked);
        test(dout.theB.postUnmarshalInvoked);
        test(dout.theB.theC.preMarshalInvoked);
        test(dout.theB.theC.postUnmarshalInvoked);

        WriteLine("ok");

        Write("testing protected members... ");
        Flush();
        EI e = (EI)initial.getE();
        test(e != null && e.checkValues());
        System.Reflection.BindingFlags flags = System.Reflection.BindingFlags.NonPublic |
                                               System.Reflection.BindingFlags.Public |
                                               System.Reflection.BindingFlags.Instance;
        test(!typeof(E).GetField("i", flags).IsPublic && !typeof(E).GetField("i", flags).IsPrivate);
        test(!typeof(E).GetField("s", flags).IsPublic && !typeof(E).GetField("s", flags).IsPrivate);
        FI f = (FI)initial.getF();
        test(f.checkValues());
        test(((EI)f.e2).checkValues());
        test(!typeof(F).GetField("e1", flags).IsPublic && !typeof(F).GetField("e1", flags).IsPrivate);
        test(typeof(F).GetField("e2", flags).IsPublic && !typeof(F).GetField("e2", flags).IsPrivate);
        WriteLine("ok");

        Write("getting I, J and H... ");
        Flush();
        var i = initial.getI();
        test(i != null);
        var j = initial.getJ();
        test(j != null);
        var h = initial.getH();
        test(h != null);
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

        Write("testing marshaled results...");
        Flush();
        b1 = initial.getMB();
        test(b1 != null && b1.theB == b1);
        b1 = initial.getAMDMBAsync().Result;
        test(b1 != null && b1.theB == b1);
        WriteLine("ok");

        Write("testing UnexpectedObjectException...");
        Flush();
        @ref = "uoet:" + app.getTestEndpoint(0);
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

// Disable Obsolete warning/error
#pragma warning disable 612, 618
        Write("testing getting ObjectFactory...");
        Flush();
        test(communicator.findObjectFactory("TestOF") != null);
        WriteLine("ok");
        Write("testing getting ObjectFactory as ValueFactory...");
        Flush();
        test(communicator.getValueFactoryManager().find("TestOF") != null);
        WriteLine("ok");
#pragma warning restore 612, 618

        return initial;
    }
}

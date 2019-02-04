//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Diagnostics;
using System.Collections.Generic;

namespace Ice
{
    namespace objects
    {
        namespace Test
        {
            public partial class IBase
            {
                partial void ice_initialize()
                {
                    id = "My id";
                }
            }

            public partial class IDerived
            {
                partial void ice_initialize()
                {
                    name = "My name";
                }
            }

            public partial class I2
            {
                public bool called
                {
                    get;
                    set;
                }

                partial void ice_initialize()
                {
                    called = true;
                }
            }

            public partial struct S1
            {
                partial void ice_initialize()
                {
                    id = 1;
                }
            }

            public partial class SC1
            {
                partial void ice_initialize()
                {
                    id = "My id";
                }
            }

            public class AllTests : global::Test.AllTests
            {
                public static Ice.Value MyValueFactory(string type)
                {
                    if(type.Equals("::Test::B"))
                    {
                        return new BI();
                    }
                    else if(type.Equals("::Test::C"))
                    {
                        return new CI();
                    }
                    else if(type.Equals("::Test::D"))
                    {
                        return new DI();
                    }
                    else if(type.Equals("::Test::E"))
                    {
                        return new EI();
                    }
                    else if(type.Equals("::Test::F"))
                    {
                        return new FI();
                    }
                    else if(type.Equals("::Test::I"))
                    {
                        return new II();
                    }
                    else if(type.Equals("::Test::J"))
                    {
                        return new JI();
                    }
                    else if(type.Equals("::Test::H"))
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

                public static Test.InitialPrx allTests(global::Test.TestHelper helper)
                {
                    Ice.Communicator communicator = helper.communicator();
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

                    var output = helper.getWriter();

                    output.Write("testing stringToProxy... ");
                    output.Flush();
                    String @ref = "initial:" + helper.getTestEndpoint(0);
                    Ice.ObjectPrx @base = communicator.stringToProxy(@ref);
                    test(@base != null);
                    output.WriteLine("ok");

                    output.Write("testing checked cast... ");
                    output.Flush();
                    var initial = Test.InitialPrxHelper.checkedCast(@base);
                    test(initial != null);
                    test(initial.Equals(@base));
                    output.WriteLine("ok");

                    output.Write("getting B1... ");
                    output.Flush();
                    B b1 = initial.getB1();
                    test(b1 != null);
                    output.WriteLine("ok");

                    output.Write("getting B2... ");
                    output.Flush();
                    B b2 = initial.getB2();
                    test(b2 != null);
                    output.WriteLine("ok");

                    output.Write("getting C... ");
                    output.Flush();
                    C c = initial.getC();
                    test(c != null);
                    output.WriteLine("ok");

                    output.Write("getting D... ");
                    output.Flush();
                    D d = initial.getD();
                    test(d != null);
                    output.WriteLine("ok");

                    output.Write("checking consistency... ");
                    output.Flush();
                    test(b1 != b2);
                    //test(b1 != c);
                    //test(b1 != d);
                    //test(b2 != c);
                    //test(b2 != d);
                    //test(c != d);
                    test(b1.theB == b1);
                    test(b1.theC == null);
                    test(b1.theA is B);
                    test(((B)b1.theA).theA == b1.theA);
                    test(((B)b1.theA).theB == b1);
                    //test(((B)b1.theA).theC is C); // Redundant -- theC is always of type C
                    test(((C)(((B)b1.theA).theC)).theB == b1.theA);
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
                    output.WriteLine("ok");

                    output.Write("getting B1, B2, C, and D all at once... ");
                    output.Flush();
                    B b1out;
                    B b2out;
                    C cout;
                    D dout;
                    initial.getAll(out b1out, out b2out, out cout, out dout);
                    test(b1out != null);
                    test(b2out != null);
                    test(cout != null);
                    test(dout != null);
                    output.WriteLine("ok");

                    output.Write("checking consistency... ");
                    output.Flush();
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

                    output.WriteLine("ok");

                    output.Write("testing protected members... ");
                    output.Flush();
                    EI e =(EI)initial.getE();
                    test(e != null && e.checkValues());
                    System.Reflection.BindingFlags flags = System.Reflection.BindingFlags.NonPublic |
                                                           System.Reflection.BindingFlags.Public |
                                                           System.Reflection.BindingFlags.Instance;
                    test(!typeof(E).GetField("i", flags).IsPublic && !typeof(E).GetField("i", flags).IsPrivate);
                    test(!typeof(E).GetField("s", flags).IsPublic && !typeof(E).GetField("s", flags).IsPrivate);
                    FI f =(FI)initial.getF();
                    test(f.checkValues());
                    test(((EI)f.e2).checkValues());
                    test(!typeof(F).GetField("e1", flags).IsPublic && !typeof(F).GetField("e1", flags).IsPrivate);
                    test(typeof(F).GetField("e2", flags).IsPublic && !typeof(F).GetField("e2", flags).IsPrivate);
                    output.WriteLine("ok");

                    output.Write("getting I, J and H... ");
                    output.Flush();
                    var i = initial.getI();
                    test(i != null);
                    var j = initial.getJ();
                    test(j != null);
                    var h = initial.getH();
                    test(h != null);
                    output.WriteLine("ok");

                    output.Write("getting K... ");
                    {
                        output.Flush();
                        var k = initial.getK();
                        var l = k.value as L;
                        test(l != null);
                        test(l.data.Equals("l"));
                    }
                    output.WriteLine("ok");

                    output.Write("testing Value as parameter... ");
                    output.Flush();
                    {
                        Ice.Value v1 = new L("l");
                        Ice.Value v2;
                        Ice.Value v3 = initial.opValue(v1, out v2);
                        test(((L)v2).data.Equals("l"));
                        test(((L)v3).data.Equals("l"));
                    }
                    {
                        L l = new L("l");
                        Ice.Value[] v1 = new Ice.Value[]{ l };
                        Ice.Value[] v2;
                        Ice.Value[] v3 = initial.opValueSeq(v1, out v2);
                        test(((L)v2[0]).data.Equals("l"));
                        test(((L)v3[0]).data.Equals("l"));
                    }
                    {
                        L l = new L("l");
                        Dictionary<string, Ice.Value> v1 = new Dictionary<string, Ice.Value>{ {"l", l} };
                        Dictionary<string, Ice.Value> v2;
                        Dictionary<string, Ice.Value> v3 = initial.opValueMap(v1, out v2);
                        test(((L)v2["l"]).data.Equals("l"));
                        test(((L)v3["l"]).data.Equals("l"));
                    }
                    output.WriteLine("ok");

                    output.Write("getting D1... ");
                    output.Flush();
                    D1 d1 = new D1(new A1("a1"), new A1("a2"), new A1("a3"), new A1("a4"));
                    d1 = initial.getD1(d1);
                    test(d1.a1.name.Equals("a1"));
                    test(d1.a2.name.Equals("a2"));
                    test(d1.a3.name.Equals("a3"));
                    test(d1.a4.name.Equals("a4"));
                    output.WriteLine("ok");

                    output.Write("throw EDerived... ");
                    output.Flush();
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
                    output.WriteLine("ok");

                    output.Write("setting G... ");
                    output.Flush();
                    try
                    {
                        initial.setG(new G(new S("hello"), "g"));
                    }
                    catch(Ice.OperationNotExistException)
                    {
                    }
                    output.WriteLine("ok");

                    output.Write("setting I... ");
                    output.Flush();
                    initial.setI(i);
                    initial.setI(j);
                    initial.setI(h);
                    output.WriteLine("ok");

                    output.Write("testing sequences...");
                    output.Flush();
                    try
                    {
                        Base[] inS = new Test.Base[0];
                        Base[] outS;
                        Base[] retS;
                        retS = initial.opBaseSeq(inS, out outS);

                        inS = new Test.Base[1];
                        inS[0] = new Test.Base(new S(), "");
                        retS = initial.opBaseSeq(inS, out outS);
                        test(retS.Length == 1 && outS.Length == 1);
                    }
                    catch(Ice.OperationNotExistException)
                    {
                    }
                    output.WriteLine("ok");

                    output.Write("testing recursive type... ");
                    output.Flush();
                    var top = new Test.Recursive();
                    var p = top;
                    int depth = 0;
                    try
                    {
                        for(; depth <= 1000; ++depth)
                        {
                            p.v = new Test.Recursive();
                            p = p.v;
                            if((depth < 10 &&(depth % 10) == 0) ||
                              (depth < 1000 &&(depth % 100) == 0) ||
                              (depth < 10000 &&(depth % 1000) == 0) ||
                              (depth % 10000) == 0)
                            {
                                initial.setRecursive(top);
                            }
                        }
                        test(!initial.supportsClassGraphDepthMax());
                    }
                    catch(Ice.UnknownLocalException)
                    {
                        // Expected marshal exception from the server(max class graph depth reached)
                    }
                    catch(Ice.UnknownException)
                    {
                        // Expected stack overflow from the server(Java only)
                    }
                    initial.setRecursive(new Test.Recursive());
                    output.WriteLine("ok");

                    output.Write("testing compact ID...");
                    output.Flush();
                    try
                    {
                        test(initial.getCompact() != null);
                    }
                    catch(Ice.OperationNotExistException)
                    {
                    }
                    output.WriteLine("ok");

                    output.Write("testing marshaled results...");
                    output.Flush();
                    b1 = initial.getMB();
                    test(b1 != null && b1.theB == b1);
                    b1 = initial.getAMDMBAsync().Result;
                    test(b1 != null && b1.theB == b1);
                    output.WriteLine("ok");

                    output.Write("testing UnexpectedObjectException...");
                    output.Flush();
                    @ref = "uoet:" + helper.getTestEndpoint(0);
                    @base = communicator.stringToProxy(@ref);
                    test(@base != null);
                    var uoet = Test.UnexpectedObjectExceptionTestPrxHelper.uncheckedCast(@base);
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
                        output.WriteLine(ex.ToString());
                        test(false);
                    }
                    output.WriteLine("ok");

                    // Disable Obsolete warning/error
#pragma warning disable 612, 618
                    output.Write("testing getting ObjectFactory...");
                    output.Flush();
                    test(communicator.findObjectFactory("TestOF") != null);
                    output.WriteLine("ok");
                    output.Write("testing getting ObjectFactory as ValueFactory...");
                    output.Flush();
                    test(communicator.getValueFactoryManager().find("TestOF") != null);
                    output.WriteLine("ok");
#pragma warning restore 612, 618

                    output.Write("testing partial ice_initialize...");
                    output.Flush();
                    var ib1 = new IBase();
                    test(ib1.id.Equals("My id"));
                    var id1 = new IDerived();
                    test(id1.id.Equals("My id"));
                    test(id1.name.Equals("My name"));

                    var id2 = new Test.IDerived2();
                    test(id2.id.Equals("My id"));
                    var i2 = new I2();
                    test(i2.called);

                    var s1 = new S1();
                    // The struct default constructor do not call ice_initialize
                    test(s1.id == 0);
                    s1 = new S1(2);
                    // The id should have the value set by ice_initialize and not 2
                    test(s1.id == 1);

                    var sc1 = new SC1();
                    test(sc1.id.Equals("My id"));
                    output.WriteLine("ok");

                    output.Write("testing class containing complex dictionary... ");
                    output.Flush();
                    {
                        var m = new Test.M();
                        m.v = new Dictionary<StructKey, L>();
                        var k1 = new StructKey(1, "1");
                        m.v[k1] = new L("one");
                        var k2 = new StructKey(2, "2");
                        m.v[k2] = new L("two");
                        Test.M m1;
                        var m2 = initial.opM(m, out m1);
                        test(m1.v.Count == 2);
                        test(m2.v.Count == 2);

                        test(m1.v[k1].data.Equals("one"));
                        test(m2.v[k1].data.Equals("one"));

                        test(m1.v[k2].data.Equals("two"));
                        test(m2.v[k2].data.Equals("two"));

                    }
                    output.WriteLine("ok");
                    return initial;
                }
            }
        }
    }
}

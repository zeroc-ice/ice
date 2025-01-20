// Copyright (c) ZeroC, Inc.

using System.Diagnostics;

namespace Ice
{
    namespace objects
    {
        namespace Test
        {
            public partial class IBase
            {
                partial void ice_initialize() => id = "My id";
            }

            public partial class IDerived
            {
                partial void ice_initialize() => name = "My name";
            }

            public partial class I2
            {
                public bool called
                {
                    get;
                    set;
                }

                partial void ice_initialize() => called = true;
            }

            public partial record struct S1
            {
                partial void ice_initialize() => id = 1;
            }

            public partial record class SC1
            {
                partial void ice_initialize() => id = "My id";
            }

            public class AllTests : global::Test.AllTests
            {
                public static Value MyValueFactory(string type)
                {
                    if (type == "::Test::B")
                    {
                        return new BI();
                    }
                    else if (type == "::Test::C")
                    {
                        return new CI();
                    }
                    else if (type == "::Test::D")
                    {
                        return new DI();
                    }
                    else if (type == "::Test::E")
                    {
                        return new EI();
                    }
                    else if (type == "::Test::F")
                    {
                        return new FI();
                    }
                    Debug.Assert(false); // Should never be reached
                    return null;
                }

                public static InitialPrx allTests(global::Test.TestHelper helper)
                {
                    Communicator communicator = helper.communicator();
                    communicator.getValueFactoryManager().add(MyValueFactory, "::Test::B");
                    communicator.getValueFactoryManager().add(MyValueFactory, "::Test::C");
                    communicator.getValueFactoryManager().add(MyValueFactory, "::Test::D");
                    communicator.getValueFactoryManager().add(MyValueFactory, "::Test::E");
                    communicator.getValueFactoryManager().add(MyValueFactory, "::Test::F");
                    communicator.getValueFactoryManager().add(MyValueFactory, "::Test::H");

                    var output = helper.getWriter();

                    output.Write("testing stringToProxy... ");
                    output.Flush();
                    string @ref = "initial:" + helper.getTestEndpoint(0);
                    ObjectPrx @base = communicator.stringToProxy(@ref);
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
                    // test(b1 != c);
                    // test(b1 != d);
                    // test(b2 != c);
                    // test(b2 != d);
                    // test(c != d);
                    test(b1.theB == b1);
                    test(b1.theC == null);
                    test(b1.theA is B);
                    test(((B)b1.theA).theA == b1.theA);
                    test(((B)b1.theA).theB == b1);
                    // test(((B)b1.theA).theC is C); // Redundant -- theC is always of type C
                    test(((C)((B)b1.theA).theC).theB == b1.theA);
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
                    initial.getAll(out B b1out, out B b2out, out C cout, out D dout);
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

                    output.Write("getting K... ");
                    {
                        output.Flush();
                        var k = initial.getK();
                        var l = k.value as L;
                        test(l != null);
                        test(l.data == "l");
                    }
                    output.WriteLine("ok");

                    output.Write("testing Value as parameter... ");
                    output.Flush();
                    {
                        Value v1 = new L("l");
                        Value v3 = initial.opValue(v1, out Value v2);
                        test(((L)v2).data == "l");
                        test(((L)v3).data == "l");
                    }
                    {
                        var l = new L("l");
                        Value[] v1 = [l];
                        Value[] v3 = initial.opValueSeq(v1, out Value[] v2);
                        test(((L)v2[0]).data == "l");
                        test(((L)v3[0]).data == "l");
                    }
                    {
                        var l = new L("l");
                        var v1 = new Dictionary<string, Value> { { "l", l } };
                        Dictionary<string, Value> v3 = initial.opValueMap(v1, out Dictionary<string, Value> v2);
                        test(((L)v2["l"]).data == "l");
                        test(((L)v3["l"]).data == "l");
                    }
                    output.WriteLine("ok");

                    output.Write("getting D1... ");
                    output.Flush();
                    var d1 = new D1(new A1("a1"), new A1("a2"), new A1("a3"), new A1("a4"));
                    d1 = initial.getD1(d1);
                    test(d1.a1.name == "a1");
                    test(d1.a2.name == "a2");
                    test(d1.a3.name == "a3");
                    test(d1.a4.name == "a4");
                    output.WriteLine("ok");

                    output.Write("throw EDerived... ");
                    output.Flush();
                    try
                    {
                        initial.throwEDerived();
                        test(false);
                    }
                    catch (EDerived ederived)
                    {
                        test(ederived.a1.name == "a1");
                        test(ederived.a2.name == "a2");
                        test(ederived.a3.name == "a3");
                        test(ederived.a4.name == "a4");
                    }
                    output.WriteLine("ok");

                    output.Write("setting G... ");
                    output.Flush();
                    try
                    {
                        initial.setG(new G(new S("hello"), "g"));
                    }
                    catch (Ice.OperationNotExistException)
                    {
                    }
                    output.WriteLine("ok");

                    output.Write("testing sequences...");
                    output.Flush();
                    try
                    {
                        var inS = new Base[0];
                        Base[] retS;
                        retS = initial.opBaseSeq(inS, out Base[] outS);

                        inS = new Base[1];
                        inS[0] = new Base(new S(""), "");
                        retS = initial.opBaseSeq(inS, out outS);
                        test(retS.Length == 1 && outS.Length == 1);
                    }
                    catch (Ice.OperationNotExistException)
                    {
                    }
                    output.WriteLine("ok");

                    output.Write("testing recursive type... ");
                    output.Flush();
                    var top = new Test.Recursive();
                    var bottom = top;
                    int maxDepth = 10;
                    for (int i = 1; i < maxDepth; i++)
                    {
                        bottom.v = new Test.Recursive();
                        bottom = bottom.v;
                    }
                    initial.setRecursive(top);

                    // Adding one more level would exceed the max class graph depth
                    bottom.v = new Test.Recursive();
                    bottom = bottom.v;
                    try
                    {
                        initial.setRecursive(top);
                        test(false);
                    }
                    catch (Ice.UnknownLocalException)
                    {
                        // Expected marshal exception from the server(max class graph depth reached)
                    }
                    initial.setRecursive(new Test.Recursive());
                    output.WriteLine("ok");

                    output.Write("testing compact ID...");
                    output.Flush();
                    try
                    {
                        test(initial.getCompact() != null);
                    }
                    catch (Ice.OperationNotExistException)
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
                    catch (Ice.MarshalException ex)
                    {
                        test(ex.Message.Contains("'::Test::AlsoEmpty'"));
                        test(ex.Message.Contains("'::Test::Empty'"));
                    }
                    catch (System.Exception ex)
                    {
                        output.WriteLine(ex.ToString());
                        test(false);
                    }
                    output.WriteLine("ok");

                    output.Write("testing partial ice_initialize...");
                    output.Flush();
                    var ib1 = new IBase();
                    test(ib1.id == "My id");
                    var id1 = new IDerived();
                    test(id1.id == "My id");
                    test(id1.name == "My name");

                    var id2 = new Test.IDerived2();
                    test(id2.id == "My id");
                    var i2 = new I2();
                    test(i2.called);

                    var s1 = new S1();
                    // The struct default constructor do not call ice_initialize
                    test(s1.id == 0);
                    s1 = new S1(2);
                    // The id should have the value set by ice_initialize and not 2
                    test(s1.id == 1);

                    var sc1 = new SC1();
                    test(sc1.id == "My id");
                    output.WriteLine("ok");

                    output.Write("testing class containing complex dictionary... ");
                    output.Flush();
                    {
                        var m = new M();
                        m.v = [];
                        var k1 = new StructKey(1, "1");
                        m.v[k1] = new L("one");
                        var k2 = new StructKey(2, "2");
                        m.v[k2] = new L("two");
                        var m2 = initial.opM(m, out M m1);
                        test(m1.v.Count == 2);
                        test(m2.v.Count == 2);

                        test(m1.v[k1].data == "one");
                        test(m2.v[k1].data == "one");

                        test(m1.v[k2].data == "two");
                        test(m2.v[k2].data == "two");

                    }
                    output.WriteLine("ok");

                    output.Write("testing forward declared types... ");
                    output.Flush();
                    {
                        F1 f11 = initial.opF1(new F1("F11"), out F1 f12);
                        test(f11.name == "F11");
                        test(f12.name == "F12");

                        F2Prx f21 = initial.opF2(
                            F2PrxHelper.uncheckedCast(communicator.stringToProxy("F21:" + helper.getTestEndpoint())),
                            out F2Prx f22);
                        test(f21.ice_getIdentity().name == "F21");
                        f21.op();
                        test(f22.ice_getIdentity().name == "F22");

                        if (initial.hasF3())
                        {
                            F3 f31 = initial.opF3(
                                new F3(new F1("F11"),
                                F2PrxHelper.uncheckedCast(communicator.stringToProxy("F21"))),
                                out F3 f32);

                            test(f31.f1.name == "F11");
                            test(f31.f2.ice_getIdentity().name == "F21");

                            test(f32.f1.name == "F12");
                            test(f32.f2.ice_getIdentity().name == "F22");
                        }
                    }
                    output.WriteLine("ok");

                    output.Write("testing sending class cycle... ");
                    output.Flush();
                    {
                        var rec = new Test.Recursive();
                        rec.v = rec;
                        var acceptsCycles = initial.acceptsClassCycles();
                        try
                        {
                            initial.setCycle(rec);
                            test(acceptsCycles);
                        }
                        catch (Ice.UnknownLocalException)
                        {
                            test(!acceptsCycles);
                        }

                    }
                    output.WriteLine("ok");

                    return initial;
                }
            }
        }
    }
}

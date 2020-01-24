//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Collections.Generic;

namespace Ice
{
    namespace objects
    {
        namespace Test
        {
            public partial class IBase
            {
                partial void IceInitialize()
                {
                    id = "My id";
                }
            }

            public partial class IDerived
            {
                partial void IceInitialize()
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

                partial void IceInitialize()
                {
                    called = true;
                }
            }

            public partial struct S1
            {
                partial void IceInitialize()
                {
                    id = 1;
                }
            }

            public class AllTests : global::Test.AllTests
            {
                public static Test.IInitialPrx allTests(global::Test.TestHelper helper)
                {
                    Ice.Communicator communicator = helper.communicator();

                    var output = helper.getWriter();

                    var initial = IInitialPrx.Parse($"initial:{helper.getTestEndpoint(0)}", communicator);

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

                    // More tests possible for b2 and d, but I think this is already
                    // sufficient.
                    test(b2.theA == b2);
                    test(d.theC == null);
                    output.WriteLine("ok");

                    output.Write("getting B1, B2, C, and D all at once... ");
                    output.Flush();
                    var (b1out, b2out, cout, dout) = initial.getAll();
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

                    output.Write("testing AnyClass as parameter... ");
                    output.Flush();
                    {
                        AnyClass v1 = new L("l");
                        var (v3, v2) = initial.opClass(v1);
                        test(((L)v2).data.Equals("l"));
                        test(((L)v3).data.Equals("l"));
                    }
                    {
                        L l = new L("l");
                        AnyClass[] v1 = new AnyClass[] { l };
                        var (v3, v2) = initial.opClassSeq(v1);
                        test(((L)v2[0]).data.Equals("l"));
                        test(((L)v3[0]).data.Equals("l"));
                    }
                    {
                        L l = new L("l");
                        Dictionary<string, AnyClass> v1 = new Dictionary<string, AnyClass> { { "l", l } };
                        var (v3, v2) = initial.opClassMap(v1);
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
                    catch (EDerived ederived)
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
                    catch (Ice.OperationNotExistException)
                    {
                    }
                    output.WriteLine("ok");

                    output.Write("testing sequences...");
                    output.Flush();
                    try
                    {
                        Base[] inS = new Base[0];
                        var (retS, outS) = initial.opBaseSeq(inS);

                        inS = new Base[1];
                        inS[0] = new Base(new S(), "");
                        (retS, outS) = initial.opBaseSeq(inS);
                        test(retS.Length == 1 && outS.Length == 1);
                    }
                    catch (Ice.OperationNotExistException)
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
                        for (; depth <= 1000; ++depth)
                        {
                            p.v = new Test.Recursive();
                            p = p.v;
                            if ((depth < 10 && (depth % 10) == 0) ||
                              (depth < 1000 && (depth % 100) == 0) ||
                              (depth < 10000 && (depth % 1000) == 0) ||
                              (depth % 10000) == 0)
                            {
                                initial.setRecursive(top);
                            }
                        }
                        test(!initial.supportsClassGraphDepthMax());
                    }
                    catch (Ice.UnknownLocalException)
                    {
                        // Expected marshal exception from the server(max class graph depth reached)
                    }
                    catch (Ice.UnknownException)
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
                    var uoet = IUnexpectedObjectExceptionTestPrx.Parse($"uoet:{helper.getTestEndpoint(0)}", communicator);
                    try
                    {
                        uoet.op();
                        test(false);
                    }
                    catch (UnexpectedObjectException ex)
                    {
                        test(ex.Type.Equals("::Test::AlsoEmpty"));
                        test(ex.ExpectedType.Equals("::Test::Empty"));
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
                        var (m2, m1) = initial.opM(m);
                        test(m1.v.Count == 2);
                        test(m2.v.Count == 2);

                        test(m1.v[k1].data.Equals("one"));
                        test(m2.v[k1].data.Equals("one"));

                        test(m1.v[k2].data.Equals("two"));
                        test(m2.v[k2].data.Equals("two"));

                    }
                    output.WriteLine("ok");

                    output.Write("testing forward declared types... ");
                    output.Flush();
                    {
                        var (f11, f12) = initial.opF1(new F1("F11"));
                        test(f11.name.Equals("F11"));
                        test(f12.name.Equals("F12"));

                        var (f21, f22) = initial.opF2(IF2Prx.Parse($"F21:{helper.getTestEndpoint()}", communicator));
                        test(f21.Identity.Name.Equals("F21"));
                        f21.op();
                        test(f22.Identity.Name.Equals("F22"));

                        if (initial.hasF3())
                        {
                            var (f31, f32) = initial.opF3(new F3(new F1("F11"), IF2Prx.Parse("F21", communicator)));

                            test(f31.f1.name.Equals("F11"));
                            test(f31.f2.Identity.Name.Equals("F21"));

                            test(f32.f1.name.Equals("F12"));
                            test(f32.f2.Identity.Name.Equals("F22"));
                        }
                    }
                    output.WriteLine("ok");

                    return initial;
                }
            }
        }
    }
}

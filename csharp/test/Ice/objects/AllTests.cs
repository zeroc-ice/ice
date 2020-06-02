//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Collections.Generic;
using Test;

namespace ZeroC.Ice.Test.Objects
{
    public partial class IBase
    {
        partial void Initialize() => id = "My id";
    }

    public partial class IDerived
    {
        partial void Initialize() => name = "My name";
    }

    public partial class I2
    {
        public bool Called
        {
            get;
            set;
        }

        partial void Initialize() => Called = true;
    }

    public partial struct S1
    {
        partial void Initialize() => id = 1;
    }

    public class AllTests
    {
        public static IInitialPrx allTests(TestHelper helper)
        {
            Communicator? communicator = helper.Communicator();
            TestHelper.Assert(communicator != null);
            var output = helper.GetWriter();

            var initial = IInitialPrx.Parse($"initial:{helper.GetTestEndpoint(0)}", communicator);
            TestHelper.Assert(initial != null);
            output.Write("getting B1... ");
            output.Flush();
            B? b1 = initial.getB1();
            TestHelper.Assert(b1 != null);
            output.WriteLine("ok");

            output.Write("getting B2... ");
            output.Flush();
            B? b2 = initial.getB2();
            TestHelper.Assert(b2 != null);
            output.WriteLine("ok");

            output.Write("getting C... ");
            output.Flush();
            C? c = initial.getC();
            TestHelper.Assert(c != null);
            output.WriteLine("ok");

            output.Write("getting D... ");
            output.Flush();
            D? d = initial.getD();
            TestHelper.Assert(d != null);
            output.WriteLine("ok");

            output.Write("checking consistency... ");
            output.Flush();
            TestHelper.Assert(b1 != b2);
            //TestHelper.Assert(b1 != c);
            //TestHelper.Assert(b1 != d);
            //TestHelper.Assert(b2 != c);
            //TestHelper.Assert(b2 != d);
            //TestHelper.Assert(c != d);
            TestHelper.Assert(b1.theB == b1);
            TestHelper.Assert(b1.theC == null);
            TestHelper.Assert(b1.theA is B);
            TestHelper.Assert(((B)b1.theA).theA == b1.theA);
            TestHelper.Assert(((B)b1.theA).theB == b1);
            //TestHelper.Assert(((B)b1.theA).theC is C); // Redundant -- theC is always of type C
            TestHelper.Assert(((B)b1.theA).theC!.theB == b1.theA);

            // More tests possible for b2 and d, but I think this is already
            // sufficient.
            TestHelper.Assert(b2.theA == b2);
            TestHelper.Assert(d.theC == null);
            output.WriteLine("ok");

            output.Write("getting B1, B2, C, and D all at once... ");
            output.Flush();
            var (b1out, b2out, cout, dout) = initial.getAll();
            TestHelper.Assert(b1out != null);
            TestHelper.Assert(b2out != null);
            TestHelper.Assert(cout != null);
            TestHelper.Assert(dout != null);
            output.WriteLine("ok");

            output.Write("checking consistency... ");
            output.Flush();
            TestHelper.Assert(b1out != b2out);
            TestHelper.Assert(b1out.theA == b2out);
            TestHelper.Assert(b1out.theB == b1out);
            TestHelper.Assert(b1out.theC == null);
            TestHelper.Assert(b2out.theA == b2out);
            TestHelper.Assert(b2out.theB == b1out);
            TestHelper.Assert(b2out.theC == cout);
            TestHelper.Assert(cout.theB == b2out);
            TestHelper.Assert(dout.theA == b1out);
            TestHelper.Assert(dout.theB == b2out);
            TestHelper.Assert(dout.theC == null);

            output.WriteLine("ok");

            output.Write("getting K... ");
            {
                output.Flush();
                K? k = initial.getK();
                var l = k!.value as L;
                TestHelper.Assert(l != null);
                TestHelper.Assert(l.data.Equals("l"));
            }
            output.WriteLine("ok");

            output.Write("testing AnyClass as parameter... ");
            output.Flush();
            {
                AnyClass v1 = new L("l");
                (AnyClass? v3, AnyClass? v2) = initial.opClass(v1);
                TestHelper.Assert(((L)v2!).data.Equals("l"));
                TestHelper.Assert(((L)v3!).data.Equals("l"));
            }
            {
                L l = new L("l");
                AnyClass[] v1 = new AnyClass[] { l };
                (AnyClass?[] v3, AnyClass?[] v2) = initial.opClassSeq(v1);
                TestHelper.Assert(((L)v2[0]!).data.Equals("l"));
                TestHelper.Assert(((L)v3[0]!).data.Equals("l"));
            }
            {
                var l = new L("l");
                var v1 = new Dictionary<string, AnyClass?> { { "l", l } };
                (Dictionary<string, AnyClass?> v3, Dictionary<string, AnyClass?> v2) = initial.opClassMap(v1);
                TestHelper.Assert(((L)v2["l"]!).data.Equals("l"));
                TestHelper.Assert(((L)v3["l"]!).data.Equals("l"));
            }
            output.WriteLine("ok");

            output.Write("getting D1... ");
            output.Flush();
            D1? d1 = initial.getD1(new D1(new A1("a1"), new A1("a2"), new A1("a3"), new A1("a4")));
            TestHelper.Assert(d1!.a1!.name.Equals("a1"));
            TestHelper.Assert(d1!.a2!.name.Equals("a2"));
            TestHelper.Assert(d1!.a3!.name.Equals("a3"));
            TestHelper.Assert(d1!.a4!.name.Equals("a4"));
            output.WriteLine("ok");

            output.Write("throw EDerived... ");
            output.Flush();
            try
            {
                initial.throwEDerived();
                TestHelper.Assert(false);
            }
            catch (EDerived ederived)
            {
                TestHelper.Assert(ederived.a1!.name.Equals("a1"));
                TestHelper.Assert(ederived.a2!.name.Equals("a2"));
                TestHelper.Assert(ederived.a3!.name.Equals("a3"));
                TestHelper.Assert(ederived.a4!.name.Equals("a4"));
            }
            output.WriteLine("ok");

            output.Write("setting G... ");
            output.Flush();
            try
            {
                initial.setG(new G(new S("hello"), "g"));
            }
            catch (OperationNotExistException)
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
                inS[0] = new Base(new S(""), "");
                (retS, outS) = initial.opBaseSeq(inS);
                TestHelper.Assert(retS.Length == 1 && outS.Length == 1);
            }
            catch (OperationNotExistException)
            {
            }
            output.WriteLine("ok");

            output.Write("testing recursive type... ");
            output.Flush();
            var top = new Recursive();
            var p = top;
            int depth = 0;
            try
            {
                for (; depth <= 1000; ++depth)
                {
                    p.v = new Recursive();
                    p = p.v;
                    if ((depth < 10 && (depth % 10) == 0) ||
                        (depth < 1000 && (depth % 100) == 0) ||
                        (depth < 10000 && (depth % 1000) == 0) ||
                        (depth % 10000) == 0)
                    {
                        initial.setRecursive(top);
                    }
                }
                TestHelper.Assert(!initial.supportsClassGraphDepthMax());
            }
            catch (UnhandledException)
            {
                // Expected marshal exception from the server (max class graph depth reached)
                // Expected stack overflow from the server(Java only)
            }
            initial.setRecursive(new Recursive());
            output.WriteLine("ok");

            output.Write("testing compact ID...");
            output.Flush();
            try
            {
                TestHelper.Assert(initial.getCompact() != null);
            }
            catch (OperationNotExistException)
            {
            }
            output.WriteLine("ok");

            output.Write("testing marshaled results...");
            output.Flush();
            b1 = initial.getMB();
            TestHelper.Assert(b1 != null && b1.theB == b1);
            b1 = initial.getAMDMBAsync().Result;
            TestHelper.Assert(b1 != null && b1.theB == b1);
            output.WriteLine("ok");

            output.Write("testing UnexpectedObjectException...");
            output.Flush();
            var uoet = IUnexpectedObjectExceptionTestPrx.Parse($"uoet:{helper.GetTestEndpoint(0)}", communicator);
            try
            {
                uoet.op();
                TestHelper.Assert(false);
            }
            catch (InvalidDataException ex)
            {
                TestHelper.Assert(ex.Message.Contains("ZeroC.Ice.Test.Objects.AlsoEmpty"));
                TestHelper.Assert(ex.Message.Contains("ZeroC.Ice.Test.Objects.Empty"));
            }
            catch (System.Exception ex)
            {
                output.WriteLine(ex.ToString());
                TestHelper.Assert(false);
            }
            output.WriteLine("ok");

            output.Write("testing partial Initialize...");
            output.Flush();
            var ib1 = new IBase();
            TestHelper.Assert(ib1.id.Equals("My id"));
            var id1 = new IDerived();
            TestHelper.Assert(id1.id.Equals("My id"));
            TestHelper.Assert(id1.name.Equals("My name"));

            var id2 = new IDerived2();
            TestHelper.Assert(id2.id.Equals("My id"));
            var i2 = new I2();
            TestHelper.Assert(i2.Called);

            var s1 = new S1();
            // The struct default constructor do not call ice_initialize
            TestHelper.Assert(s1.id == 0);
            s1 = new S1(2);
            // The id should have the value set by ice_initialize and not 2
            TestHelper.Assert(s1.id == 1);

            output.WriteLine("ok");

            output.Write("testing class containing complex dictionary... ");
            output.Flush();
            {
                var m = new M(new Dictionary<StructKey, L?>());
                var k1 = new StructKey(1, "1");
                m.v[k1] = new L("one");
                var k2 = new StructKey(2, "2");
                m.v[k2] = new L("two");
                (M? m2, M? m1) = initial.opM(m);
                TestHelper.Assert(m1 != null && m2 != null);
                TestHelper.Assert(m1.v.Count == 2);
                TestHelper.Assert(m2.v.Count == 2);

                TestHelper.Assert(m1.v[k1]!.data.Equals("one"));
                TestHelper.Assert(m2.v[k1]!.data.Equals("one"));

                TestHelper.Assert(m1.v[k2]!.data.Equals("two"));
                TestHelper.Assert(m2.v[k2]!.data.Equals("two"));

            }
            output.WriteLine("ok");

            output.Write("testing forward declared types... ");
            output.Flush();
            {
                (F1? f11, F1? f12) = initial.opF1(new F1("F11"));
                TestHelper.Assert(f11!.name.Equals("F11"));
                TestHelper.Assert(f12!.name.Equals("F12"));

                (IF2Prx? f21, IF2Prx? f22) =
                    initial.opF2(IF2Prx.Parse($"F21:{helper.GetTestEndpoint()}", communicator));
                TestHelper.Assert(f21!.Identity.Name.Equals("F21"));
                f21.op();
                TestHelper.Assert(f22!.Identity.Name.Equals("F22"));

                if (initial.hasF3())
                {
                    (F3? f31, F3? f32) = initial.opF3(new F3(new F1("F11"), IF2Prx.Parse("F21", communicator)));

                    TestHelper.Assert(f31!.f1!.name.Equals("F11"));
                    TestHelper.Assert(f31!.f2!.Identity.Name.Equals("F21"));

                    TestHelper.Assert(f32!.f1!.name.Equals("F12"));
                    TestHelper.Assert(f32!.f2!.Identity.Name.Equals("F22"));
                }
            }
            output.WriteLine("ok");

            return initial;
        }
    }
}

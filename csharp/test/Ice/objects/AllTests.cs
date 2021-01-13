// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Generic;
using System.IO;
using System.Threading.Tasks;
using ZeroC.Test;

namespace ZeroC.Ice.Test.Objects
{
    public partial class IBase
    {
        partial void Initialize() => Id = "My id";
    }

    public partial class IDerived
    {
        partial void Initialize() => Name = "My name";
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
        partial void Initialize() => Id = 1;
    }

    public static class AllTests
    {
        public static async Task RunAsync(TestHelper helper)
        {
            Communicator communicator = helper.Communicator;

            TextWriter output = helper.Output;

            var initial = IInitialPrx.Parse(helper.GetTestProxy("initial", 0), communicator);
            TestHelper.Assert(initial != null);
            output.Write("getting B1... ");
            output.Flush();
            B? b1 = initial.GetB1();
            TestHelper.Assert(b1 != null);
            output.WriteLine("ok");

            output.Write("getting B2... ");
            output.Flush();
            B? b2 = initial.GetB2();
            TestHelper.Assert(b2 != null);
            output.WriteLine("ok");

            output.Write("getting C... ");
            output.Flush();
            C? c = initial.GetC();
            TestHelper.Assert(c != null);
            output.WriteLine("ok");

            output.Write("getting D... ");
            output.Flush();
            D? d = initial.GetD();
            TestHelper.Assert(d != null);
            output.WriteLine("ok");

            output.Write("checking consistency... ");
            output.Flush();
            TestHelper.Assert(b1 != b2);
            TestHelper.Assert(b1.TheB == b1);
            TestHelper.Assert(b1.TheC == null);
            TestHelper.Assert(b1.TheA is B);
            TestHelper.Assert(((B)b1.TheA).TheA == b1.TheA);
            TestHelper.Assert(((B)b1.TheA).TheB == b1);
            TestHelper.Assert(((B)b1.TheA).TheC!.TheB == b1.TheA);

            // More tests possible for b2 and d, but I think this is already
            // sufficient.
            TestHelper.Assert(b2.TheA == b2);
            TestHelper.Assert(d.TheC == null);
            output.WriteLine("ok");

            output.Write("getting B1, B2, C, and D all at once... ");
            output.Flush();
            (B? b1out, B? b2out, C? cout, D? dout) = initial.GetAll();
            TestHelper.Assert(b1out != null);
            TestHelper.Assert(b2out != null);
            TestHelper.Assert(cout != null);
            TestHelper.Assert(dout != null);
            output.WriteLine("ok");

            output.Write("checking consistency... ");
            output.Flush();
            TestHelper.Assert(b1out != b2out);
            TestHelper.Assert(b1out.TheA == b2out);
            TestHelper.Assert(b1out.TheB == b1out);
            TestHelper.Assert(b1out.TheC == null);
            TestHelper.Assert(b2out.TheA == b2out);
            TestHelper.Assert(b2out.TheB == b1out);
            TestHelper.Assert(b2out.TheC == cout);
            TestHelper.Assert(cout.TheB == b2out);
            TestHelper.Assert(dout.TheA == b1out);
            TestHelper.Assert(dout.TheB == b2out);
            TestHelper.Assert(dout.TheC == null);

            output.WriteLine("ok");

            output.Write("getting K... ");
            {
                output.Flush();
                K? k = initial.GetK();
                var l = k!.Value as L;
                TestHelper.Assert(l != null);
                TestHelper.Assert(l.Data.Equals("l"));
            }
            output.WriteLine("ok");

            output.Write("testing AnyClass as parameter... ");
            output.Flush();
            {
                AnyClass v1 = new L("l");
                (AnyClass? v3, AnyClass? v2) = initial.OpClass(v1);
                TestHelper.Assert(((L)v2!).Data.Equals("l"));
                TestHelper.Assert(((L)v3!).Data.Equals("l"));
            }
            {
                var l = new L("l");
                var v1 = new AnyClass[] { l };
                (AnyClass?[] v3, AnyClass?[] v2) = initial.OpClassSeq(v1);
                TestHelper.Assert(((L)v2[0]!).Data.Equals("l"));
                TestHelper.Assert(((L)v3[0]!).Data.Equals("l"));
            }
            {
                var l = new L("l");
                var v1 = new Dictionary<string, AnyClass?> { { "l", l } };
                (Dictionary<string, AnyClass?> v3, Dictionary<string, AnyClass?> v2) = initial.OpClassMap(v1);
                TestHelper.Assert(((L)v2["l"]!).Data.Equals("l"));
                TestHelper.Assert(((L)v3["l"]!).Data.Equals("l"));
            }
            output.WriteLine("ok");

            output.Write("getting D1... ");
            output.Flush();
            D1? d1 = initial.GetD1(new D1(new A1("a1"), new A1("a2"), new A1("a3"), new A1("a4")));
            TestHelper.Assert(d1!.A1!.Name.Equals("a1"));
            TestHelper.Assert(d1!.A2!.Name.Equals("a2"));
            TestHelper.Assert(d1!.A3!.Name.Equals("a3"));
            TestHelper.Assert(d1!.A4!.Name.Equals("a4"));
            output.WriteLine("ok");

            output.Write("throw EDerived... ");
            output.Flush();
            try
            {
                initial.ThrowEDerived();
                TestHelper.Assert(false);
            }
            catch (EDerived ederived)
            {
                TestHelper.Assert(ederived.A1!.Name.Equals("a1"));
                TestHelper.Assert(ederived.A2!.Name.Equals("a2"));
                TestHelper.Assert(ederived.A3!.Name.Equals("a3"));
                TestHelper.Assert(ederived.A4!.Name.Equals("a4"));
            }
            output.WriteLine("ok");

            output.Write("setting G... ");
            output.Flush();
            try
            {
                initial.SetG(new G(new S("hello"), "g"));
            }
            catch (OperationNotExistException)
            {
            }
            output.WriteLine("ok");

            output.Write("testing sequences...");
            output.Flush();
            try
            {
                Base[] inS = Array.Empty<Base>();
                (Base?[] retS, Base?[] outS) = initial.OpBaseSeq(inS);

                inS = new Base[1];
                inS[0] = new Base(new S(""), "");
                (retS, outS) = initial.OpBaseSeq(inS);
                TestHelper.Assert(retS.Length == 1 && outS.Length == 1);
            }
            catch (OperationNotExistException)
            {
            }
            output.WriteLine("ok");

            output.Write("testing recursive type... ");
            output.Flush();
            var top = new Recursive();
            Recursive p = top;
            int depth = 0;
            try
            {
                for (; depth <= 1000; ++depth)
                {
                    p.V = new Recursive();
                    p = p.V;
                    if ((depth < 10 && (depth % 10) == 0) ||
                        (depth < 1000 && (depth % 100) == 0) ||
                        (depth < 10000 && (depth % 1000) == 0) ||
                        (depth % 10000) == 0)
                    {
                        initial.SetRecursive(top);
                    }
                }
                TestHelper.Assert(!initial.SupportsClassGraphMaxDepth());
            }
            catch (UnhandledException)
            {
                // Expected marshal exception from the server (max class graph depth reached)
                // Expected stack overflow from the server(Java only)
            }
            initial.SetRecursive(new Recursive());
            output.WriteLine("ok");

            output.Write("testing compact ID...");
            output.Flush();
            try
            {
                TestHelper.Assert(initial.GetCompact() != null);
            }
            catch (OperationNotExistException)
            {
            }
            output.WriteLine("ok");

            output.Write("testing marshaled results...");
            output.Flush();
            b1 = initial.GetMB();
            TestHelper.Assert(b1 != null && b1.TheB == b1);
            b1 = initial.GetAMDMBAsync().Result;
            TestHelper.Assert(b1 != null && b1.TheB == b1);
            output.WriteLine("ok");

            output.Write("testing UnexpectedObjectException...");
            output.Flush();
            var uoet = IUnexpectedObjectExceptionTestPrx.Parse(helper.GetTestProxy("uoet", 0), communicator);
            try
            {
                uoet.Op();
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
            TestHelper.Assert(ib1.Id.Equals("My id"));
            var id1 = new IDerived();
            TestHelper.Assert(id1.Id.Equals("My id"));
            TestHelper.Assert(id1.Name.Equals("My name"));

            var id2 = new IDerived2();
            TestHelper.Assert(id2.Id.Equals("My id"));
            var i2 = new I2();
            TestHelper.Assert(i2.Called);

            var s1 = new S1();
            // The struct default constructor do not call ice_initialize
            TestHelper.Assert(s1.Id == 0);
            s1 = new S1(2);
            // The id should have the value set by ice_initialize and not 2
            TestHelper.Assert(s1.Id == 1);

            output.WriteLine("ok");

            output.Write("testing class containing complex dictionary... ");
            output.Flush();
            {
                var m = new M(new Dictionary<StructKey, L?>());
                var k1 = new StructKey(1, "1");
                m.V[k1] = new L("one");
                var k2 = new StructKey(2, "2");
                m.V[k2] = new L("two");
                (M? m2, M? m1) = initial.OpM(m);
                TestHelper.Assert(m1 != null && m2 != null);
                TestHelper.Assert(m1.V.Count == 2);
                TestHelper.Assert(m2.V.Count == 2);

                TestHelper.Assert(m1.V[k1]!.Data.Equals("one"));
                TestHelper.Assert(m2.V[k1]!.Data.Equals("one"));

                TestHelper.Assert(m1.V[k2]!.Data.Equals("two"));
                TestHelper.Assert(m2.V[k2]!.Data.Equals("two"));
            }
            output.WriteLine("ok");

            output.Write("testing forward declared types... ");
            output.Flush();
            {
                (F1? f11, F1? f12) = initial.OpF1(new F1("F11"));
                TestHelper.Assert(f11!.Name.Equals("F11"));
                TestHelper.Assert(f12!.Name.Equals("F12"));

                (IF2Prx? f21, IF2Prx? f22) =
                    initial.OpF2(IF2Prx.Parse(helper.GetTestProxy("F21"), communicator));
                TestHelper.Assert(f21!.Identity.Name.Equals("F21"));
                f21.Op();
                TestHelper.Assert(f22!.Identity.Name.Equals("F22"));

                if (initial.HasF3())
                {
                    (F3? f31, F3? f32) = initial.OpF3(new F3(new F1("F11"), IF2Prx.Parse("F21", communicator)));

                    TestHelper.Assert(f31!.F1!.Name.Equals("F11"));
                    TestHelper.Assert(f31!.F2!.Identity.Name.Equals("F21"));

                    TestHelper.Assert(f32!.F1!.Name.Equals("F12"));
                    TestHelper.Assert(f32!.F2!.Identity.Name.Equals("F22"));
                }
            }
            output.WriteLine("ok");

            await initial.ShutdownAsync();
        }
    }
}

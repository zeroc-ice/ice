// Copyright (c) ZeroC, Inc. All rights reserved.

using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Threading.Tasks;
using ZeroC.Test;

namespace ZeroC.Ice.Test.Tagged
{
    public static class AllTests
    {
        public static async Task RunAsync(TestHelper helper)
        {
            Communicator communicator = helper.Communicator;

            TextWriter output = helper.Output;
            var initial = IInitialPrx.Parse(helper.GetTestProxy("initial", 0), communicator);

            output.Write("testing tagged data members... ");
            output.Flush();

            var oo1 = new OneTagged();
            TestHelper.Assert(!oo1.A.HasValue);
            oo1.A = 15;
            TestHelper.Assert(oo1.A.HasValue && oo1.A == 15);

            var oo2 = new OneTagged(16);
            TestHelper.Assert(oo2.A.HasValue && oo2.A == 16);

            var mo1 = new MultiTagged();
            mo1.A = 15;
            mo1.B = true;
            mo1.C = 19;
            mo1.D = 78;
            mo1.E = 99;
            mo1.F = 5.5F;
            mo1.G = 1.0;
            mo1.H = "test";
            mo1.I = MyEnum.M1;
            mo1.Bs = new byte[] { 5 };
            mo1.Ss = new string[] { "test", "test2" };
            mo1.Iid = new Dictionary<int, int>
            {
                { 4, 3 }
            };
            mo1.Sid = new Dictionary<string, int>
            {
                { "test", 10 }
            };
            var fs = new FixedStruct();
            fs.M = 78;
            mo1.Fs = fs;
            var vs1 = new VarStruct();
            vs1.M = "hello";
            mo1.Vs = vs1;

            mo1.Shs = new short[] { 1 };
            mo1.Es = new MyEnum[] { MyEnum.M1, MyEnum.M1 };
            mo1.Fss = new FixedStruct[] { fs };
            mo1.Vss = new VarStruct[] { vs1 };

            mo1.Ied = new Dictionary<int, MyEnum>
            {
                { 4, MyEnum.M1 }
            };
            mo1.Ifsd = new Dictionary<int, FixedStruct>
            {
                { 4, fs }
            };
            mo1.Ivsd = new Dictionary<int, VarStruct>
            {
                { 5, vs1 }
            };

            mo1.Bos = new bool[] { false, true, false };

            mo1.Us = 321;
            mo1.Ui = 123;
            mo1.Ul = 10_000_000_000_000;
            mo1.Vi = 3_000_000;
            mo1.Vl = -10_000_000_000_000;
            mo1.Vui = 123;
            mo1.Vul = 20_000_000_000_000;

            mo1.Uss = new ushort[] { 6, 1, 327 };
            mo1.Vuls = new ulong[] { 100, 500, 1 };
            mo1.Vil = new List<int> { 2, -300, 0 };

            TestHelper.Assert(mo1.A == 15);
            TestHelper.Assert(mo1.B == true);
            TestHelper.Assert(mo1.C == 19);
            TestHelper.Assert(mo1.D == 78);
            TestHelper.Assert(mo1.E == 99);
            TestHelper.Assert(mo1.F == 5.5F);
            TestHelper.Assert(mo1.G == 1.0);
            TestHelper.Assert(mo1.H.Equals("test"));
            TestHelper.Assert(mo1.I == MyEnum.M1);
            TestHelper.Assert(Enumerable.SequenceEqual(mo1.Bs, new byte[] { 5 }));
            TestHelper.Assert(Enumerable.SequenceEqual(mo1.Ss, new string[] { "test", "test2" }));
            TestHelper.Assert(mo1.Iid[4] == 3);
            TestHelper.Assert(mo1.Sid["test"] == 10);
            TestHelper.Assert(mo1.Fs.Equals(new FixedStruct(78)));
            TestHelper.Assert(mo1.Vs.Equals(new VarStruct("hello")));

            TestHelper.Assert(mo1.Shs[0] == 1);
            TestHelper.Assert(mo1.Es[0] == MyEnum.M1 && mo1.Es[1] == MyEnum.M1);
            TestHelper.Assert(mo1.Fss[0].Equals(new FixedStruct(78)));
            TestHelper.Assert(mo1.Vss[0].Equals(new VarStruct("hello")));

            TestHelper.Assert(mo1.Ied[4] == MyEnum.M1);
            TestHelper.Assert(mo1.Ifsd[4].Equals(new FixedStruct(78)));
            TestHelper.Assert(mo1.Ivsd[5].Equals(new VarStruct("hello")));

            TestHelper.Assert(Enumerable.SequenceEqual(mo1.Bos, new bool[] { false, true, false }));

            output.WriteLine("ok");

            output.Write("testing marshaling... ");
            output.Flush();

            var oo4 = (OneTagged?)initial.PingPong(new OneTagged());
            TestHelper.Assert(oo4 != null && !oo4.A.HasValue);

            var oo5 = (OneTagged?)initial.PingPong(oo1);
            TestHelper.Assert(oo5 != null && oo1.A == oo5.A);

            var mo4 = (MultiTagged?)initial.PingPong(new MultiTagged());
            TestHelper.Assert(mo4 != null);
            TestHelper.Assert(mo4.A == null);
            TestHelper.Assert(mo4.B == null);
            TestHelper.Assert(mo4.C == null);
            TestHelper.Assert(mo4.D == null);
            TestHelper.Assert(mo4.E == null);
            TestHelper.Assert(mo4.F == null);
            TestHelper.Assert(mo4.G == null);
            TestHelper.Assert(mo4.H == null);
            TestHelper.Assert(mo4.I == null);
            TestHelper.Assert(mo4.Bs == null);
            TestHelper.Assert(mo4.Ss == null);
            TestHelper.Assert(mo4.Iid == null);
            TestHelper.Assert(mo4.Sid == null);
            TestHelper.Assert(mo4.Fs == null);
            TestHelper.Assert(mo4.Vs == null);

            TestHelper.Assert(mo4.Shs == null);
            TestHelper.Assert(mo4.Es == null);
            TestHelper.Assert(mo4.Fss == null);
            TestHelper.Assert(mo4.Vss == null);

            TestHelper.Assert(mo4.Ied == null);
            TestHelper.Assert(mo4.Ifsd == null);
            TestHelper.Assert(mo4.Ivsd == null);

            TestHelper.Assert(mo4.Bos == null);

            TestHelper.Assert(mo4.Us == null);
            TestHelper.Assert(mo4.Ui == null);
            TestHelper.Assert(mo4.Ul == null);
            TestHelper.Assert(mo4.Vi == null);
            TestHelper.Assert(mo4.Vl == null);
            TestHelper.Assert(mo4.Vui == null);
            TestHelper.Assert(mo4.Vul == null);

            TestHelper.Assert(mo4.Uss == null);
            TestHelper.Assert(mo4.Vuls == null);
            TestHelper.Assert(mo4.Vil == null);

            var mo5 = (MultiTagged?)initial.PingPong(mo1);
            TestHelper.Assert(mo5 != null);
            TestHelper.Assert(mo5.A == mo1.A);
            TestHelper.Assert(mo5.B == mo1.B);
            TestHelper.Assert(mo5.C == mo1.C);
            TestHelper.Assert(mo5.D == mo1.D);
            TestHelper.Assert(mo5.E == mo1.E);
            TestHelper.Assert(mo5.F == mo1.F);
            TestHelper.Assert(mo5.G == mo1.G);
            TestHelper.Assert(mo5.H!.Equals(mo1.H));
            TestHelper.Assert(mo5.I == mo1.I);
            TestHelper.Assert(Enumerable.SequenceEqual(mo5.Bs!, mo1.Bs));
            TestHelper.Assert(Enumerable.SequenceEqual(mo5.Ss!, mo1.Ss));
            TestHelper.Assert(mo5.Iid != null && mo5.Iid[4] == 3);
            TestHelper.Assert(mo5.Sid != null && mo5.Sid["test"] == 10);
            TestHelper.Assert(mo5.Fs.Equals(mo1.Fs));
            TestHelper.Assert(mo5.Vs.Equals(mo1.Vs));
            TestHelper.Assert(Enumerable.SequenceEqual(mo5.Shs!, mo1.Shs));
            TestHelper.Assert(mo5.Es != null && mo5.Es[0] == MyEnum.M1 && mo1.Es[1] == MyEnum.M1);
            TestHelper.Assert(mo5.Fss != null && mo5.Fss[0].Equals(new FixedStruct(78)));
            TestHelper.Assert(mo5.Vss != null && mo5.Vss[0].Equals(new VarStruct("hello")));

            TestHelper.Assert(mo5.Ied != null && mo5.Ied[4] == MyEnum.M1);
            TestHelper.Assert(mo5.Ifsd != null && mo5.Ifsd[4].Equals(new FixedStruct(78)));
            TestHelper.Assert(mo5.Ivsd != null && mo5.Ivsd[5].Equals(new VarStruct("hello")));

            TestHelper.Assert(mo5.Us == mo1.Us);
            TestHelper.Assert(mo5.Ui == mo1.Ui);
            TestHelper.Assert(mo5.Ul == mo1.Ul);
            TestHelper.Assert(mo5.Vi == mo1.Vi);
            TestHelper.Assert(mo5.Vl == mo1.Vl);
            TestHelper.Assert(mo5.Vui == mo1.Vui);
            TestHelper.Assert(mo5.Vul == mo1.Vul);

            TestHelper.Assert(mo5.Uss!.SequenceEqual(mo1.Uss));
            TestHelper.Assert(mo5.Vuls!.SequenceEqual(mo1.Vuls));
            TestHelper.Assert(mo5.Vil!.SequenceEqual(mo1.Vil));

            // Clear the first half of the tagged members
            var mo6 = new MultiTagged();
            mo6.B = mo5.B;
            mo6.D = mo5.D;
            mo6.F = mo5.F;
            mo6.H = mo5.H;
            mo6.Bs = mo5.Bs;
            mo6.Iid = mo5.Iid;
            mo6.Fs = mo5.Fs;
            mo6.Shs = mo5.Shs;
            mo6.Fss = mo5.Fss;
            mo6.Ifsd = mo5.Ifsd;
            mo6.Bos = mo5.Bos;

            var mo7 = (MultiTagged?)initial.PingPong(mo6);
            TestHelper.Assert(mo7 != null);
            TestHelper.Assert(mo7.A == null);
            TestHelper.Assert(mo7.B.Equals(mo1.B));
            TestHelper.Assert(mo7.C == null);
            TestHelper.Assert(mo7.D.Equals(mo1.D));
            TestHelper.Assert(mo7.E == null);
            TestHelper.Assert(mo7.F.Equals(mo1.F));
            TestHelper.Assert(mo7.G == null);
            TestHelper.Assert(mo7.H != null && mo7.H.Equals(mo1.H));
            TestHelper.Assert(mo7.I == null);
            TestHelper.Assert(Enumerable.SequenceEqual(mo7.Bs!, mo1.Bs));
            TestHelper.Assert(mo7.Ss == null);
            TestHelper.Assert(mo7.Iid != null && mo7.Iid[4] == 3);
            TestHelper.Assert(mo7.Sid == null);
            TestHelper.Assert(mo7.Fs.Equals(mo1.Fs));
            TestHelper.Assert(mo7.Vs == null);

            TestHelper.Assert(Enumerable.SequenceEqual(mo7.Shs!, mo1.Shs));
            TestHelper.Assert(mo7.Es == null);
            TestHelper.Assert(mo7.Fss != null && mo7.Fss[0].Equals(new FixedStruct(78)));
            TestHelper.Assert(mo7.Vss == null);

            TestHelper.Assert(mo7.Ied == null);
            TestHelper.Assert(mo7.Ifsd != null && mo7.Ifsd[4].Equals(new FixedStruct(78)));
            TestHelper.Assert(mo7.Ivsd == null);

            TestHelper.Assert(Enumerable.SequenceEqual(mo7.Bos!, new bool[] { false, true, false }));

            // Clear the second half of the tagged members
            var mo8 = new MultiTagged();
            mo8.A = mo5.A;
            mo8.C = mo5.C;
            mo8.E = mo5.E;
            mo8.G = mo5.G;
            mo8.I = mo5.I;
            mo8.Ss = mo5.Ss;
            mo8.Sid = mo5.Sid;
            mo8.Vs = mo5.Vs;

            mo8.Es = mo5.Es;
            mo8.Vss = mo5.Vss;

            mo8.Ied = mo5.Ied;
            mo8.Ivsd = mo5.Ivsd;

            var mo9 = (MultiTagged?)initial.PingPong(mo8);
            TestHelper.Assert(mo9 != null);
            TestHelper.Assert(mo9.A.Equals(mo1.A));
            TestHelper.Assert(!mo9.B.HasValue);
            TestHelper.Assert(mo9.C.Equals(mo1.C));
            TestHelper.Assert(!mo9.D.HasValue);
            TestHelper.Assert(mo9.E.Equals(mo1.E));
            TestHelper.Assert(!mo9.F.HasValue);
            TestHelper.Assert(mo9.G.Equals(mo1.G));
            TestHelper.Assert(mo9.H == null);
            TestHelper.Assert(mo9.I.Equals(mo1.I));
            TestHelper.Assert(mo9.Bs == null);
            TestHelper.Assert(Enumerable.SequenceEqual(mo9.Ss!, mo1.Ss));
            TestHelper.Assert(mo9.Iid == null);
            TestHelper.Assert(mo9.Sid != null && mo9.Sid["test"] == 10);
            TestHelper.Assert(mo9.Fs == null);
            TestHelper.Assert(mo9.Vs.Equals(mo1.Vs));

            TestHelper.Assert(mo9.Shs == null);
            TestHelper.Assert(mo9.Es != null && mo9.Es[0] == MyEnum.M1 && mo9.Es[1] == MyEnum.M1);
            TestHelper.Assert(mo9.Fss == null);
            TestHelper.Assert(mo9.Vss != null && mo9.Vss[0].Equals(new VarStruct("hello")));

            TestHelper.Assert(mo9.Ied != null && mo9.Ied[4] == MyEnum.M1);
            TestHelper.Assert(mo9.Ifsd == null);
            TestHelper.Assert(mo9.Ivsd != null && mo9.Ivsd[5].Equals(new VarStruct("hello")));

            TestHelper.Assert(mo9.Bos == null);

            {
                var owc1 = new TaggedWithCustom();
                owc1.L = new List<SmallStruct>
                {
                    new SmallStruct(5),
                    new SmallStruct(6),
                    new SmallStruct(7)
                };
                owc1.S = new ClassVarStruct(5);
                var owc2 = (TaggedWithCustom?)initial.PingPong(owc1);
                TestHelper.Assert(owc2 != null);
                TestHelper.Assert(owc2.L != null);
                TestHelper.Assert(Enumerable.SequenceEqual(owc1.L, owc2.L));
                TestHelper.Assert(owc2.S != null && owc2.S.Value.A == 5);
            }

            /* TODO: rewrite test without factories

            //
            // Send a request using blobjects. Upon receival, we don't read
            // any of the tagged members. This ensures the tagged members
            // are skipped even if the receiver knows nothing about them.
            //
            factory.setEnabled(true);
            OutputStream os = new OutputStream(communicator);
            os.StartEncapsulation();
            os.WriteNullableClass(oo1);
            os.EndEncapsulation();
            byte[] inEncaps = os.Finished();
            byte[] outEncaps;
            test(initial.Invoke("pingPong", idempotent: false, inEncaps, out outEncaps));
            InputStream
            responseFrame.InputStream.StartEncapsulation();
            ReadNullableClassCallbackI cb = new ReadNullableClassCallbackI();
            responseFrame.InputStream.ReadNullableClass(cb.invoke);
            responseFrame.InputStream.EndEncapsulation();
            test(cb.obj != null && cb.obj is TestClassReader);

            os = new OutputStream(communicator);
            os.StartEncapsulation();
            os.WriteNullableClass(mo1);
            os.EndEncapsulation();
            inEncaps = os.Finished();
            test(initial.Invoke("pingPong", idempotent: false, inEncaps, out outEncaps));

            responseFrame.InputStream.StartEncapsulation();
            responseFrame.InputStream.ReadNullableClass(cb.invoke);
            responseFrame.InputStream.EndEncapsulation();
            test(cb.obj != null && cb.obj is TestClassReader);
            factory.setEnabled(false);
            */

            initial.OpVoid();

            {
                using var requestFrame = OutgoingRequestFrame.WithArgs(
                    initial,
                    "opVoid",
                    idempotent: false,
                    compress: false,
                    format: default,
                    context: null,
                    (15, "test"),
                    (OutputStream ostr, in (int n, string s) value) =>
                    {
                        ostr.WriteTaggedInt(1, value.n);
                        ostr.WriteTaggedString(1, value.s); // duplicate tag ignored by the server
                    });

                using IncomingResponseFrame response = initial.InvokeAsync(requestFrame).Result;
                TestHelper.Assert(response.ResultType == ResultType.Success);
            }

            output.WriteLine("ok");

            output.Write("testing marshaling of large containers with fixed size elements... ");
            output.Flush();
            var mc = new MultiTagged();

            mc.Bs = new byte[1000];
            mc.Shs = new short[300];

            mc.Fss = new FixedStruct[300];
            for (int i = 0; i < 300; ++i)
            {
                mc.Fss[i] = new FixedStruct();
            }

            mc.Ifsd = new Dictionary<int, FixedStruct>();
            for (int i = 0; i < 300; ++i)
            {
                mc.Ifsd.Add(i, new FixedStruct());
            }

            mc = (MultiTagged?)initial.PingPong(mc);
            TestHelper.Assert(mc != null);
            TestHelper.Assert(mc.Bs!.Length == 1000);
            TestHelper.Assert(mc.Shs!.Length == 300);
            TestHelper.Assert(mc.Fss!.Length == 300);
            TestHelper.Assert(mc.Ifsd!.Count == 300);

            /*
            factory.setEnabled(true);
            os = new OutputStream(communicator);
            os.StartEncapsulation();
            os.WriteNullableClass(mc);
            os.EndEncapsulation();
            inEncaps = os.Finished();
            test(initial.Invoke("pingPong", idempotent: false, inEncaps, out outEncaps));

            responseFrame.InputStream.StartEncapsulation();
            responseFrame.InputStream.ReadNullableClass(cb.invoke);
            responseFrame.InputStream.EndEncapsulation();
            test(cb.obj != null && cb.obj is TestClassReader);
            factory.setEnabled(false);
            */

            output.WriteLine("ok");

            output.Write("testing tag marshaling... ");
            output.Flush();
            {
                var b = new B();
                var b2 = (B?)initial.PingPong(b);
                TestHelper.Assert(b2 != null);
                TestHelper.Assert(!b2.Ma.HasValue);
                TestHelper.Assert(!b2.Mb.HasValue);
                TestHelper.Assert(!b2.Mc.HasValue);

                b.Ma = 10;
                b.Mb = 11;
                b.Mc = 12;
                b.Md = 13;

                b2 = (B?)initial.PingPong(b);
                TestHelper.Assert(b2 != null);
                TestHelper.Assert(b2.Ma == 10);
                TestHelper.Assert(b2.Mb == 11);
                TestHelper.Assert(b2.Mc == 12);
                TestHelper.Assert(b2.Md == 13);

                /*
                factory.setEnabled(true);
                os = new OutputStream(communicator);
                os.StartEncapsulation();
                os.WriteNullableClass(b);
                os.EndEncapsulation();
                inEncaps = os.Finished();
                test(initial.Invoke("pingPong", idempotent: false, inEncaps, out outEncaps));

                responseFrame.InputStream.StartEncapsulation();
                responseFrame.InputStream.ReadNullableClass(cb.invoke);
                responseFrame.InputStream.EndEncapsulation();
                test(cb.obj != null);
                factory.setEnabled(false);
                */
            }
            output.WriteLine("ok");

            output.Write("testing tagged with default values... ");
            output.Flush();
            {
                var wd = (WD?)initial.PingPong(new WD());
                TestHelper.Assert(wd != null);
                TestHelper.Assert(wd.A == 5);
                TestHelper.Assert(wd.S! == "test");
                wd.A = null;
                wd.S = null;
                wd = (WD?)initial.PingPong(wd);
                TestHelper.Assert(wd != null);
                // When a tagged member is set to null (equivalent to not set) explicitly, it remains null / not set,
                // even when it has a default value. This is consistent with the behavior for non-tagged optional
                // data members with default values.
                TestHelper.Assert(wd.A == null);
                TestHelper.Assert(wd.S == null);
            }
            output.WriteLine("ok");

            output.Write("testing tagged parameters... ");
            output.Flush();
            {
                byte? p1 = null;
                (byte? r1, byte? r2) = initial.OpByte(p1);
                TestHelper.Assert(r1 == null && r2 == null);
                (r1, r2) = initial.OpByte(null);
                TestHelper.Assert(r1 == null && r2 == null);

                p1 = 56;
                (r1, r2) = initial.OpByte(p1);
                TestHelper.Assert(r1 == 56 && r2 == 56);
                (byte? ReturnValue, byte? r2) r = initial.OpByteAsync(p1).Result;
                TestHelper.Assert(r.ReturnValue == 56 && r.r2 == 56);
                (r1, r2) = initial.OpByte(p1);
                TestHelper.Assert(r1 == 56 && r2 == 56);
                r = initial.OpByteAsync(p1).Result;
                TestHelper.Assert(r.ReturnValue == 56 && r.r2 == 56);

                (r1, r2) = initial.OpByte(null);
                TestHelper.Assert(r1 == null && r2 == null); // Ensure out parameter is cleared.

                using var requestFrame = OutgoingRequestFrame.WithArgs(
                    initial,
                    "opByte",
                    idempotent: false,
                    compress: false,
                    format: default,
                    context: null,
                    p1,
                    (OutputStream ostr, byte? p1) => ostr.WriteTaggedByte(1, p1));

                using IncomingResponseFrame responseFrame = initial.InvokeAsync(requestFrame).Result;
                (r1, r2) = responseFrame.ReadReturnValue(initial, istr =>
                    {
                        byte? b1 = istr.ReadTaggedByte(1);
                        byte? b2 = istr.ReadTaggedByte(2);
                        return (b1, b2);
                    });
                TestHelper.Assert(r1 == 56);
                TestHelper.Assert(r2 == 56);
            }

            {
                bool? p1 = null;
                (bool? r1, bool? r2) = initial.OpBool(p1);
                TestHelper.Assert(r1 == null && r2 == null);
                (r1, r2) = initial.OpBool(null);
                TestHelper.Assert(r1 == null && r2 == null);

                p1 = true;
                (r1, r2) = initial.OpBool(p1);
                TestHelper.Assert(r1 == true && r2 == true);
                (bool? ReturnValue, bool? r2) r = initial.OpBoolAsync(p1).Result;
                TestHelper.Assert(r.ReturnValue == true && r.r2 == true);
                (r1, r2) = initial.OpBool(true);
                TestHelper.Assert(r1 == true && r2 == true);
                r = initial.OpBoolAsync(true).Result;
                TestHelper.Assert(r.ReturnValue == true && r.r2 == true);

                (r1, r2) = initial.OpBool(null);
                TestHelper.Assert(r1 == null && r2 == null); // Ensure out parameter is cleared.

                using var requestFrame = OutgoingRequestFrame.WithArgs(
                    initial,
                    "opBool",
                    idempotent: false,
                    compress: false,
                    format: default,
                    context: null,
                    p1,
                    (OutputStream ostr, bool? p1) => ostr.WriteTaggedBool(1, p1));

                using IncomingResponseFrame responseFrame = initial.InvokeAsync(requestFrame).Result;
                (r1, r2) = responseFrame.ReadReturnValue(initial, istr =>
                {
                    bool? b1 = istr.ReadTaggedBool(1);
                    bool? b2 = istr.ReadTaggedBool(2);
                    return (b1, b2);
                });
                TestHelper.Assert(r1 == true);
                TestHelper.Assert(r2 == true);
            }

            {
                short? p1 = null;
                (short? r1, short? r2) = initial.OpShort(p1);
                TestHelper.Assert(r1 == null && r2 == null);
                (r1, r2) = initial.OpShort(null);
                TestHelper.Assert(r1 == null && r2 == null);

                p1 = 56;
                (r1, r2) = initial.OpShort(p1);
                TestHelper.Assert(r1 == 56 && r2 == 56);
                (short? ReturnValue, short? r2) r = initial.OpShortAsync(p1).Result;
                TestHelper.Assert(r.ReturnValue == 56 && r.r2 == 56);
                (r1, r2) = initial.OpShort(p1);
                TestHelper.Assert(r1 == 56 && r2 == 56);
                r = initial.OpShortAsync(p1).Result;
                TestHelper.Assert(r.ReturnValue == 56 && r.r2 == 56);

                (r1, r2) = initial.OpShort(null);
                TestHelper.Assert(r1 == null && r2 == null); // Ensure out parameter is cleared.

                using var requestFrame = OutgoingRequestFrame.WithArgs(
                    initial,
                    "opShort",
                    idempotent: false,
                    compress: false,
                    format: default,
                    context: null,
                    p1,
                    (OutputStream ostr, short? p1) => ostr.WriteTaggedShort(1, p1));

                using IncomingResponseFrame responseFrame = initial.InvokeAsync(requestFrame).Result;
                (r1, r2) = responseFrame.ReadReturnValue(initial, istr =>
                    {
                        short? s1 = istr.ReadTaggedShort(1);
                        short? s2 = istr.ReadTaggedShort(2);
                        return (s1, s2);
                    });
                TestHelper.Assert(r1 == 56);
                TestHelper.Assert(r2 == 56);
            }

            {
                int? p1 = null;
                (int? r1, int? r2) = initial.OpInt(p1);
                TestHelper.Assert(r1 == null && r2 == null);
                (r1, r2) = initial.OpInt(null);
                TestHelper.Assert(r1 == null && r2 == null);

                p1 = 56;
                (r1, r2) = initial.OpInt(p1);
                TestHelper.Assert(r1 == 56 && r2 == 56);
                (int? ReturnValue, int? r2) r = initial.OpIntAsync(p1).Result;
                TestHelper.Assert(r.ReturnValue == 56 && r.r2 == 56);
                (r1, r2) = initial.OpInt(p1);
                TestHelper.Assert(r1 == 56 && r2 == 56);
                r = initial.OpIntAsync(p1).Result;
                TestHelper.Assert(r.ReturnValue == 56 && r.r2 == 56);

                (r1, r2) = initial.OpInt(null);
                TestHelper.Assert(r1 == null && r2 == null); // Ensure out parameter is cleared.

                using var requestFrame = OutgoingRequestFrame.WithArgs(
                    initial,
                    "opInt",
                    idempotent: false,
                    compress: false,
                    format: default,
                    context: null,
                    p1,
                    (OutputStream ostr, int? p1) => ostr.WriteTaggedInt(1, p1));

                using IncomingResponseFrame responseFrame = initial.InvokeAsync(requestFrame).Result;
                (r1, r2) = responseFrame.ReadReturnValue(initial, istr =>
                {
                    int? r1 = istr.ReadTaggedInt(1);
                    int? r2 = istr.ReadTaggedInt(2);
                    return (r1, r2);
                });
                TestHelper.Assert(r1 == 56);
                TestHelper.Assert(r2 == 56);
            }

            {
                long? p1 = null;
                (long? r1, long? r2) = initial.OpLong(p1);
                TestHelper.Assert(r1 == null && r2 == null);
                (r1, r2) = initial.OpLong(null);
                TestHelper.Assert(r1 == null && r2 == null);

                p1 = 56;
                (r1, r2) = initial.OpLong(p1);
                TestHelper.Assert(r1 == 56 && r2 == 56);
                (long? ReturnValue, long? r2) r = initial.OpLongAsync(p1).Result;
                TestHelper.Assert(r.ReturnValue == 56 && r.r2 == 56);
                (r1, r2) = initial.OpLong(p1);
                TestHelper.Assert(r1 == 56 && r2 == 56);
                r = initial.OpLongAsync(p1).Result;
                TestHelper.Assert(r.ReturnValue == 56 && r.r2 == 56);

                (r1, r2) = initial.OpLong(null);
                TestHelper.Assert(r1 == null && r2 == null); // Ensure out parameter is cleared.

                using var requestFrame = OutgoingRequestFrame.WithArgs(
                    initial,
                    "opLong",
                    idempotent: false,
                    compress: false,
                    format: default,
                    context: null,
                    p1,
                    (OutputStream ostr, long? p1) => ostr.WriteTaggedLong(1, p1));

                using IncomingResponseFrame responseFrame = initial.InvokeAsync(requestFrame).Result;
                (r1, r2) = responseFrame.ReadReturnValue(initial, istr =>
                {
                    long? l1 = istr.ReadTaggedLong(1);
                    long? l2 = istr.ReadTaggedLong(2);
                    return (l1, l2);
                });
                TestHelper.Assert(r1 == 56);
                TestHelper.Assert(r2 == 56);
            }

            {
                float? p1 = null;
                (float? r1, float? r2) = initial.OpFloat(p1);
                TestHelper.Assert(r1 == null && r2 == null);
                (r1, r2) = initial.OpFloat(null);
                TestHelper.Assert(r1 == null && r2 == null);

                p1 = 1.0f;
                (r1, r2) = initial.OpFloat(p1);
                TestHelper.Assert(r1 == 1.0f && r2 == 1.0f);
                (float? ReturnValue, float? r2) r = initial.OpFloatAsync(p1).Result;
                TestHelper.Assert(r.ReturnValue == 1.0f && r.r2 == 1.0f);
                (r1, r2) = initial.OpFloat(p1);
                TestHelper.Assert(r1 == 1.0f && r2 == 1.0f);
                r = initial.OpFloatAsync(p1).Result;
                TestHelper.Assert(r.ReturnValue == 1.0f && r.r2 == 1.0f);

                (r1, r2) = initial.OpFloat(null);
                TestHelper.Assert(r1 == null && r2 == null); // Ensure out parameter is cleared.

                using var requestFrame = OutgoingRequestFrame.WithArgs(
                    initial,
                    "opFloat",
                    idempotent: false,
                    compress: false,
                    format: default,
                    context: null,
                    p1,
                    (OutputStream ostr, float? p1) => ostr.WriteTaggedFloat(1, p1));

                using IncomingResponseFrame responseFrame = initial.InvokeAsync(requestFrame).Result;
                (r1, r2) = responseFrame.ReadReturnValue(initial, istr =>
                    {
                        float? f1 = istr.ReadTaggedFloat(1);
                        float? f2 = istr.ReadTaggedFloat(2);
                        return (f1, f2);
                    });
                TestHelper.Assert(r1 == 1.0f);
                TestHelper.Assert(r2 == 1.0f);
            }

            {
                double? p1 = null;
                (double? r1, double? r2) = initial.OpDouble(p1);
                TestHelper.Assert(r1 == null && r2 == null);
                (r1, r2) = initial.OpDouble(null);
                TestHelper.Assert(r1 == null && r2 == null);

                p1 = 1.0;
                (r1, r2) = initial.OpDouble(p1);
                TestHelper.Assert(r1 == 1.0 && r2 == 1.0);
                (double? ReturnValue, double? r2) r = initial.OpDoubleAsync(p1).Result;
                TestHelper.Assert(r.ReturnValue == 1.0 && r.r2 == 1.0);
                (r1, r2) = initial.OpDouble(p1);
                TestHelper.Assert(r1 == 1.0 && r2 == 1.0);
                r = initial.OpDoubleAsync(p1).Result;
                TestHelper.Assert(r.ReturnValue == 1.0 && r.r2 == 1.0);

                (r1, r2) = initial.OpDouble(null);
                TestHelper.Assert(r1 == null && r2 == null); // Ensure out parameter is cleared.

                using var requestFrame = OutgoingRequestFrame.WithArgs(
                    initial,
                    "opDouble",
                    idempotent: false,
                    compress: false,
                    format: default,
                    context: null,
                    p1,
                    (OutputStream ostr, double? p1) => ostr.WriteTaggedDouble(1, p1));

                using IncomingResponseFrame responseFrame = initial.InvokeAsync(requestFrame).Result;
                (r1, r2) = responseFrame.ReadReturnValue(initial, istr =>
                    {
                        double? d1 = istr.ReadTaggedDouble(1);
                        double? d2 = istr.ReadTaggedDouble(2);
                        return (d1, d2);
                    });
                TestHelper.Assert(r1 == 1.0);
                TestHelper.Assert(r2 == 1.0);
            }

            {
                string? p1 = null;
                (string? r1, string? r2) = initial.OpString(p1);
                TestHelper.Assert(r1 == null && r2 == null);
                (r1, r2) = initial.OpString(null);
                TestHelper.Assert(r1 == null && r2 == null);
                (r1, r2) = initial.OpString(null); // Implicitly converts to string>(null)
                TestHelper.Assert(r1 == null && r2 == null);

                p1 = "test";
                (r1, r2) = initial.OpString(p1);
                TestHelper.Assert(r1 == "test" && r2 == "test");
                (string? ReturnValue, string? r2) r = initial.OpStringAsync(p1).Result;
                TestHelper.Assert(r.ReturnValue == "test" && r.r2 == "test");
                (r1, r2) = initial.OpString(p1);
                TestHelper.Assert(r1 == "test" && r2 == "test");
                r = initial.OpStringAsync(p1).Result;
                TestHelper.Assert(r.ReturnValue == "test" && r.r2 == "test");

                (r1, r2) = initial.OpString(null);
                TestHelper.Assert(r1 == null && r2 == null); // Ensure out parameter is cleared.

                using var requestFrame = OutgoingRequestFrame.WithArgs(
                    initial,
                    "opString",
                    idempotent: false,
                    compress: false,
                    format: default,
                    context: null,
                    p1,
                    (OutputStream ostr, string? p1) => ostr.WriteTaggedString(1, p1));

                using IncomingResponseFrame responseFrame = initial.InvokeAsync(requestFrame).Result;
                (r1, r2) = responseFrame.ReadReturnValue(initial, istr =>
                {
                    string? s1 = istr.ReadTaggedString(1);
                    string? s2 = istr.ReadTaggedString(2);
                    return (s1, s2);
                });
                TestHelper.Assert(r1 == "test");
                TestHelper.Assert(r2 == "test");
            }

            {
                MyEnum? p1 = null;
                (MyEnum? r1, MyEnum? r2) = initial.OpMyEnum(p1);
                TestHelper.Assert(r1 == null && r2 == null);
                (r1, r2) = initial.OpMyEnum(null);
                TestHelper.Assert(r1 == null && r2 == null);

                p1 = MyEnum.M1;
                (r1, r2) = initial.OpMyEnum(p1);
                TestHelper.Assert(r1 == MyEnum.M1 && r2 == MyEnum.M1);
                (MyEnum? ReturnValue, MyEnum? r2) r = initial.OpMyEnumAsync(p1).Result;
                TestHelper.Assert(r.ReturnValue == MyEnum.M1 && r.r2 == MyEnum.M1);
                (r1, r2) = initial.OpMyEnum(p1);
                TestHelper.Assert(r1 == MyEnum.M1 && r2 == MyEnum.M1);
                r = initial.OpMyEnumAsync(p1).Result;
                TestHelper.Assert(r.ReturnValue == MyEnum.M1 && r.r2 == MyEnum.M1);

                (r1, r2) = initial.OpMyEnum(null);
                TestHelper.Assert(r1 == null && r2 == null); // Ensure out parameter is cleared.

                using var requestFrame = OutgoingRequestFrame.WithArgs(
                    initial,
                    "opMyEnum",
                    idempotent: false,
                    compress: false,
                    format: default,
                    context: null,
                    p1,
                    (OutputStream ostr, MyEnum? p1) => ostr.WriteTaggedSize(1, (int?)p1));

                using IncomingResponseFrame responseFrame = initial.InvokeAsync(requestFrame).Result;
                (r1, r2) = responseFrame.ReadReturnValue(initial, istr =>
                    (istr.ReadTaggedSize(1)?.AsMyEnum(), istr.ReadTaggedSize(2)?.AsMyEnum()));
                TestHelper.Assert(r1 == MyEnum.M1);
                TestHelper.Assert(r2 == MyEnum.M1);
            }

            {
                SmallStruct? p1 = null;
                (SmallStruct? r1, SmallStruct? r2) = initial.OpSmallStruct(p1);
                TestHelper.Assert(r1 == null && r2 == null);
                (r1, r2) = initial.OpSmallStruct(null);
                TestHelper.Assert(r1 == null && r2 == null);

                p1 = new SmallStruct(56);
                (r1, r2) = initial.OpSmallStruct(p1);
                TestHelper.Assert(r1!.Value.M == 56 && r2!.Value.M == 56);
                (SmallStruct? ReturnValue, SmallStruct? r2) r = initial.OpSmallStructAsync(p1).Result;
                TestHelper.Assert(r1!.Value.M == 56 && r2!.Value.M == 56);
                (r1, r2) = initial.OpSmallStruct(p1);
                TestHelper.Assert(r1!.Value.M == 56 && r2!.Value.M == 56);
                r = initial.OpSmallStructAsync(p1).Result;
                TestHelper.Assert(r.ReturnValue!.Value.M == 56 && r.r2!.Value.M == 56);

                (r1, r2) = initial.OpSmallStruct(null);
                TestHelper.Assert(r1 == null && r2 == null); // Ensure out parameter is cleared.

                using var requestFrame = OutgoingRequestFrame.WithArgs(
                    initial,
                    "opSmallStruct",
                    idempotent: false,
                    compress: false,
                    format: default,
                    context: null,
                    p1,
                    (OutputStream ostr, SmallStruct? p1) => ostr.WriteTaggedStruct(1, p1, 1));

                using IncomingResponseFrame responseFrame = initial.InvokeAsync(requestFrame).Result;
                (r1, r2) = responseFrame.ReadReturnValue(initial, istr =>
                    (istr.ReadTaggedStruct(1, fixedSize: true, istr => new SmallStruct(istr)),
                        istr.ReadTaggedStruct(2, fixedSize: true, istr => new SmallStruct(istr))));

                TestHelper.Assert(r1!.Value.M == 56);
                TestHelper.Assert(r2!.Value.M == 56);
            }

            {
                FixedStruct? p1 = null;
                (FixedStruct? r1, FixedStruct? r2) = initial.OpFixedStruct(p1);
                TestHelper.Assert(r1 == null && r2 == null);
                (r1, r2) = initial.OpFixedStruct(null);
                TestHelper.Assert(r1 == null && r2 == null);

                p1 = new FixedStruct(56);
                (r1, r2) = initial.OpFixedStruct(p1);
                TestHelper.Assert(r1!.Value.M == 56 && r2!.Value.M == 56);
                (FixedStruct? ReturnValue, FixedStruct? r2) r = initial.OpFixedStructAsync(p1).Result;
                TestHelper.Assert(r.ReturnValue!.Value.M == 56 && r.r2!.Value.M == 56);
                (r1, r2) = initial.OpFixedStruct(p1);
                TestHelper.Assert(r1!.Value.M == 56 && r2!.Value.M == 56);
                r = initial.OpFixedStructAsync(p1).Result;
                TestHelper.Assert(r.ReturnValue!.Value.M == 56 && r.r2!.Value.M == 56);

                (r1, r2) = initial.OpFixedStruct(null);
                TestHelper.Assert(r1 == null && r2 == null); // Ensure out parameter is cleared.

                using var requestFrame = OutgoingRequestFrame.WithArgs(
                    initial,
                    "opFixedStruct",
                    idempotent: false,
                    compress: false,
                    format: default,
                    context: null,
                    p1,
                    (OutputStream ostr, FixedStruct? p1) => ostr.WriteTaggedStruct(1, p1, 4));

                using IncomingResponseFrame responseFrame = initial.InvokeAsync(requestFrame).Result;
                (r1, r2) = responseFrame.ReadReturnValue(initial, istr =>
                    (istr.ReadTaggedStruct(1, fixedSize: true, istr => new FixedStruct(istr)),
                        istr.ReadTaggedStruct(2, fixedSize: true, istr => new FixedStruct(istr))));
                TestHelper.Assert(r1!.Value.M == 56);
                TestHelper.Assert(r2!.Value.M == 56);
            }

            {
                VarStruct? p1 = null;
                (VarStruct? r1, VarStruct? r2) = initial.OpVarStruct(p1);
                TestHelper.Assert(r1 == null && r2 == null);
                (r1, r2) = initial.OpVarStruct(null);
                TestHelper.Assert(r1 == null && r2 == null);

                p1 = new VarStruct("test");
                (r1, r2) = initial.OpVarStruct(p1);
                TestHelper.Assert(r1!.Value.M.Equals("test") && r2!.Value.M.Equals("test"));

                // Test null struct
                (r1, r2) = initial.OpVarStruct(null);
                TestHelper.Assert(r1 == null && r2 == null);

                (VarStruct? ReturnValue, VarStruct? r2) r = initial.OpVarStructAsync(p1).Result;
                TestHelper.Assert(r.ReturnValue!.Value.M.Equals("test") && r.r2!.Value.M.Equals("test"));
                (r1, r2) = initial.OpVarStruct(p1);
                TestHelper.Assert(r1!.Value.M.Equals("test") && r2!.Value.M.Equals("test"));
                r = initial.OpVarStructAsync(p1).Result;
                TestHelper.Assert(r.ReturnValue!.Value.M.Equals("test") && r.r2!.Value.M.Equals("test"));

                (r1, r2) = initial.OpVarStruct(null);
                TestHelper.Assert(r1 == null && r2 == null); // Ensure out parameter is cleared.

                using var requestFrame = OutgoingRequestFrame.WithArgs(
                    initial,
                    "opVarStruct",
                    idempotent: false,
                    compress: false,
                    format: default,
                    context: null,
                    p1,
                    (OutputStream ostr, VarStruct? p1) =>
                    {
                        TestHelper.Assert(p1 != null);
                        ostr.WriteTaggedStruct(1, p1);
                    });

                using IncomingResponseFrame responseFrame = initial.InvokeAsync(requestFrame).Result;
                (r1, r2) = responseFrame.ReadReturnValue(initial, istr =>
                    (istr.ReadTaggedStruct(1, fixedSize: false, istr => new VarStruct(istr)),
                        istr.ReadTaggedStruct(2, fixedSize: false, istr => new VarStruct(istr))));
                TestHelper.Assert(r1!.Value.M.Equals("test"));
                TestHelper.Assert(r2!.Value.M.Equals("test"));

                // TODO: why are we testing this here?
                /* F f = new F();
                f.af = new A();
                f.af.requiredA = 56;
                f.ae = f.af;

                ostr = new OutputStream(communicator);
                ostr.StartEncapsulation();
                ostr.WriteTagged(1, TaggedFormat.Class);
                ostr.WriteNullableClass(f);
                ostr.WriteTagged(2, TaggedFormat.Class);
                ostr.WriteNullableClass(f.ae);
                ostr.EndEncapsulation();
                var inEncaps = ostr.ToArray();

                var istr = new InputStream(communicator, inEncaps);
                istr.StartEncapsulation();
                test(istr.ReadTagged(2, TaggedFormat.Class));
                var a = istr.ReadNullableClass<A>();
                istr.EndEncapsulation();
                test(a != null && a.requiredA == 56);*/
            }

            {
                byte[]? p1 = null;
                (byte[]? r1, byte[]? r2) = initial.OpByteSeq(p1);
                TestHelper.Assert(r1 == null && r2 == null);
                (r1, r2) = initial.OpByteSeq(null);
                TestHelper.Assert(r1 == null && r2 == null);

                p1 = Enumerable.Range(0, 100).Select(x => (byte)56).ToArray();
                (r1, r2) = initial.OpByteSeq(p1);
                TestHelper.Assert(Enumerable.SequenceEqual(r1!, p1) && Enumerable.SequenceEqual(r2!, p1));
                (r1, r2) = initial.OpByteSeqAsync(p1).Result;
                TestHelper.Assert(Enumerable.SequenceEqual(r1!, p1) && Enumerable.SequenceEqual(r2!, p1));
                (r1, r2) = initial.OpByteSeq(p1);
                TestHelper.Assert(Enumerable.SequenceEqual(r1!, p1) && Enumerable.SequenceEqual(r2!, p1));
                (r1, r2) = initial.OpByteSeqAsync(p1).Result;
                TestHelper.Assert(Enumerable.SequenceEqual(r1!, p1) && Enumerable.SequenceEqual(r2!, p1));

                (r1, r2) = initial.OpByteSeq(null);
                TestHelper.Assert(r1 == null && r2 == null); // Ensure out parameter is cleared.

                using var requestFrame = OutgoingRequestFrame.WithArgs(
                    initial,
                    "opByteSeq",
                    idempotent: false,
                    compress: false,
                    format: default,
                    context: null,
                    p1,
                    (OutputStream ostr, byte[]? p1) => ostr.WriteTaggedArray(1, p1));

                using IncomingResponseFrame responseFrame = initial.InvokeAsync(requestFrame).Result;
                (r1, r2) = responseFrame.ReadReturnValue(initial, istr =>
                    (istr.ReadTaggedArray<byte>(1), istr.ReadTaggedArray<byte>(2)));

                TestHelper.Assert(Enumerable.SequenceEqual(p1, r1!));
                TestHelper.Assert(Enumerable.SequenceEqual(p1, r2!));
            }

            {
                bool[]? p1 = null;
                (bool[]? r1, bool[]? r2) = initial.OpBoolSeq(p1);
                TestHelper.Assert(r1 == null && r2 == null);
                (r1, r2) = initial.OpBoolSeq(null);
                TestHelper.Assert(r1 == null && r2 == null);

                p1 = Enumerable.Range(0, 100).Select(_ => true).ToArray();
                (r1, r2) = initial.OpBoolSeq(p1);
                TestHelper.Assert(Enumerable.SequenceEqual(r1!, p1) && Enumerable.SequenceEqual(r2!, p1));
                (r1, r2) = initial.OpBoolSeqAsync(p1).Result;
                TestHelper.Assert(Enumerable.SequenceEqual(r1!, p1) && Enumerable.SequenceEqual(r2!, p1));
                (r1, r2) = initial.OpBoolSeq(p1);
                TestHelper.Assert(Enumerable.SequenceEqual(r1!, p1) && Enumerable.SequenceEqual(r2!, p1));
                (r1, r2) = initial.OpBoolSeqAsync(p1).Result;
                TestHelper.Assert(Enumerable.SequenceEqual(r1!, p1) && Enumerable.SequenceEqual(r2!, p1));

                (r1, r2) = initial.OpBoolSeq(null);
                TestHelper.Assert(r1 == null && r2 == null); // Ensure out parameter is cleared.

                using var requestFrame = OutgoingRequestFrame.WithArgs(
                    initial,
                    "opBoolSeq",
                    idempotent: false,
                    compress: false,
                    format: default,
                    context: null,
                    p1,
                    (OutputStream ostr, bool[]? p1) => ostr.WriteTaggedArray(1, p1));

                using IncomingResponseFrame responseFrame = initial.InvokeAsync(requestFrame).Result;
                (r1, r2) = responseFrame.ReadReturnValue(initial, istr =>
                    (istr.ReadTaggedArray<bool>(1), istr.ReadTaggedArray<bool>(2)));
                TestHelper.Assert(Enumerable.SequenceEqual(p1, r1!));
                TestHelper.Assert(Enumerable.SequenceEqual(p1, r2!));
            }

            {
                short[]? p1 = null;
                (short[]? r1, short[]? r2) = initial.OpShortSeq(p1);
                TestHelper.Assert(r1 == null && r2 == null);
                (r1, r2) = initial.OpShortSeq(null);
                TestHelper.Assert(r1 == null && r2 == null);

                p1 = Enumerable.Range(0, 100).Select(_ => (short)56).ToArray();
                (r1, r2) = initial.OpShortSeq(p1);
                TestHelper.Assert(Enumerable.SequenceEqual(r1!, p1) && Enumerable.SequenceEqual(r2!, p1));
                (r1, r2) = initial.OpShortSeqAsync(p1).Result;

                TestHelper.Assert(Enumerable.SequenceEqual(r1!, p1) && Enumerable.SequenceEqual(r2!, p1));
                (r1, r2) = initial.OpShortSeq(p1);
                TestHelper.Assert(Enumerable.SequenceEqual(r1!, p1) && Enumerable.SequenceEqual(r2!, p1));
                (r1, r2) = initial.OpShortSeqAsync(p1).Result;
                TestHelper.Assert(Enumerable.SequenceEqual(r1!, p1) && Enumerable.SequenceEqual(r2!, p1));

                (r1, r2) = initial.OpShortSeq(null);
                TestHelper.Assert(r1 == null && r2 == null); // Ensure out parameter is cleared.

                using var requestFrame = OutgoingRequestFrame.WithArgs(
                    initial,
                    "opShortSeq",
                    idempotent: false,
                    compress: false,
                    format: default,
                    context: null,
                    p1,
                    (OutputStream ostr, short[]? p1) => ostr.WriteTaggedArray(1, p1));

                using IncomingResponseFrame responseFrame = initial.InvokeAsync(requestFrame).Result;
                (r1, r2) = responseFrame.ReadReturnValue(initial, istr =>
                    (istr.ReadTaggedArray<short>(1), istr.ReadTaggedArray<short>(2)));
                TestHelper.Assert(Enumerable.SequenceEqual(p1, r1!));
                TestHelper.Assert(Enumerable.SequenceEqual(p1, r2!));
            }

            {
                int[]? p1 = null;
                (int[]? r1, int[]? r2) = initial.OpIntSeq(p1);
                TestHelper.Assert(r1 == null && r2 == null);
                (r1, r2) = initial.OpIntSeq(null);
                TestHelper.Assert(r1 == null && r2 == null);

                p1 = Enumerable.Range(0, 100).Select(_ => 56).ToArray();
                (r1, r2) = initial.OpIntSeq(p1);
                TestHelper.Assert(Enumerable.SequenceEqual(r1!, p1) && Enumerable.SequenceEqual(r2!, p1));
                (r1, r2) = initial.OpIntSeqAsync(p1).Result;
                TestHelper.Assert(Enumerable.SequenceEqual(r1!, p1) && Enumerable.SequenceEqual(r2!, p1));
                (r1, r2) = initial.OpIntSeq(p1);
                TestHelper.Assert(Enumerable.SequenceEqual(r1!, p1) && Enumerable.SequenceEqual(r2!, p1));
                (r1, r2) = initial.OpIntSeqAsync(p1).Result;
                TestHelper.Assert(Enumerable.SequenceEqual(r1!, p1) && Enumerable.SequenceEqual(r2!, p1));

                (r1, r2) = initial.OpIntSeq(null);
                TestHelper.Assert(r1 == null && r2 == null); // Ensure out parameter is cleared.

                using var requestFrame = OutgoingRequestFrame.WithArgs(
                    initial,
                    "opIntSeq",
                    idempotent: false,
                    compress: false,
                    format: default,
                    context: null,
                    p1,
                    (OutputStream ostr, int[]? p1) => ostr.WriteTaggedArray(1, p1));

                using IncomingResponseFrame responseFrame = initial.InvokeAsync(requestFrame).Result;
                (r1, r2) = responseFrame.ReadReturnValue(initial, istr =>
                    (istr.ReadTaggedArray<int>(1), istr.ReadTaggedArray<int>(2)));
                TestHelper.Assert(Enumerable.SequenceEqual(p1, r1!));
                TestHelper.Assert(Enumerable.SequenceEqual(p1, r2!));
            }

            {
                long[]? p1 = null;
                (long[]? r1, long[]? r2) = initial.OpLongSeq(p1);
                TestHelper.Assert(r1 == null && r2 == null);
                (r1, r2) = initial.OpLongSeq(null);
                TestHelper.Assert(r1 == null && r2 == null);

                p1 = Enumerable.Range(0, 100).Select(_ => 56L).ToArray();
                (r1, r2) = initial.OpLongSeq(p1);
                TestHelper.Assert(Enumerable.SequenceEqual(r1!, p1) && Enumerable.SequenceEqual(r2!, p1));
                (r1, r2) = initial.OpLongSeqAsync(p1).Result;
                TestHelper.Assert(Enumerable.SequenceEqual(r1!, p1) && Enumerable.SequenceEqual(r2!, p1));
                (r1, r2) = initial.OpLongSeq(p1);
                TestHelper.Assert(Enumerable.SequenceEqual(r1!, p1) && Enumerable.SequenceEqual(r2!, p1));
                (r1, r2) = initial.OpLongSeqAsync(p1).Result;
                TestHelper.Assert(Enumerable.SequenceEqual(r1!, p1) && Enumerable.SequenceEqual(r2!, p1));

                (r1, r2) = initial.OpLongSeq(null);
                TestHelper.Assert(r1 == null && r2 == null); // Ensure out parameter is cleared.

                using var requestFrame = OutgoingRequestFrame.WithArgs(
                    initial,
                    "opLongSeq",
                    idempotent: false,
                    compress: false,
                    format: default,
                    context: null,
                    p1,
                    (OutputStream ostr, long[]? p1) => ostr.WriteTaggedArray(1, p1));

                using IncomingResponseFrame responseFrame = initial.InvokeAsync(requestFrame).Result;
                (r1, r2) = responseFrame.ReadReturnValue(initial, istr =>
                    (istr.ReadTaggedArray<long>(1), istr.ReadTaggedArray<long>(2)));
                TestHelper.Assert(Enumerable.SequenceEqual(p1, r1!));
                TestHelper.Assert(Enumerable.SequenceEqual(p1, r2!));
            }

            {
                float[]? p1 = null;
                (float[]? r1, float[]? r2) = initial.OpFloatSeq(p1);
                TestHelper.Assert(r1 == null && r2 == null);
                (r1, r2) = initial.OpFloatSeq(null);
                TestHelper.Assert(r1 == null && r2 == null);

                p1 = Enumerable.Range(0, 100).Select(_ => 1.0f).ToArray();
                (r1, r2) = initial.OpFloatSeq(p1);
                TestHelper.Assert(Enumerable.SequenceEqual(r1!, p1) && Enumerable.SequenceEqual(r2!, p1));
                (r1, r2) = initial.OpFloatSeqAsync(p1).Result;
                TestHelper.Assert(Enumerable.SequenceEqual(r1!, p1) && Enumerable.SequenceEqual(r2!, p1));
                (r1, r2) = initial.OpFloatSeq(p1);
                TestHelper.Assert(Enumerable.SequenceEqual(r1!, p1) && Enumerable.SequenceEqual(r2!, p1));
                (r1, r2) = initial.OpFloatSeqAsync(p1).Result;
                TestHelper.Assert(Enumerable.SequenceEqual(r1!, p1) && Enumerable.SequenceEqual(r2!, p1));

                (r1, r2) = initial.OpFloatSeq(null);
                TestHelper.Assert(r1 == null && r2 == null); // Ensure out parameter is cleared.

                using var requestFrame = OutgoingRequestFrame.WithArgs(
                    initial,
                    "opFloatSeq",
                    idempotent: false,
                    compress: false,
                    format: default,
                    context: null,
                    p1,
                    (OutputStream ostr, float[]? p1) => ostr.WriteTaggedArray(1, p1));

                using IncomingResponseFrame responseFrame = initial.InvokeAsync(requestFrame).Result;
                (r1, r2) = responseFrame.ReadReturnValue(initial, istr =>
                    (istr.ReadTaggedArray<float>(1), istr.ReadTaggedArray<float>(2)));
                TestHelper.Assert(Enumerable.SequenceEqual(p1, r1!));
                TestHelper.Assert(Enumerable.SequenceEqual(p1, r2!));
            }

            {
                double[]? p1 = null;
                (double[]? r1, double[]? r2) = initial.OpDoubleSeq(p1);
                TestHelper.Assert(r1 == null && r2 == null);
                (r1, r2) = initial.OpDoubleSeq(null);
                TestHelper.Assert(r1 == null && r2 == null);

                p1 = Enumerable.Range(0, 100).Select(_ => 1.0).ToArray();
                (r1, r2) = initial.OpDoubleSeq(p1);
                TestHelper.Assert(Enumerable.SequenceEqual(r1!, p1) && Enumerable.SequenceEqual(r2!, p1));
                (r1, r2) = initial.OpDoubleSeqAsync(p1).Result;
                TestHelper.Assert(Enumerable.SequenceEqual(r1!, p1) && Enumerable.SequenceEqual(r2!, p1));
                (r1, r2) = initial.OpDoubleSeq(p1);
                TestHelper.Assert(Enumerable.SequenceEqual(r1!, p1) && Enumerable.SequenceEqual(r2!, p1));
                (r1, r2) = initial.OpDoubleSeqAsync(p1).Result;
                TestHelper.Assert(Enumerable.SequenceEqual(r1!, p1) && Enumerable.SequenceEqual(r2!, p1));

                (r1, r2) = initial.OpDoubleSeq(null);
                TestHelper.Assert(r1 == null && r2 == null); // Ensure out parameter is cleared.

                using var requestFrame = OutgoingRequestFrame.WithArgs(
                    initial,
                    "opDoubleSeq",
                    idempotent: false,
                    compress: false,
                    format: default,
                    context: null,
                    p1,
                    (OutputStream ostr, double[]? p1) => ostr.WriteTaggedArray(1, p1));

                using IncomingResponseFrame responseFrame = initial.InvokeAsync(requestFrame).Result;
                (r1, r2) = responseFrame.ReadReturnValue(initial, istr =>
                    (istr.ReadTaggedArray<double>(1), istr.ReadTaggedArray<double>(2)));

                TestHelper.Assert(Enumerable.SequenceEqual(p1, r1!));
                TestHelper.Assert(Enumerable.SequenceEqual(p1, r2!));
            }

            {
                string[]? p1 = null;
                (string[]? r1, string[]? r2) = initial.OpStringSeq(p1);
                TestHelper.Assert(r1 == null && r2 == null);
                (r1, r2) = initial.OpStringSeq(null);
                TestHelper.Assert(r1 == null && r2 == null);

                p1 = Enumerable.Range(0, 10).Select(_ => "test1").ToArray();
                (r1, r2) = initial.OpStringSeq(p1);
                TestHelper.Assert(Enumerable.SequenceEqual(r1!, p1) && Enumerable.SequenceEqual(r2!, p1));
                (r1, r2) = initial.OpStringSeqAsync(p1).Result;
                TestHelper.Assert(Enumerable.SequenceEqual(r1!, p1) && Enumerable.SequenceEqual(r2!, p1));
                (r1, r2) = initial.OpStringSeq(p1);
                TestHelper.Assert(Enumerable.SequenceEqual(r1!, p1) && Enumerable.SequenceEqual(r2!, p1));
                (r1, r2) = initial.OpStringSeqAsync(p1).Result;
                TestHelper.Assert(Enumerable.SequenceEqual(r1!, p1) && Enumerable.SequenceEqual(r2!, p1));

                (r1, r2) = initial.OpStringSeq(null);
                TestHelper.Assert(r1 == null && r2 == null); // Ensure out parameter is cleared.

                using var requestFrame = OutgoingRequestFrame.WithArgs(
                    initial,
                    "opStringSeq",
                    idempotent: false,
                    compress: false,
                    format: default,
                    context: null,
                    p1,
                    (OutputStream ostr, string[]? p1) =>
                        ostr.WriteTaggedSequence(1, p1, (ost, s) => ostr.WriteString(s)));

                using IncomingResponseFrame responseFrame = initial.InvokeAsync(requestFrame).Result;
                (r1, r2) = responseFrame.ReadReturnValue(initial, istr =>
                    (istr.ReadTaggedArray(1, 1, fixedSize: false, istr => istr.ReadString()),
                       istr.ReadTaggedArray(2, 1, fixedSize: false, istr => istr.ReadString())));
                TestHelper.Assert(Enumerable.SequenceEqual(p1, r1!));
                TestHelper.Assert(Enumerable.SequenceEqual(p1, r2!));
            }

            {
                SmallStruct[]? p1 = null;
                (SmallStruct[]? r1, SmallStruct[]? r2) = initial.OpSmallStructSeq(p1);
                TestHelper.Assert(r1 == null && r2 == null);
                (r1, r2) = initial.OpSmallStructSeq(null);
                TestHelper.Assert(r1 == null && r2 == null);

                p1 = Enumerable.Range(0, 10).Select(_ => new SmallStruct()).ToArray();
                (r1, r2) = initial.OpSmallStructSeq(p1);
                TestHelper.Assert(Enumerable.SequenceEqual(r1!, p1) && Enumerable.SequenceEqual(r2!, p1));
                (r1, r2) = initial.OpSmallStructSeqAsync(p1).Result;
                TestHelper.Assert(Enumerable.SequenceEqual(r1!, p1) && Enumerable.SequenceEqual(r2!, p1));
                (r1, r2) = initial.OpSmallStructSeq(p1);
                TestHelper.Assert(Enumerable.SequenceEqual(r1!, p1) && Enumerable.SequenceEqual(r2!, p1));
                (r1, r2) = initial.OpSmallStructSeqAsync(p1).Result;
                TestHelper.Assert(Enumerable.SequenceEqual(r1!, p1) && Enumerable.SequenceEqual(r2!, p1));

                (r1, r2) = initial.OpSmallStructSeq(null);
                TestHelper.Assert(r1 == null && r2 == null); // Ensure out parameter is cleared.

                using var requestFrame = OutgoingRequestFrame.WithArgs(
                    initial,
                    "opSmallStructSeq",
                    idempotent: false,
                    compress: false,
                    format: default,
                    context: null,
                    p1,
                    (OutputStream ostr, SmallStruct[]? p1) => ostr.WriteTaggedSequence(1, p1, 1,
                        (ostr, st) => ostr.WriteStruct(st)));

                using IncomingResponseFrame responseFrame = initial.InvokeAsync(requestFrame).Result;
                (r1, r2) = responseFrame.ReadReturnValue(initial, istr =>
                    (istr.ReadTaggedArray(1, 1, fixedSize: true, istr => new SmallStruct(istr)),
                        istr.ReadTaggedArray(2, 1, fixedSize: true, istr => new SmallStruct(istr))));

                TestHelper.Assert(Enumerable.SequenceEqual(p1, r1!));
                TestHelper.Assert(Enumerable.SequenceEqual(p1, r2!));
            }

            {
                List<SmallStruct>? p1 = null;
                (List<SmallStruct>? r1, List<SmallStruct>? r2) = initial.OpSmallStructList(p1);
                TestHelper.Assert(r1 == null && r2 == null);
                (r1, r2) = initial.OpSmallStructList(null);
                TestHelper.Assert(r1 == null && r2 == null);

                p1 = new List<SmallStruct>();
                for (int i = 0; i < 10; ++i)
                {
                    p1.Add(new SmallStruct());
                }
                (r1, r2) = initial.OpSmallStructList(p1);
                TestHelper.Assert(Enumerable.SequenceEqual(r1!, p1));
                (r1, r2) = initial.OpSmallStructListAsync(p1).Result;
                TestHelper.Assert(Enumerable.SequenceEqual(r1!, p1));
                (r1, r2) = initial.OpSmallStructList(p1);
                TestHelper.Assert(Enumerable.SequenceEqual(r1!, p1));
                (r1, r2) = initial.OpSmallStructListAsync(p1).Result;
                TestHelper.Assert(Enumerable.SequenceEqual(r1!, p1));

                (r1, r2) = initial.OpSmallStructList(null);
                TestHelper.Assert(r1 == null && r2 == null); // Ensure out parameter is cleared.

                using var requestFrame = OutgoingRequestFrame.WithArgs(
                    initial,
                    "opSmallStructList",
                    idempotent: false,
                    compress: false,
                    format: default,
                    context: null,
                    p1,
                    (OutputStream ostr, List<SmallStruct>? p1) => ostr.WriteTaggedSequence(
                        1,
                        p1,
                        1,
                        (ostr, st) => ostr.WriteStruct(st)));

                using IncomingResponseFrame responseFrame = initial.InvokeAsync(requestFrame).Result;
                (r1, r2) = responseFrame.ReadReturnValue(initial, istr =>
                    {
                        List<SmallStruct>? list1 =
                            istr.ReadTaggedSequence(1, 1, fixedSize: true, istr => new SmallStruct(istr)) is
                                ICollection<SmallStruct> collection1 ? new List<SmallStruct>(collection1) : null;

                        List<SmallStruct>? list2 =
                            istr.ReadTaggedSequence(2, 1, fixedSize: true, istr => new SmallStruct(istr)) is
                                ICollection<SmallStruct> collection2 ? new List<SmallStruct>(collection2) : null;

                        return (list1, list2);
                    });
                TestHelper.Assert(Enumerable.SequenceEqual(p1, r1!));
                TestHelper.Assert(Enumerable.SequenceEqual(p1, r2!));
            }

            {
                FixedStruct[]? p1 = null;
                (FixedStruct[]? r1, FixedStruct[]? r2) = initial.OpFixedStructSeq(p1);
                TestHelper.Assert(r1 == null && r2 == null);
                (r1, r2) = initial.OpFixedStructSeq(null);
                TestHelper.Assert(r1 == null && r2 == null);

                p1 = Enumerable.Range(0, 10).Select(_ => new FixedStruct()).ToArray();
                (r1, r2) = initial.OpFixedStructSeq(p1);
                TestHelper.Assert(Enumerable.SequenceEqual(r1!, p1) && Enumerable.SequenceEqual(r2!, p1));
                (r1, r2) = initial.OpFixedStructSeqAsync(p1).Result;
                TestHelper.Assert(Enumerable.SequenceEqual(r1!, p1) && Enumerable.SequenceEqual(r2!, p1));
                (r1, r2) = initial.OpFixedStructSeq(p1);
                TestHelper.Assert(Enumerable.SequenceEqual(r1!, p1) && Enumerable.SequenceEqual(r2!, p1));
                (r1, r2) = initial.OpFixedStructSeqAsync(p1).Result;
                TestHelper.Assert(Enumerable.SequenceEqual(r1!, p1) && Enumerable.SequenceEqual(r2!, p1));

                (r1, r2) = initial.OpFixedStructSeq(null);
                TestHelper.Assert(r1 == null && r2 == null); // Ensure out parameter is cleared.

                using var requestFrame = OutgoingRequestFrame.WithArgs(
                    initial,
                    "opFixedStructSeq",
                    idempotent: false,
                    compress: false,
                    format: default,
                    context: null,
                    p1,
                    (OutputStream ostr, FixedStruct[]? p1) => ostr.WriteTaggedSequence(
                        1,
                        p1,
                        4,
                        (ostr, st) => ostr.WriteStruct(st)));

                using IncomingResponseFrame responseFrame = initial.InvokeAsync(requestFrame).Result;
                (r1, r2) = responseFrame.ReadReturnValue(initial, istr =>
                    (istr.ReadTaggedArray(1, 4, fixedSize: true, istr => new FixedStruct(istr)),
                        istr.ReadTaggedArray(2, 4, fixedSize: true, istr => new FixedStruct(istr))));

                TestHelper.Assert(Enumerable.SequenceEqual(p1, r1!));
                TestHelper.Assert(Enumerable.SequenceEqual(p1, r2!));
            }

            {
                LinkedList<FixedStruct>? p1 = null;
                (LinkedList<FixedStruct>? r1, LinkedList<FixedStruct>? r2) = initial.OpFixedStructList(p1);
                TestHelper.Assert(r1 == null && r2 == null);
                (r1, r2) = initial.OpFixedStructList(null);
                TestHelper.Assert(r1 == null && r2 == null);

                p1 = new LinkedList<FixedStruct>();
                for (int i = 0; i < 10; ++i)
                {
                    p1.AddLast(new FixedStruct());
                }
                (r1, r2) = initial.OpFixedStructList(p1);
                TestHelper.Assert(Enumerable.SequenceEqual(r1!, p1) && Enumerable.SequenceEqual(r2!, p1));
                (r1, r2) = initial.OpFixedStructListAsync(p1).Result;
                TestHelper.Assert(Enumerable.SequenceEqual(r1!, p1) && Enumerable.SequenceEqual(r2!, p1));
                (r1, r2) = initial.OpFixedStructList(p1);
                TestHelper.Assert(Enumerable.SequenceEqual(r1!, p1) && Enumerable.SequenceEqual(r2!, p1));
                (r1, r2) = initial.OpFixedStructListAsync(p1).Result;
                TestHelper.Assert(Enumerable.SequenceEqual(r1!, p1) && Enumerable.SequenceEqual(r2!, p1));

                (r1, r2) = initial.OpFixedStructList(null);
                TestHelper.Assert(r1 == null && r2 == null); // Ensure out parameter is cleared.

                using var requestFrame = OutgoingRequestFrame.WithArgs(
                    initial,
                    "opFixedStructList",
                    idempotent: false,
                    compress: false,
                    format: default,
                    context: null,
                    p1,
                    (OutputStream ostr, LinkedList<FixedStruct>? p1) => ostr.WriteTaggedSequence(
                        1,
                        p1,
                        4,
                        (ostr, st) => ostr.WriteStruct(st)));

                using IncomingResponseFrame responseFrame = initial.InvokeAsync(requestFrame).Result;
                (r1, r2) = responseFrame.ReadReturnValue(initial, istr =>
                    {
                        LinkedList<FixedStruct>? list1 =
                            istr.ReadTaggedSequence(1, 4, fixedSize: true, istr => new FixedStruct(istr)) is
                                ICollection<FixedStruct> collection1 ? new LinkedList<FixedStruct>(collection1) : null;

                        LinkedList<FixedStruct>? list2 =
                            istr.ReadTaggedSequence(2, 4, fixedSize: true, istr => new FixedStruct(istr)) is
                                ICollection<FixedStruct> collection2 ? new LinkedList<FixedStruct>(collection2) : null;

                        return (list1, list2);
                    });
                TestHelper.Assert(Enumerable.SequenceEqual(p1, r1!));
                TestHelper.Assert(Enumerable.SequenceEqual(p1, r2!));
            }

            {
                VarStruct[]? p1 = null;
                (VarStruct[]? r1, VarStruct[]? r2) = initial.OpVarStructSeq(p1);
                TestHelper.Assert(r1 == null && r2 == null);
                (r1, r2) = initial.OpVarStructSeq(null);
                TestHelper.Assert(r1 == null && r2 == null);

                p1 = Enumerable.Range(0, 10).Select(_ => new VarStruct("")).ToArray();
                (r1, r2) = initial.OpVarStructSeq(p1);
                TestHelper.Assert(Enumerable.SequenceEqual(r1!, p1) && Enumerable.SequenceEqual(r2!, p1));
                (r1, r2) = initial.OpVarStructSeqAsync(p1).Result;
                TestHelper.Assert(Enumerable.SequenceEqual(r1!, p1) && Enumerable.SequenceEqual(r2!, p1));
                (r1, r2) = initial.OpVarStructSeq(p1);
                TestHelper.Assert(Enumerable.SequenceEqual(r1!, p1) && Enumerable.SequenceEqual(r2!, p1));
                (r1, r2) = initial.OpVarStructSeqAsync(p1).Result;
                TestHelper.Assert(Enumerable.SequenceEqual(r1!, p1) && Enumerable.SequenceEqual(r2!, p1));

                (r1, r2) = initial.OpVarStructSeq(null);
                TestHelper.Assert(r1 == null && r2 == null); // Ensure out parameter is cleared.

                using var requestFrame = OutgoingRequestFrame.WithArgs(
                    initial,
                    "opVarStructSeq",
                    idempotent: false,
                    compress: false,
                    format: default,
                    context: null,
                    p1,
                    (OutputStream ostr, VarStruct[]? p1) =>
                        ostr.WriteTaggedSequence(1, p1, (ostr, vs) => ostr.WriteStruct(vs)));

                using IncomingResponseFrame responseFrame = initial.InvokeAsync(requestFrame).Result;
                (r1, r2) = responseFrame.ReadReturnValue(initial, istr =>
                      (istr.ReadTaggedArray(1, 1, fixedSize: false, istr => new VarStruct(istr)),
                        istr.ReadTaggedArray(2, 1, fixedSize: false, istr => new VarStruct(istr))));
                TestHelper.Assert(Enumerable.SequenceEqual(p1, r1!));
                TestHelper.Assert(Enumerable.SequenceEqual(p1, r2!));
            }

            {
                List<int>? p1 = null;
                (List<int>? r1, List<int>? r2) = initial.OpIntList(p1);
                TestHelper.Assert(r1 == null && r2 == null);

                p1 = new List<int> { 7, 8, 13 };
                (r1, r2) = initial.OpIntList(p1);
                TestHelper.Assert(p1.SequenceEqual(r1!) && p1.SequenceEqual(r2!));
            }

            {
                int[][]? p1 = null;
                (int[][]? r1, int[][]? r2) = initial.OpIntSeqSeq(p1);
                TestHelper.Assert(r1 == null && r2 == null);

                p1 = new int[][] { new int[] { 1, 2, 3 }, new int[] { 4, 5, 6 } };
                (r1, r2) = initial.OpIntSeqSeq(p1);

                TestHelper.Assert(r1 != null && r2 != null);
                TestHelper.Assert(p1.Length == r1.Length);
                TestHelper.Assert(p1.Length == r2.Length);
                for (int i = 0; i < p1.Length; ++i)
                {
                    TestHelper.Assert(p1[i].SequenceEqual(r1[i]));
                    TestHelper.Assert(p1[i].SequenceEqual(r2[i]));
                }
            }

            {
                Dictionary<int, int>? p1 = null;
                (Dictionary<int, int>? r1, Dictionary<int, int>? r2) = initial.OpIntIntDict(p1);
                TestHelper.Assert(r1 == null && r2 == null);
                (r1, r2) = initial.OpIntIntDict(null);
                TestHelper.Assert(r1 == null && r2 == null);

                p1 = new Dictionary<int, int>
                {
                    { 1, 2 },
                    { 2, 3 }
                };
                (r1, r2) = initial.OpIntIntDict(p1);
                TestHelper.Assert(Enumerable.SequenceEqual(r1!, p1) && Enumerable.SequenceEqual(r2!, p1));
                (r1, r2) = initial.OpIntIntDictAsync(p1).Result;
                TestHelper.Assert(Enumerable.SequenceEqual(r1!, p1) && Enumerable.SequenceEqual(r2!, p1));
                (r1, r2) = initial.OpIntIntDict(p1);
                TestHelper.Assert(Enumerable.SequenceEqual(r1!, p1) && Enumerable.SequenceEqual(r2!, p1));
                (r1, r2) = initial.OpIntIntDictAsync(p1).Result;
                TestHelper.Assert(Enumerable.SequenceEqual(r1!, p1) && Enumerable.SequenceEqual(r2!, p1));

                (r1, r2) = initial.OpIntIntDict(null);
                TestHelper.Assert(r1 == null && r2 == null); // Ensure out parameter is cleared.

                using var requestFrame = OutgoingRequestFrame.WithArgs(
                    initial,
                    "opIntIntDict",
                    idempotent: false,
                    compress: false,
                    format: default,
                    context: null,
                    p1,
                    (OutputStream ostr, Dictionary<int, int>? p1) => ostr.WriteTaggedDictionary(
                        1,
                        p1,
                        8,
                        (ostr, k) => ostr.WriteInt(k),
                        (ostr, v) => ostr.WriteInt(v)));

                using IncomingResponseFrame responseFrame = initial.InvokeAsync(requestFrame).Result;
                (r1, r2) = responseFrame.ReadReturnValue(initial, istr =>
                    (istr.ReadTaggedDictionary(
                        tag: 1,
                        minKeySize: 4,
                        minValueSize: 4,
                        fixedSize: true,
                        istr => istr.ReadInt(),
                        istr => istr.ReadInt()),
                     istr.ReadTaggedDictionary(
                         tag: 2,
                         4, 4, fixedSize: true,
                         istr => istr.ReadInt(),
                         istr => istr.ReadInt())));

                TestHelper.Assert(Enumerable.SequenceEqual(p1, r1!));
                TestHelper.Assert(Enumerable.SequenceEqual(p1, r2!));
            }

            {
                Dictionary<string, int>? p1 = null;
                (Dictionary<string, int>? r1, Dictionary<string, int>? r2) = initial.OpStringIntDict(p1);
                TestHelper.Assert(r1 == null && r2 == null);
                (r1, r2) = initial.OpStringIntDict(null);
                TestHelper.Assert(r1 == null && r2 == null);

                p1 = new Dictionary<string, int>
                {
                    { "1", 1 },
                    { "2", 2 }
                };
                (r1, r2) = initial.OpStringIntDict(p1);
                TestHelper.Assert(Enumerable.SequenceEqual(r1!, p1) && Enumerable.SequenceEqual(r2!, p1));
                (r1, r2) = initial.OpStringIntDictAsync(p1).Result;
                TestHelper.Assert(Enumerable.SequenceEqual(r1!, p1) && Enumerable.SequenceEqual(r2!, p1));
                (r1, r2) = initial.OpStringIntDict(p1);
                TestHelper.Assert(Enumerable.SequenceEqual(r1!, p1) && Enumerable.SequenceEqual(r2!, p1));
                (r1, r2) = initial.OpStringIntDictAsync(p1).Result;
                TestHelper.Assert(Enumerable.SequenceEqual(r1!, p1) && Enumerable.SequenceEqual(r2!, p1));

                (r1, r2) = initial.OpStringIntDict(null);
                TestHelper.Assert(r1 == null && r2 == null); // Ensure out parameter is cleared.

                using var requestFrame = OutgoingRequestFrame.WithArgs(
                    initial,
                    "opStringIntDict",
                    idempotent: false,
                    compress: false,
                    format: default,
                    context: null,
                    p1,
                    (OutputStream ostr, Dictionary<string, int>? p1) =>
                    {
                        TestHelper.Assert(p1 != null);
                        ostr.WriteTaggedDictionary(1, p1,
                            (ostr, k) => ostr.WriteString(k), (ostr, v) => ostr.WriteInt(v));
                    });

                using IncomingResponseFrame responseFrame = initial.InvokeAsync(requestFrame).Result;
                (r1, r2) = responseFrame.ReadReturnValue(initial, istr =>
                      (istr.ReadTaggedDictionary(1, 1, 4, fixedSize: false, istr => istr.ReadString(),
                                                                            istr => istr.ReadInt()),
                       istr.ReadTaggedDictionary(2, 1, 4, fixedSize: false, istr => istr.ReadString(),
                                                                            istr => istr.ReadInt())));
                TestHelper.Assert(Enumerable.SequenceEqual(p1, r1!));
                TestHelper.Assert(Enumerable.SequenceEqual(p1, r2!));
            }
            output.WriteLine("ok");

            output.Write("testing exceptions with tagged members... ");
            output.Flush();
            {
                try
                {
                    int? a = null;
                    string? b = null;
                    VarStruct? vs = null;
                    initial.OpTaggedException(a, b, vs);
                }
                catch (TaggedException ex)
                {
                    TestHelper.Assert(ex.A == null); // don't use default value for 'a' data member since set explicitly
                                                     // to null by server
                    TestHelper.Assert(ex.B == null);
                    TestHelper.Assert(ex.Vs == null);
                }

                try
                {
                    int? a = 30;
                    string? b = "test";
                    VarStruct? vs = new VarStruct("hello");
                    initial.OpTaggedException(a, b, vs);
                }
                catch (TaggedException ex)
                {
                    TestHelper.Assert(ex.A == 30);
                    TestHelper.Assert(ex.B == "test");
                    TestHelper.Assert(ex.Vs.Equals(new VarStruct("hello")));
                }

                try
                {
                    int? a = null;
                    string? b = null;
                    VarStruct? vs = null;
                    initial.OpDerivedException(a, b, vs);
                }
                catch (DerivedException ex)
                {
                    TestHelper.Assert(ex.A == null); // don't use default value for 'a' data member since set explicitly
                                                     // to null by server
                    TestHelper.Assert(ex.B == null);
                    TestHelper.Assert(ex.Vs == null);
                    TestHelper.Assert(ex.Ss == null);
                    TestHelper.Assert(ex.Vs2 == null);
                }

                try
                {
                    int? a = 30;
                    string? b = "test2";
                    VarStruct? vs = new VarStruct("hello2");
                    initial.OpDerivedException(a, b, vs);
                }
                catch (DerivedException ex)
                {
                    TestHelper.Assert(ex.A == 30);
                    TestHelper.Assert(ex.B == "test2");
                    TestHelper.Assert(ex.Vs.Equals(new VarStruct("hello2")));
                    TestHelper.Assert(ex.Ss == "test2");
                    TestHelper.Assert(ex.Vs2.Equals(new VarStruct("hello2")));
                }

                try
                {
                    int? a = null;
                    string? b = null;
                    VarStruct? vs = null;
                    initial.OpRequiredException(a, b, vs);
                }
                catch (RequiredException ex)
                {
                    TestHelper.Assert(ex.A == null);
                    TestHelper.Assert(ex.B == null);
                    TestHelper.Assert(ex.Vs == null);
                    TestHelper.Assert(ex.Ss == "test");
                    TestHelper.Assert(ex.Vs2.Equals(new VarStruct("")));
                }

                try
                {
                    int? a = 30;
                    string? b = "test2";
                    VarStruct? vs = new VarStruct("hello2");
                    initial.OpRequiredException(a, b, vs);
                }
                catch (RequiredException ex)
                {
                    TestHelper.Assert(ex.A == 30);
                    TestHelper.Assert(ex.B == "test2");
                    TestHelper.Assert(ex.Vs.Equals(new VarStruct("hello2")));
                    TestHelper.Assert(ex.Ss == "test2");
                    TestHelper.Assert(ex.Vs2.Equals(new VarStruct("hello2")));
                }
            }
            output.WriteLine("ok");

            output.Write("testing tagged members with marshaled results... ");
            output.Flush();
            {
                TestHelper.Assert(initial.OpMStruct1() != null);
                TestHelper.Assert(initial.OpMDict1() != null);
                TestHelper.Assert(initial.OpMSeq1() != null);

                {
                    SmallStruct? p1, r1, r2;
                    (r2, r1) = initial.OpMStruct2(null);
                    TestHelper.Assert(r1 == null && r2 == null);

                    p1 = new SmallStruct();
                    (r2, r1) = initial.OpMStruct2(p1);
                    TestHelper.Assert(r1.Equals(p1) && r2.Equals(p1));
                }
                {
                    string[]? p1, r1, r2;
                    (r2, r1) = initial.OpMSeq2(null);
                    TestHelper.Assert(r1 == null && r2 == null);

                    p1 = new string[1] { "hello" };
                    (r2, r1) = initial.OpMSeq2(p1);
                    TestHelper.Assert(Enumerable.SequenceEqual(r1!, p1) && Enumerable.SequenceEqual(r2!, p1));
                }
                {
                    Dictionary<string, int>? p1, r1, r2;
                    (r2, r1) = initial.OpMDict2(null);
                    TestHelper.Assert(r1 == null && r2 == null);

                    p1 = new Dictionary<string, int>
                    {
                        ["test"] = 54
                    };
                    (r2, r1) = initial.OpMDict2(p1);
                    TestHelper.Assert(Enumerable.SequenceEqual(r1!, p1) && Enumerable.SequenceEqual(r2!, p1));
                }
            }
            output.WriteLine("ok");
            await initial.ShutdownAsync();
        }
    }
}

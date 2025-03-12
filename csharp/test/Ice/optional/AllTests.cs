// Copyright (c) ZeroC, Inc.

using System.Diagnostics;

namespace Ice.optional
{
    public class AllTests : global::Test.AllTests
    {
        public static async Task<Test.InitialPrx> allTests(global::Test.TestHelper helper)
        {
            var communicator = helper.communicator();
            var factory = new FactoryI();
            communicator.getValueFactoryManager().add(factory.create, "");

            var output = helper.getWriter();
            output.Write("testing stringToProxy... ");
            output.Flush();
            string @ref = "initial:" + helper.getTestEndpoint(0);
            var @base = communicator.stringToProxy(@ref);
            test(@base is not null);
            output.WriteLine("ok");

            output.Write("testing checked cast... ");
            output.Flush();
            Test.InitialPrx initial = Test.InitialPrxHelper.checkedCast(@base);
            test(initial is not null);
            test(initial.Equals(@base));
            output.WriteLine("ok");

            output.Write("testing optional data members... ");
            output.Flush();

            var oo1 = new Test.OneOptional();
            test(oo1.a is null);
            oo1.a = 15;
            test(oo1.a is not null && oo1.a.Value == 15);

            var oo2 = new Test.OneOptional(16);
            test(oo2.a is not null && oo2.a.Value == 16);

            var mo1 = new Test.MultiOptional();
            mo1.a = 15;
            mo1.b = true;
            mo1.c = 19;
            mo1.d = 78;
            mo1.e = 99;
            mo1.f = (float)5.5;
            mo1.g = 1.0;
            mo1.h = "test";
            mo1.i = Test.MyEnum.MyEnumMember;
            mo1.j = Test.MyInterfacePrxHelper.uncheckedCast(communicator.stringToProxy("test"));
            mo1.bs = [5];
            mo1.ss = ["test", "test2"];
            mo1.iid = new Dictionary<int, int>
                {
                    { 4, 3 }
                };
            mo1.sid = new Dictionary<string, int>
                {
                    { "test", 10 }
                };
            var fs = new Test.FixedStruct();
            fs.m = 78;
            mo1.fs = fs;
            var vs = new Test.VarStruct();
            vs.m = "hello";
            mo1.vs = vs;

            mo1.shs = [1];
            mo1.es = [Test.MyEnum.MyEnumMember, Test.MyEnum.MyEnumMember];
            mo1.fss = [fs];
            mo1.vss = [vs];
            mo1.mips = [Test.MyInterfacePrxHelper.uncheckedCast(communicator.stringToProxy("test"))];

            mo1.ied = new Dictionary<int, Test.MyEnum>
                {
                    { 4, Test.MyEnum.MyEnumMember }
                };
            mo1.ifsd = new Dictionary<int, Test.FixedStruct>
                {
                    { 4, fs }
                };
            mo1.ivsd = new Dictionary<int, Test.VarStruct>
                {
                    { 5, vs }
                };
            mo1.imipd = new Dictionary<int, Test.MyInterfacePrx>
                {
                    { 5, Test.MyInterfacePrxHelper.uncheckedCast(communicator.stringToProxy("test")) }
                };

            mo1.bos = [false, true, false];

            test(mo1.a.Value == (byte)15);
            test(mo1.b.Value);
            test(mo1.c.Value == 19);
            test(mo1.d.Value == 78);
            test(mo1.e.Value == 99);
            test(mo1.f.Value == (float)5.5);
            test(mo1.g.Value == 1.0);
            test(mo1.h == "test");
            test(mo1.i.Value == Test.MyEnum.MyEnumMember);
            test(mo1.j.Equals(communicator.stringToProxy("test")));
            test(ArraysEqual(mo1.bs, [(byte)5]));
            test(ArraysEqual(mo1.ss, ["test", "test2"]));
            test(mo1.iid[4] == 3);
            test(mo1.sid["test"] == 10);
            test(mo1.fs.Value.Equals(new Test.FixedStruct(78)));
            test(mo1.vs.Equals(new Test.VarStruct("hello")));

            test(mo1.shs[0] == (short)1);
            test(mo1.es[0] == Test.MyEnum.MyEnumMember && mo1.es[1] == Test.MyEnum.MyEnumMember);
            test(mo1.fss[0].Equals(new Test.FixedStruct(78)));
            test(mo1.vss[0].Equals(new Test.VarStruct("hello")));
            test(mo1.mips[0].Equals(communicator.stringToProxy("test")));

            test(mo1.ied[4] == Test.MyEnum.MyEnumMember);
            test(mo1.ifsd[4].Equals(new Test.FixedStruct(78)));
            test(mo1.ivsd[5].Equals(new Test.VarStruct("hello")));
            test(mo1.imipd[5].Equals(communicator.stringToProxy("test")));

            test(ArraysEqual(mo1.bos, [false, true, false]));

            output.WriteLine("ok");

            output.Write("testing marshaling... ");
            output.Flush();

            var oo4 = (Test.OneOptional)initial.pingPong(new Test.OneOptional());
            test(oo4.a is null);

            var oo5 = (Test.OneOptional)initial.pingPong(oo1);
            test(oo1.a.Value == oo5.a.Value);

            var mo4 = (Test.MultiOptional)initial.pingPong(new Test.MultiOptional());
            test(mo4.a is null);
            test(mo4.b is null);
            test(mo4.c is null);
            test(mo4.d is null);
            test(mo4.e is null);
            test(mo4.f is null);
            test(mo4.g is null);
            test(mo4.h is null);
            test(mo4.i is null);
            test(mo4.j is null);
            test(mo4.bs is null);
            test(mo4.ss is null);
            test(mo4.iid is null);
            test(mo4.sid is null);
            test(mo4.fs is null);
            test(mo4.vs is null);

            test(mo4.shs is null);
            test(mo4.es is null);
            test(mo4.fss is null);
            test(mo4.vss is null);
            test(mo4.mips is null);

            test(mo4.ied is null);
            test(mo4.ifsd is null);
            test(mo4.ivsd is null);
            test(mo4.imipd is null);

            test(mo4.bos is null);

            var mo5 = (Test.MultiOptional)initial.pingPong(mo1);
            test(mo5.a.Value == mo1.a.Value);
            test(mo5.b.Value == mo1.b.Value);
            test(mo5.c.Value == mo1.c.Value);
            test(mo5.d.Value == mo1.d.Value);
            test(mo5.e.Value == mo1.e.Value);
            test(mo5.f.Value == mo1.f.Value);
            test(mo5.g.Value == mo1.g.Value);
            test(mo5.h.Equals(mo1.h));
            test(mo5.i.Value == mo1.i.Value);
            test(mo5.j.Equals(mo1.j));
            test(ArraysEqual(mo5.bs, mo1.bs));
            test(ArraysEqual(mo5.ss, mo1.ss));
            test(mo5.iid[4] == 3);
            test(mo5.sid["test"] == 10);
            test(mo5.fs.Value.Equals(mo1.fs.Value));
            test(mo5.vs.Equals(mo1.vs));
            test(mo5.shs.SequenceEqual(mo1.shs));
            test(mo5.es[0] == Test.MyEnum.MyEnumMember && mo1.es[1] == Test.MyEnum.MyEnumMember);
            test(mo5.fss[0].Equals(new Test.FixedStruct(78)));
            test(mo5.vss[0].Equals(new Test.VarStruct("hello")));
            test(mo5.mips[0].Equals(communicator.stringToProxy("test")));

            test(mo5.ied[4] == Test.MyEnum.MyEnumMember);
            test(mo5.ifsd[4].Equals(new Test.FixedStruct(78)));
            test(mo5.ivsd[5].Equals(new Test.VarStruct("hello")));
            test(mo5.imipd[5].Equals(communicator.stringToProxy("test")));

            test(ArraysEqual(mo5.bos, [false, true, false]));

            // Clear the first half of the optional members
            var mo6 = new Test.MultiOptional();
            mo6.b = mo5.b;
            mo6.d = mo5.d;
            mo6.f = mo5.f;
            mo6.h = mo5.h;
            mo6.j = mo5.j;
            mo6.bs = mo5.bs;
            mo6.iid = mo5.iid;
            mo6.fs = mo5.fs;
            mo6.shs = mo5.shs;
            mo6.fss = mo5.fss;
            mo6.ifsd = mo5.ifsd;
            mo6.bos = mo5.bos;

            var mo7 = (Test.MultiOptional)initial.pingPong(mo6);
            test(mo7.a is null);
            test(mo7.b.Equals(mo1.b));
            test(mo7.c is null);
            test(mo7.d.Equals(mo1.d));
            test(mo7.e is null);
            test(mo7.f.Equals(mo1.f));
            test(mo7.g is null);
            test(mo7.h.Equals(mo1.h));
            test(mo7.i is null);
            test(mo7.j.Equals(mo1.j));
            test(ArraysEqual(mo7.bs, mo1.bs));
            test(mo7.ss is null);
            test(mo7.iid[4] == 3);
            test(mo7.sid is null);
            test(mo7.fs.Equals(mo1.fs));
            test(mo7.vs is null);

            test(mo7.shs.SequenceEqual(mo1.shs));
            test(mo7.es is null);
            test(mo7.fss[0].Equals(new Test.FixedStruct(78)));
            test(mo7.vss is null);
            test(mo7.mips is null);

            test(mo7.ied is null);
            test(mo7.ifsd[4].Equals(new Test.FixedStruct(78)));
            test(mo7.ivsd is null);
            test(mo7.imipd is null);

            test(ArraysEqual(mo7.bos, [false, true, false]));

            // Clear the second half of the optional members
            var mo8 = new Test.MultiOptional();
            mo8.a = mo5.a;
            mo8.c = mo5.c;
            mo8.e = mo5.e;
            mo8.g = mo5.g;
            mo8.i = mo5.i;
            mo8.ss = mo5.ss;
            mo8.sid = mo5.sid;
            mo8.vs = mo5.vs;

            mo8.es = mo5.es;
            mo8.vss = mo5.vss;
            mo8.mips = mo5.mips;

            mo8.ied = mo5.ied;
            mo8.ivsd = mo5.ivsd;
            mo8.imipd = mo5.imipd;

            var mo9 = (Test.MultiOptional)initial.pingPong(mo8);
            test(mo9.a.Equals(mo1.a));
            test(mo9.b is null);
            test(mo9.c.Equals(mo1.c));
            test(mo9.d is null);
            test(mo9.e.Equals(mo1.e));
            test(mo9.f is null);
            test(mo9.g.Equals(mo1.g));
            test(mo9.h is null);
            test(mo9.i.Equals(mo1.i));
            test(mo9.j is null);
            test(mo9.bs is null);
            test(ArraysEqual(mo9.ss, mo1.ss));
            test(mo9.iid is null);
            test(mo9.sid["test"] == 10);
            test(mo9.fs is null);
            test(mo9.vs.Equals(mo1.vs));

            test(mo9.shs is null);
            test(mo9.es[0] == Test.MyEnum.MyEnumMember && mo9.es[1] == Test.MyEnum.MyEnumMember);
            test(mo9.fss is null);
            test(mo9.vss[0].Equals(new Test.VarStruct("hello")));
            test(mo9.mips[0].Equals(communicator.stringToProxy("test")));

            test(mo9.ied[4] == Test.MyEnum.MyEnumMember);
            test(mo9.ifsd is null);
            test(mo9.ivsd[5].Equals(new Test.VarStruct("hello")));
            test(mo9.imipd[5].Equals(communicator.stringToProxy("test")));

            // Send a request using blobjects. Upon receival, we don't read any of the optional members. This ensures
            // the optional members are skipped even if the receiver knows nothing about them.
            factory.setEnabled(true);
            var os = new OutputStream(communicator);
            os.startEncapsulation();
            os.writeValue(oo1);
            os.endEncapsulation();
            byte[] inEncaps = os.finished();
            test(initial.ice_invoke("pingPong", OperationMode.Normal, inEncaps, out byte[] outEncaps));
            var @in = new InputStream(communicator, outEncaps);
            @in.startEncapsulation();
            var cb = new ReadValueCallbackI();
            @in.readValue(cb.invoke);
            @in.endEncapsulation();
            test(cb.obj is not null && cb.obj is TestValueReader);

            os = new OutputStream(communicator);
            os.startEncapsulation();
            os.writeValue(mo1);
            os.endEncapsulation();
            inEncaps = os.finished();
            test(initial.ice_invoke("pingPong", Ice.OperationMode.Normal, inEncaps, out outEncaps));
            @in = new InputStream(communicator, outEncaps);
            @in.startEncapsulation();
            @in.readValue(cb.invoke);
            @in.endEncapsulation();
            test(cb.obj is not null && cb.obj is TestValueReader);
            factory.setEnabled(false);

            var g = new Test.G();
            g.gg1Opt = new Test.G1("gg1Opt");
            g.gg2 = new Test.G2(10);
            g.gg2Opt = new Test.G2(20);
            g.gg1 = new Test.G1("gg1");
            g = initial.opG(g);
            test("gg1Opt".Equals(g.gg1Opt.a));
            test(10 == g.gg2.a);
            test(20 == g.gg2Opt.Value.a);
            test("gg1".Equals(g.gg1.a));

            initial.opVoid();

            os = new OutputStream(communicator);
            os.startEncapsulation();
            os.writeOptional(1, OptionalFormat.F4);
            os.writeInt(15);
            os.writeOptional(1, OptionalFormat.VSize);
            os.writeString("test");
            os.endEncapsulation();
            inEncaps = os.finished();
            test(initial.ice_invoke("opVoid", OperationMode.Normal, inEncaps, out outEncaps));

            output.WriteLine("ok");

            output.Write("testing marshaling of large containers with fixed size elements... ");
            output.Flush();
            var mc = new Test.MultiOptional();

            mc.bs = new byte[1000];
            mc.shs = new List<short>(300);

            mc.fss = new Test.FixedStruct[300];
            for (int i = 0; i < 300; ++i)
            {
                mc.fss[i] = new Test.FixedStruct();
                mc.shs.Add((short)i);
            }

            mc.ifsd = [];
            for (int i = 0; i < 300; ++i)
            {
                mc.ifsd.Add(i, new Test.FixedStruct());
            }

            mc = (Test.MultiOptional)initial.pingPong(mc);
            test(mc.bs.Length == 1000);
            test(mc.shs.Count == 300);
            test(mc.fss.Length == 300);
            test(mc.ifsd.Count == 300);

            factory.setEnabled(true);
            os = new OutputStream(communicator);
            os.startEncapsulation();
            os.writeValue(mc);
            os.endEncapsulation();
            inEncaps = os.finished();
            test(initial.ice_invoke("pingPong", OperationMode.Normal, inEncaps, out outEncaps));
            @in = new InputStream(communicator, outEncaps);
            @in.startEncapsulation();
            @in.readValue(cb.invoke);
            @in.endEncapsulation();
            test(cb.obj is not null && cb.obj is TestValueReader);
            factory.setEnabled(false);

            output.WriteLine("ok");

            output.Write("testing tag marshaling... ");
            output.Flush();
            {
                var b = new Test.B();
                var b2 = (Test.B)initial.pingPong(b);
                test(b2.ma is null);
                test(b2.mb is null);
                test(b2.mc is null);

                b.ma = 10;
                b.mb = 11;
                b.mc = 12;
                b.md = 13;

                b2 = (Test.B)initial.pingPong(b);
                test(b2.ma.Value == 10);
                test(b2.mb.Value == 11);
                test(b2.mc.Value == 12);
                test(b2.md.Value == 13);

                factory.setEnabled(true);
                os = new OutputStream(communicator);
                os.startEncapsulation();
                os.writeValue(b);
                os.endEncapsulation();
                inEncaps = os.finished();
                test(initial.ice_invoke("pingPong", Ice.OperationMode.Normal, inEncaps, out outEncaps));
                @in = new InputStream(communicator, outEncaps);
                @in.startEncapsulation();
                @in.readValue(cb.invoke);
                @in.endEncapsulation();
                test(cb.obj is not null);
                factory.setEnabled(false);
            }
            output.WriteLine("ok");

            output.Write("testing marshaling of objects with optional members...");
            output.Flush();
            {
                var f = new Test.F();

                f.fsf = new Test.FixedStruct();
                f.fse = (Test.FixedStruct)f.fsf;

                var rf = (Test.F)initial.pingPong(f);
                test(rf.fse == rf.fsf.Value);

                factory.setEnabled(true);
                os = new OutputStream(communicator);
                os.startEncapsulation();
                os.writeValue(f);
                os.endEncapsulation();
                inEncaps = os.finished();
                @in = new InputStream(communicator, inEncaps);
                @in.startEncapsulation();
                var rocb = new ReadValueCallbackI();
                @in.readValue(rocb.invoke);
                @in.endEncapsulation();
                factory.setEnabled(false);
                rf = ((FValueReader)rocb.obj).getF();
                test(rf.fsf is null);
            }
            output.WriteLine("ok");

            output.Write("testing optional with default values... ");
            output.Flush();
            {
                var wd = (Test.WD)initial.pingPong(new Test.WD());
                test(wd.a.Value == 5);
                test(wd.s == "test");
                wd.a = null;
                wd.s = null;
                wd = (Test.WD)initial.pingPong(wd);
                test(wd.a is null);
                test(wd.s is null);
            }
            output.WriteLine("ok");

            if (communicator.getProperties().getIcePropertyAsInt("Ice.Default.SlicedFormat") > 0)
            {
                output.Write("testing marshaling with unknown class slices... ");
                output.Flush();
                {
                    var c = new Test.C();
                    c.ss = "test";
                    c.ms = "testms";
                    os = new Ice.OutputStream(communicator);
                    os.startEncapsulation();
                    os.writeValue(c);
                    os.endEncapsulation();
                    inEncaps = os.finished();
                    factory.setEnabled(true);
                    test(initial.ice_invoke("pingPong", Ice.OperationMode.Normal, inEncaps, out outEncaps));
                    @in = new Ice.InputStream(communicator, outEncaps);
                    @in.startEncapsulation();
                    @in.readValue(cb.invoke);
                    @in.endEncapsulation();
                    test(cb.obj is CValueReader);
                    factory.setEnabled(false);

                    factory.setEnabled(true);
                    os = new Ice.OutputStream(communicator);
                    os.startEncapsulation();
                    Ice.Value d = new DValueWriter();
                    os.writeValue(d);
                    os.endEncapsulation();
                    inEncaps = os.finished();
                    test(initial.ice_invoke("pingPong", Ice.OperationMode.Normal, inEncaps, out outEncaps));
                    @in = new Ice.InputStream(communicator, outEncaps);
                    @in.startEncapsulation();
                    @in.readValue(cb.invoke);
                    @in.endEncapsulation();
                    test(cb.obj is not null && cb.obj is DValueReader);
                    ((DValueReader)cb.obj).check();
                    factory.setEnabled(false);
                }
                output.WriteLine("ok");

                output.Write("testing operations with unknown optionals... ");
                output.Flush();
                {
                    var a = new Test.A();
                    var ovs = new Test.VarStruct("test");

                    os = new Ice.OutputStream(communicator);
                    os.startEncapsulation();
                    os.writeValue(a);
                    os.writeOptional(1, Ice.OptionalFormat.FSize);
                    int pos = os.startSize();
                    Test.VarStruct.ice_write(os, ovs);
                    os.endSize(pos);
                    os.endEncapsulation();
                    inEncaps = os.finished();
                    test(initial.ice_invoke("opClassAndUnknownOptional", Ice.OperationMode.Normal, inEncaps,
                                            out outEncaps));

                    @in = new Ice.InputStream(communicator, outEncaps);
                    @in.startEncapsulation();
                    @in.endEncapsulation();
                }
                output.WriteLine("ok");
            }

            output.Write("testing optional parameters... ");
            output.Flush();
            {
                byte? p1 = null;
                byte? p2 = initial.opByte(p1, out byte? p3);
                test(p2 is null && p3 is null);

                p1 = 56;
                p2 = initial.opByte(p1, out p3);
                test(p2.Value == 56 && p3.Value == 56);

                var result = await initial.opByteAsync(p1);
                test(result.returnValue.Value == 56 && result.p3.Value == 56);

                p2 = initial.opByte(null, out p3);
                test(p2 is null && p3 is null); // Ensure out parameter is cleared.

                os = new OutputStream(communicator);
                os.startEncapsulation();
                os.writeOptional(2, OptionalFormat.F1);
                os.writeByte(p1.Value);
                os.endEncapsulation();
                inEncaps = os.finished();
                initial.ice_invoke("opByte", OperationMode.Normal, inEncaps, out outEncaps);
                @in = new InputStream(communicator, outEncaps);
                @in.startEncapsulation();
                test(@in.readOptional(1, OptionalFormat.F1));
                test(@in.readByte() == 56);
                test(@in.readOptional(3, OptionalFormat.F1));
                test(@in.readByte() == 56);
                @in.endEncapsulation();

                @in = new InputStream(communicator, outEncaps);
                @in.startEncapsulation();
                @in.endEncapsulation();
            }

            {
                bool? p1 = null;
                bool? p2 = initial.opBool(p1, out bool? p3);
                test(p2 is null && p3 is null);

                p1 = true;
                p2 = initial.opBool(p1, out p3);
                test(p2.Value == true && p3.Value == true);

                var result = await initial.opBoolAsync(p1);
                test(result.returnValue.Value == true && result.p3.Value == true);

                p2 = initial.opBool(null, out p3);
                test(p2 is null && p3 is null); // Ensure out parameter is cleared.

                os = new OutputStream(communicator);
                os.startEncapsulation();
                os.writeOptional(2, OptionalFormat.F1);
                os.writeBool(p1.Value);
                os.endEncapsulation();
                inEncaps = os.finished();
                initial.ice_invoke("opBool", OperationMode.Normal, inEncaps, out outEncaps);
                @in = new InputStream(communicator, outEncaps);
                @in.startEncapsulation();
                test(@in.readOptional(1, OptionalFormat.F1));
                test(@in.readBool() == true);
                test(@in.readOptional(3, OptionalFormat.F1));
                test(@in.readBool() == true);
                @in.endEncapsulation();

                @in = new InputStream(communicator, outEncaps);
                @in.startEncapsulation();
                @in.endEncapsulation();
            }

            {
                short? p1 = null;
                short? p2 = initial.opShort(p1, out short? p3);
                test(p2 is null && p3 is null);

                p1 = 56;
                p2 = initial.opShort(p1, out p3);
                test(p2.Value == 56 && p3.Value == 56);

                var result = await initial.opShortAsync(p1);
                test(result.returnValue.Value == 56 && result.p3.Value == 56);

                p2 = initial.opShort(null, out p3);
                test(p2 is null && p3 is null); // Ensure out parameter is cleared.

                os = new OutputStream(communicator);
                os.startEncapsulation();
                os.writeOptional(2, OptionalFormat.F2);
                os.writeShort(p1.Value);
                os.endEncapsulation();
                inEncaps = os.finished();
                initial.ice_invoke("opShort", OperationMode.Normal, inEncaps, out outEncaps);
                @in = new InputStream(communicator, outEncaps);
                @in.startEncapsulation();
                test(@in.readOptional(1, OptionalFormat.F2));
                test(@in.readShort() == 56);
                test(@in.readOptional(3, OptionalFormat.F2));
                test(@in.readShort() == 56);
                @in.endEncapsulation();

                @in = new InputStream(communicator, outEncaps);
                @in.startEncapsulation();
                @in.endEncapsulation();
            }

            {
                int? p1 = null;
                int? p2 = initial.opInt(p1, out int? p3);
                test(p2 is null && p3 is null);

                p1 = 56;
                p2 = initial.opInt(p1, out p3);
                test(p2.Value == 56 && p3.Value == 56);

                var result = await initial.opIntAsync(p1);
                test(result.returnValue.Value == 56 && result.p3.Value == 56);

                p2 = initial.opInt(null, out p3);
                test(p2 is null && p3 is null); // Ensure out parameter is cleared.

                os = new OutputStream(communicator);
                os.startEncapsulation();
                os.writeOptional(2, OptionalFormat.F4);
                os.writeInt(p1.Value);
                os.endEncapsulation();
                inEncaps = os.finished();
                initial.ice_invoke("opInt", OperationMode.Normal, inEncaps, out outEncaps);
                @in = new InputStream(communicator, outEncaps);
                @in.startEncapsulation();
                test(@in.readOptional(1, OptionalFormat.F4));
                test(@in.readInt() == 56);
                test(@in.readOptional(3, OptionalFormat.F4));
                test(@in.readInt() == 56);
                @in.endEncapsulation();

                @in = new InputStream(communicator, outEncaps);
                @in.startEncapsulation();
                @in.endEncapsulation();
            }

            {
                long? p1 = null;
                long? p2 = initial.opLong(p1, out long? p3);
                test(p2 is null && p3 is null);

                p1 = 56;
                p2 = initial.opLong(p1, out p3);
                test(p2.Value == 56 && p3.Value == 56);

                var result = await initial.opLongAsync(p1);
                test(result.returnValue.Value == 56 && p3.Value == 56);

                p2 = initial.opLong(null, out p3);
                test(p2 is null && p3 is null); // Ensure out parameter is cleared.

                os = new OutputStream(communicator);
                os.startEncapsulation();
                os.writeOptional(1, OptionalFormat.F8);
                os.writeLong(p1.Value);
                os.endEncapsulation();
                inEncaps = os.finished();
                initial.ice_invoke("opLong", OperationMode.Normal, inEncaps, out outEncaps);
                @in = new InputStream(communicator, outEncaps);
                @in.startEncapsulation();
                test(@in.readOptional(2, OptionalFormat.F8));
                test(@in.readLong() == 56);
                test(@in.readOptional(3, OptionalFormat.F8));
                test(@in.readLong() == 56);
                @in.endEncapsulation();

                @in = new InputStream(communicator, outEncaps);
                @in.startEncapsulation();
                @in.endEncapsulation();
            }

            {
                float? p1 = null;
                float? p2 = initial.opFloat(p1, out float? p3);
                test(p2 is null && p3 is null);

                p1 = (float)1.0;
                p2 = initial.opFloat(p1, out p3);
                test(p2.Value == 1.0 && p3.Value == 1.0);

                var result = await initial.opFloatAsync(p1);
                test(result.returnValue.Value == 1.0 && result.p3.Value == 1.0);

                p2 = initial.opFloat(null, out p3);
                test(p2 is null && p3 is null); // Ensure out parameter is cleared.

                os = new OutputStream(communicator);
                os.startEncapsulation();
                os.writeOptional(2, OptionalFormat.F4);
                os.writeFloat(p1.Value);
                os.endEncapsulation();
                inEncaps = os.finished();
                initial.ice_invoke("opFloat", OperationMode.Normal, inEncaps, out outEncaps);
                @in = new InputStream(communicator, outEncaps);
                @in.startEncapsulation();
                test(@in.readOptional(1, OptionalFormat.F4));
                test(@in.readFloat() == 1.0);
                test(@in.readOptional(3, OptionalFormat.F4));
                test(@in.readFloat() == 1.0);
                @in.endEncapsulation();

                @in = new InputStream(communicator, outEncaps);
                @in.startEncapsulation();
                @in.endEncapsulation();
            }

            {
                double? p1 = null;
                double? p2 = initial.opDouble(p1, out double? p3);
                test(p2 is null && p3 is null);

                p1 = 1.0;
                p2 = initial.opDouble(p1, out p3);
                test(p2.Value == 1.0 && p3.Value == 1.0);
                var result = await initial.opDoubleAsync(p1);
                test(result.returnValue.Value == 1.0 && result.p3.Value == 1.0);

                p2 = initial.opDouble(null, out p3);
                test(p2 is null && p3 is null); // Ensure out parameter is cleared.

                os = new OutputStream(communicator);
                os.startEncapsulation();
                os.writeOptional(2, OptionalFormat.F8);
                os.writeDouble(p1.Value);
                os.endEncapsulation();
                inEncaps = os.finished();
                initial.ice_invoke("opDouble", OperationMode.Normal, inEncaps, out outEncaps);
                @in = new InputStream(communicator, outEncaps);
                @in.startEncapsulation();
                test(@in.readOptional(1, OptionalFormat.F8));
                test(@in.readDouble() == 1.0);
                test(@in.readOptional(3, OptionalFormat.F8));
                test(@in.readDouble() == 1.0);
                @in.endEncapsulation();

                @in = new InputStream(communicator, outEncaps);
                @in.startEncapsulation();
                @in.endEncapsulation();
            }

            {
                string p1 = null;

                string p2 = initial.opString(p1, out string p3);
                test(p2 is null && p3 is null);

                p1 = "test";
                p2 = initial.opString(p1, out p3);
                test(p2 == "test" && p3 == "test");

                var result = await initial.opStringAsync(p1);
                test(result.returnValue == "test" && result.p3 == "test");

                p2 = initial.opString(null, out p3);
                test(p2 is null && p3 is null); // Ensure out parameter is cleared.

                os = new OutputStream(communicator);
                os.startEncapsulation();
                os.writeOptional(2, OptionalFormat.VSize);
                os.writeString(p1);
                os.endEncapsulation();
                inEncaps = os.finished();
                initial.ice_invoke("opString", OperationMode.Normal, inEncaps, out outEncaps);
                @in = new InputStream(communicator, outEncaps);
                @in.startEncapsulation();
                test(@in.readOptional(1, OptionalFormat.VSize));
                test(@in.readString() == "test");
                test(@in.readOptional(3, OptionalFormat.VSize));
                test(@in.readString() == "test");
                @in.endEncapsulation();

                @in = new InputStream(communicator, outEncaps);
                @in.startEncapsulation();
                @in.endEncapsulation();
            }

            {
                Test.MyEnum? p1 = null;
                Test.MyEnum? p2 = initial.opMyEnum(p1, out Test.MyEnum? p3);
                test(p2 is null && p3 is null);

                p1 = Test.MyEnum.MyEnumMember;
                p2 = initial.opMyEnum(p1, out p3);
                test(p2.Value == Test.MyEnum.MyEnumMember && p3.Value == Test.MyEnum.MyEnumMember);

                var result = await initial.opMyEnumAsync(p1);
                test(result.returnValue.Value == Test.MyEnum.MyEnumMember &&
                     result.p3.Value == Test.MyEnum.MyEnumMember);
                p2 = initial.opMyEnum(p1.Value, out p3);

                p2 = initial.opMyEnum(null, out p3);
                test(p2 is null && p3 is null); // Ensure out parameter is cleared.

                os = new OutputStream(communicator);
                os.startEncapsulation();
                os.writeOptional(2, OptionalFormat.Size);
                os.writeEnum((int)p1.Value, 1);
                os.endEncapsulation();
                inEncaps = os.finished();
                initial.ice_invoke("opMyEnum", OperationMode.Normal, inEncaps, out outEncaps);
                @in = new InputStream(communicator, outEncaps);
                @in.startEncapsulation();
                test(@in.readOptional(1, OptionalFormat.Size));
                test((Test.MyEnum)@in.readEnum(1) == Test.MyEnum.MyEnumMember);
                test(@in.readOptional(3, OptionalFormat.Size));
                test((Test.MyEnum)@in.readEnum(1) == Test.MyEnum.MyEnumMember);
                @in.endEncapsulation();

                @in = new InputStream(communicator, outEncaps);
                @in.startEncapsulation();
                @in.endEncapsulation();
            }

            {
                Test.SmallStruct? p1 = null;
                Test.SmallStruct? p2 = initial.opSmallStruct(p1, out Test.SmallStruct? p3);
                test(p2 is null && p3 is null);

                p1 = new Test.SmallStruct(56);
                p2 = initial.opSmallStruct(p1, out p3);
                test(p2.Value.m == 56 && p3.Value.m == 56);

                var result = await initial.opSmallStructAsync(p1);
                test(result.returnValue.Value.m == 56 && result.p3.Value.m == 56);

                p2 = initial.opSmallStruct(null, out p3);
                test(p2 is null && p3 is null); // Ensure out parameter is cleared.

                os = new OutputStream(communicator);
                os.startEncapsulation();
                os.writeOptional(2, OptionalFormat.VSize);
                os.writeSize(1);
                Test.SmallStruct.ice_write(os, p1.Value);
                os.endEncapsulation();
                inEncaps = os.finished();
                initial.ice_invoke("opSmallStruct", OperationMode.Normal, inEncaps, out outEncaps);
                @in = new InputStream(communicator, outEncaps);
                @in.startEncapsulation();
                test(@in.readOptional(1, OptionalFormat.VSize));
                @in.skipSize();
                var f = new Test.SmallStruct(@in);
                test(f.m == (byte)56);
                test(@in.readOptional(3, OptionalFormat.VSize));
                @in.skipSize();
                f = new Test.SmallStruct(@in);
                test(f.m == (byte)56);
                @in.endEncapsulation();

                @in = new InputStream(communicator, outEncaps);
                @in.startEncapsulation();
                @in.endEncapsulation();
            }

            {
                Test.FixedStruct? p1 = null;
                Test.FixedStruct? p2 = initial.opFixedStruct(p1, out Test.FixedStruct? p3);
                test(p2 is null && p3 is null);

                p1 = new Test.FixedStruct(56);
                p2 = initial.opFixedStruct(p1, out p3);
                test(p2.Value.m == 56 && p3.Value.m == 56);

                var result = await initial.opFixedStructAsync(p1);
                test(result.returnValue.Value.m == 56 && result.p3.Value.m == 56);

                p2 = initial.opFixedStruct(null, out p3);
                test(p2 is null && p3 is null); // Ensure out parameter is cleared.

                os = new OutputStream(communicator);
                os.startEncapsulation();
                os.writeOptional(2, OptionalFormat.VSize);
                os.writeSize(4);
                Test.FixedStruct.ice_write(os, p1.Value);
                os.endEncapsulation();
                inEncaps = os.finished();
                initial.ice_invoke("opFixedStruct", OperationMode.Normal, inEncaps, out outEncaps);
                @in = new InputStream(communicator, outEncaps);
                @in.startEncapsulation();
                test(@in.readOptional(1, OptionalFormat.VSize));
                @in.skipSize();
                var f = new Test.FixedStruct(@in);
                test(f.m == 56);
                test(@in.readOptional(3, OptionalFormat.VSize));
                @in.skipSize();
                f = new Test.FixedStruct(@in);
                test(f.m == 56);
                @in.endEncapsulation();

                @in = new InputStream(communicator, outEncaps);
                @in.startEncapsulation();
                @in.endEncapsulation();
            }

            {
                Test.VarStruct p1 = null;
                Test.VarStruct p2 = initial.opVarStruct(p1, out Test.VarStruct p3);
                test(p2 is null && p3 is null);

                p1 = new Test.VarStruct("test");
                p2 = initial.opVarStruct(p1, out p3);
                test(p2.m == "test" && p3.m == "test");

                var result = await initial.opVarStructAsync(p1);
                test(result.returnValue.m == "test" && result.p3.m == "test");

                p2 = initial.opVarStruct(null, out p3);
                test(p2 is null && p3 is null); // Ensure out parameter is cleared.

                os = new OutputStream(communicator);
                os.startEncapsulation();
                os.writeOptional(2, OptionalFormat.FSize);
                int pos = os.startSize();
                Test.VarStruct.ice_write(os, p1);
                os.endSize(pos);
                os.endEncapsulation();
                inEncaps = os.finished();
                initial.ice_invoke("opVarStruct", OperationMode.Normal, inEncaps, out outEncaps);
                @in = new InputStream(communicator, outEncaps);
                @in.startEncapsulation();
                test(@in.readOptional(1, OptionalFormat.FSize));
                @in.skip(4);
                var v = new Test.VarStruct(@in);
                test(v.m == "test");
                test(@in.readOptional(3, OptionalFormat.FSize));
                @in.skip(4);
                v = new Test.VarStruct(@in);
                test(v.m == "test");
                @in.endEncapsulation();

                @in = new InputStream(communicator, outEncaps);
                @in.startEncapsulation();
                @in.endEncapsulation();
            }

            {
                Test.MyInterfacePrx p1 = null;
                Test.MyInterfacePrx p2 = initial.opMyInterfaceProxy(p1, out Test.MyInterfacePrx p3);
                test(p2 is null && p3 is null);
                p2 = initial.opMyInterfaceProxy(p1, out p3);
                test(p2 is null && p3 is null);

                p1 = Test.MyInterfacePrxHelper.uncheckedCast(communicator.stringToProxy("test"));
                p2 = initial.opMyInterfaceProxy(p1, out p3);
                test(p2.Equals(p1) && p3.Equals(p1));

                var result = await initial.opMyInterfaceProxyAsync(p1);
                test(result.returnValue.Equals(p1) && result.p3.Equals(p1));
                p2 = initial.opMyInterfaceProxy(p1, out p3);
                test(p2.Equals(p1) && p3.Equals(p1));
                result = await initial.opMyInterfaceProxyAsync(p1);
                test(result.returnValue.Equals(p1) && result.p3.Equals(p1));

                os = new Ice.OutputStream(communicator);
                os.startEncapsulation();
                os.writeOptional(2, Ice.OptionalFormat.FSize);
                int pos = os.startSize();
                os.writeProxy(p1);
                os.endSize(pos);
                os.endEncapsulation();
                inEncaps = os.finished();
                initial.ice_invoke("opMyInterfaceProxy", Ice.OperationMode.Normal, inEncaps, out outEncaps);
                @in = new Ice.InputStream(communicator, outEncaps);
                @in.startEncapsulation();
                test(@in.readOptional(1, Ice.OptionalFormat.FSize));
                @in.skip(4);
                test(@in.readProxy().Equals(p1));
                test(@in.readOptional(3, Ice.OptionalFormat.FSize));
                @in.skip(4);
                test(@in.readProxy().Equals(p1));
                @in.endEncapsulation();

                @in = new Ice.InputStream(communicator, outEncaps);
                @in.startEncapsulation();
                @in.endEncapsulation();
            }

            {
                var p1 = new Test.OneOptional();
                Test.OneOptional p2 = initial.opOneOptional(p1, out Test.OneOptional p3);
                test(!p2.a.HasValue && !p3.a.HasValue);

                p1 = new Test.OneOptional(58);
                p2 = initial.opOneOptional(p1, out p3);
                test(p2.a.Value == 58 && p3.a.Value == 58);

                var result = await initial.opOneOptionalAsync(p1);
                test(result.returnValue.a.Value == 58 && result.p3.a.Value == 58);

                p2 = initial.opOneOptional(new Test.OneOptional(), out p3);
                test(!p2.a.HasValue && !p3.a.HasValue); // Ensure out parameter is cleared.

                os = new OutputStream(communicator);
                os.startEncapsulation();
                os.writeValue(p1);
                os.endEncapsulation();
                inEncaps = os.finished();
                initial.ice_invoke("opOneOptional", OperationMode.Normal, inEncaps, out outEncaps);
                @in = new InputStream(communicator, outEncaps);
                @in.startEncapsulation();
                var p2cb = new ReadValueCallbackI();
                @in.readValue(p2cb.invoke);
                var p3cb = new ReadValueCallbackI();
                @in.readValue(p3cb.invoke);
                @in.endEncapsulation();
                test(((Test.OneOptional)p2cb.obj).a.Value == 58 && ((Test.OneOptional)p3cb.obj).a.Value == 58);
            }

            {
                byte[] p1 = null;
                byte[] p2 = initial.opByteSeq(p1, out byte[] p3);
                test(p2 is null && p3 is null);

                p1 = new byte[100];
                Populate(p1, (byte)56);
                p2 = initial.opByteSeq(p1, out p3);
                test(ArraysEqual(p2, p1) && ArraysEqual(p3, p1));

                var result = await initial.opByteSeqAsync(p1);
                test(ArraysEqual(result.returnValue, p1) && ArraysEqual(result.p3, p1));

                p2 = initial.opByteSeq(null, out p3);
                test(p2 is null && p3 is null); // Ensure out parameter is cleared.

                os = new OutputStream(communicator);
                os.startEncapsulation();
                os.writeOptional(2, OptionalFormat.VSize);
                os.writeByteSeq(p1);
                os.endEncapsulation();
                inEncaps = os.finished();
                initial.ice_invoke("opByteSeq", OperationMode.Normal, inEncaps, out outEncaps);
                @in = new InputStream(communicator, outEncaps);
                @in.startEncapsulation();
                test(@in.readOptional(1, OptionalFormat.VSize));
                test(ArraysEqual(@in.readByteSeq(), p1));
                test(@in.readOptional(3, OptionalFormat.VSize));
                test(ArraysEqual(@in.readByteSeq(), p1));
                @in.endEncapsulation();

                @in = new InputStream(communicator, outEncaps);
                @in.startEncapsulation();
                @in.endEncapsulation();
            }

            {
                bool[] p1 = null;
                bool[] p2 = initial.opBoolSeq(p1, out bool[] p3);
                test(p2 is null && p3 is null);

                p1 = new bool[100];
                Populate(p1, true);
                p2 = initial.opBoolSeq(p1, out p3);
                test(ArraysEqual(p2, p1) && ArraysEqual(p3, p1));

                var result = await initial.opBoolSeqAsync(p1);
                test(ArraysEqual(result.returnValue, p1) && ArraysEqual(result.p3, p1));

                p2 = initial.opBoolSeq(null, out p3);
                test(p2 is null && p3 is null); // Ensure out parameter is cleared.

                os = new OutputStream(communicator);
                os.startEncapsulation();
                os.writeOptional(2, OptionalFormat.VSize);
                os.writeBoolSeq(p1);
                os.endEncapsulation();
                inEncaps = os.finished();
                initial.ice_invoke("opBoolSeq", OperationMode.Normal, inEncaps, out outEncaps);
                @in = new InputStream(communicator, outEncaps);
                @in.startEncapsulation();
                test(@in.readOptional(1, OptionalFormat.VSize));
                test(ArraysEqual(@in.readBoolSeq(), p1));
                test(@in.readOptional(3, OptionalFormat.VSize));
                test(ArraysEqual(@in.readBoolSeq(), p1));
                @in.endEncapsulation();

                @in = new InputStream(communicator, outEncaps);
                @in.startEncapsulation();
                @in.endEncapsulation();
            }

            {
                List<short> p1 = null;
                List<short> p2 = initial.opShortSeq(p1, out List<short> p3);
                test(p2 is null && p3 is null);

                p1 = [.. Enumerable.Repeat((short)56, 300)];
                p2 = initial.opShortSeq(p1, out p3);
                test(p2.SequenceEqual(p1) && p3.SequenceEqual(p1));

                var result = await initial.opShortSeqAsync(p1);
                test(result.returnValue.SequenceEqual(p1) && result.p3.SequenceEqual(p1));

                p2 = initial.opShortSeq(null, out p3);
                test(p2 is null && p3 is null); // Ensure out parameter is cleared.

                os = new OutputStream(communicator);
                os.startEncapsulation();
                os.writeOptional(2, OptionalFormat.VSize);
                os.writeSize((p1.Count * 2) + (p1.Count > 254 ? 5 : 1));
                os.writeShortSeq([.. p1]);
                os.endEncapsulation();
                inEncaps = os.finished();
                initial.ice_invoke("opShortSeq", OperationMode.Normal, inEncaps, out outEncaps);
                @in = new InputStream(communicator, outEncaps);
                @in.startEncapsulation();
                test(@in.readOptional(1, OptionalFormat.VSize));
                @in.skipSize();
                test(@in.readShortSeq().SequenceEqual(p1));
                test(@in.readOptional(3, OptionalFormat.VSize));
                @in.skipSize();
                test(@in.readShortSeq().SequenceEqual(p1));
                @in.endEncapsulation();

                @in = new InputStream(communicator, outEncaps);
                @in.startEncapsulation();
                @in.endEncapsulation();
            }

            {
                int[] p1 = null;

                int[] p2 = initial.opIntSeq(p1, out int[] p3);
                test(p2 is null && p3 is null);

                p1 = new int[100];
                Populate(p1, 56);
                p2 = initial.opIntSeq(p1, out p3);
                test(ArraysEqual(p2, p1) && ArraysEqual(p3, p1));

                var result = await initial.opIntSeqAsync(p1);
                test(ArraysEqual(result.returnValue, p1) && ArraysEqual(result.p3, p1));

                p2 = initial.opIntSeq(null, out p3);
                test(p2 is null && p3 is null); // Ensure out parameter is cleared.

                os = new OutputStream(communicator);
                os.startEncapsulation();
                os.writeOptional(2, OptionalFormat.VSize);
                os.writeSize((p1.Length * 4) + (p1.Length > 254 ? 5 : 1));
                os.writeIntSeq(p1);
                os.endEncapsulation();
                inEncaps = os.finished();
                initial.ice_invoke("opIntSeq", OperationMode.Normal, inEncaps, out outEncaps);
                @in = new InputStream(communicator, outEncaps);
                @in.startEncapsulation();
                test(@in.readOptional(1, OptionalFormat.VSize));
                @in.skipSize();
                test(ArraysEqual(@in.readIntSeq(), p1));
                test(@in.readOptional(3, OptionalFormat.VSize));
                @in.skipSize();
                test(ArraysEqual(@in.readIntSeq(), p1));
                @in.endEncapsulation();

                @in = new InputStream(communicator, outEncaps);
                @in.startEncapsulation();
                @in.endEncapsulation();
            }

            {
                long[] p1 = null;
                long[] p2 = initial.opLongSeq(p1, out long[] p3);
                test(p2 is null && p3 is null);

                p1 = new long[100];
                Populate(p1, 56);
                p2 = initial.opLongSeq(p1, out p3);
                test(ArraysEqual(p2, p1) && ArraysEqual(p3, p1));

                var result = await initial.opLongSeqAsync(p1);
                test(ArraysEqual(result.returnValue, p1) && ArraysEqual(result.p3, p1));

                p2 = initial.opLongSeq(null, out p3);
                test(p2 is null && p3 is null); // Ensure out parameter is cleared.

                os = new OutputStream(communicator);
                os.startEncapsulation();
                os.writeOptional(2, OptionalFormat.VSize);
                os.writeSize((p1.Length * 8) + (p1.Length > 254 ? 5 : 1));
                os.writeLongSeq(p1);
                os.endEncapsulation();
                inEncaps = os.finished();
                initial.ice_invoke("opLongSeq", OperationMode.Normal, inEncaps, out outEncaps);
                @in = new InputStream(communicator, outEncaps);
                @in.startEncapsulation();
                test(@in.readOptional(1, OptionalFormat.VSize));
                @in.skipSize();
                test(ArraysEqual(@in.readLongSeq(), p1));
                test(@in.readOptional(3, OptionalFormat.VSize));
                @in.skipSize();
                test(ArraysEqual(@in.readLongSeq(), p1));
                @in.endEncapsulation();

                @in = new InputStream(communicator, outEncaps);
                @in.startEncapsulation();
                @in.endEncapsulation();
            }

            {
                float[] p1 = null;

                float[] p2 = initial.opFloatSeq(p1, out float[] p3);
                test(p2 is null && p3 is null);

                p1 = new float[100];
                Populate(p1, (float)1.0);
                p2 = initial.opFloatSeq(p1, out p3);
                test(ArraysEqual(p2, p1) && ArraysEqual(p3, p1));

                var result = await initial.opFloatSeqAsync(p1);
                test(ArraysEqual(result.returnValue, p1) && ArraysEqual(result.p3, p1));

                p2 = initial.opFloatSeq(null, out p3);
                test(p2 is null && p3 is null); // Ensure out parameter is cleared.

                os = new OutputStream(communicator);
                os.startEncapsulation();
                os.writeOptional(2, OptionalFormat.VSize);
                os.writeSize((p1.Length * 4) + (p1.Length > 254 ? 5 : 1));
                os.writeFloatSeq(p1);
                os.endEncapsulation();
                inEncaps = os.finished();
                initial.ice_invoke("opFloatSeq", OperationMode.Normal, inEncaps, out outEncaps);
                @in = new InputStream(communicator, outEncaps);
                @in.startEncapsulation();
                test(@in.readOptional(1, OptionalFormat.VSize));
                @in.skipSize();
                test(ArraysEqual(@in.readFloatSeq(), p1));
                test(@in.readOptional(3, OptionalFormat.VSize));
                @in.skipSize();
                test(ArraysEqual(@in.readFloatSeq(), p1));
                @in.endEncapsulation();

                @in = new InputStream(communicator, outEncaps);
                @in.startEncapsulation();
                @in.endEncapsulation();
            }

            {
                double[] p1 = null;
                double[] p2 = initial.opDoubleSeq(p1, out double[] p3);
                test(p2 is null && p3 is null);

                p1 = new double[100];
                Populate(p1, 1.0);
                p2 = initial.opDoubleSeq(p1, out p3);
                test(ArraysEqual(p2, p1) && ArraysEqual(p3, p1));

                var result = await initial.opDoubleSeqAsync(p1);
                test(ArraysEqual(result.returnValue, p1) && ArraysEqual(result.p3, p1));

                p2 = initial.opDoubleSeq(null, out p3);
                test(p2 is null && p3 is null); // Ensure out parameter is cleared.

                os = new OutputStream(communicator);
                os.startEncapsulation();
                os.writeOptional(2, OptionalFormat.VSize);
                os.writeSize((p1.Length * 8) + (p1.Length > 254 ? 5 : 1));
                os.writeDoubleSeq(p1);
                os.endEncapsulation();
                inEncaps = os.finished();
                initial.ice_invoke("opDoubleSeq", OperationMode.Normal, inEncaps, out outEncaps);
                @in = new InputStream(communicator, outEncaps);
                @in.startEncapsulation();
                test(@in.readOptional(1, OptionalFormat.VSize));
                @in.skipSize();
                test(ArraysEqual(@in.readDoubleSeq(), p1));
                test(@in.readOptional(3, OptionalFormat.VSize));
                @in.skipSize();
                test(ArraysEqual(@in.readDoubleSeq(), p1));
                @in.endEncapsulation();

                @in = new InputStream(communicator, outEncaps);
                @in.startEncapsulation();
                @in.endEncapsulation();
            }

            {
                string[] p1 = null;
                string[] p2 = initial.opStringSeq(p1, out string[] p3);
                test(p2 is null && p3 is null);

                p1 = new string[10];
                Populate(p1, "test1");
                p2 = initial.opStringSeq(p1, out p3);
                test(ArraysEqual(p2, p1) && ArraysEqual(p3, p1));

                var result = await initial.opStringSeqAsync(p1);
                test(ArraysEqual(result.returnValue, p1) && ArraysEqual(result.p3, p1));

                p2 = initial.opStringSeq(null, out p3);
                test(p2 is null && p3 is null); // Ensure out parameter is cleared.

                os = new OutputStream(communicator);
                os.startEncapsulation();
                os.writeOptional(2, OptionalFormat.FSize);
                int pos = os.startSize();
                os.writeStringSeq(p1);
                os.endSize(pos);
                os.endEncapsulation();
                inEncaps = os.finished();
                initial.ice_invoke("opStringSeq", OperationMode.Normal, inEncaps, out outEncaps);
                @in = new InputStream(communicator, outEncaps);
                @in.startEncapsulation();
                test(@in.readOptional(1, OptionalFormat.FSize));
                @in.skip(4);
                test(ArraysEqual(@in.readStringSeq(), p1));
                test(@in.readOptional(3, OptionalFormat.FSize));
                @in.skip(4);
                test(ArraysEqual(@in.readStringSeq(), p1));
                @in.endEncapsulation();

                @in = new InputStream(communicator, outEncaps);
                @in.startEncapsulation();
                @in.endEncapsulation();
            }

            {
                Test.SmallStruct[] p1 = null;
                Test.SmallStruct[] p2 = initial.opSmallStructSeq(p1, out Test.SmallStruct[] p3);
                test(p2 is null && p3 is null);

                p1 = new Test.SmallStruct[10];
                for (int i = 0; i < p1.Length; ++i)
                {
                    p1[i] = new Test.SmallStruct();
                }
                p2 = initial.opSmallStructSeq(p1, out p3);
                test(ArraysEqual(p2, p1) && ArraysEqual(p3, p1));

                var result = await initial.opSmallStructSeqAsync(p1);
                test(ArraysEqual(result.returnValue, p1) && ArraysEqual(result.p3, p1));

                p2 = initial.opSmallStructSeq(null, out p3);
                test(p2 is null && p3 is null); // Ensure out parameter is cleared.

                os = new OutputStream(communicator);
                os.startEncapsulation();
                os.writeOptional(2, OptionalFormat.VSize);
                Test.SmallStructSeqHelper.write(os, p1);
                os.endEncapsulation();
                inEncaps = os.finished();
                initial.ice_invoke("opSmallStructSeq", OperationMode.Normal, inEncaps, out outEncaps);
                @in = new InputStream(communicator, outEncaps);
                @in.startEncapsulation();
                test(@in.readOptional(1, OptionalFormat.VSize));
                Test.SmallStruct[] arr = Test.SmallStructSeqHelper.read(@in);
                test(ArraysEqual(arr, p1));
                test(@in.readOptional(3, OptionalFormat.VSize));
                arr = Test.SmallStructSeqHelper.read(@in);
                test(ArraysEqual(arr, p1));
                @in.endEncapsulation();

                // Check the outEncaps size matches the expected size, 6 bytes for the encapsulation, plus each
                // 12 bytes for each sequence ( 1 byte tag, 1 byte size, 10 byte contents)
                test(outEncaps.Length == 12 + 12 + 6);

                @in = new InputStream(communicator, outEncaps);
                @in.startEncapsulation();
                @in.endEncapsulation();
            }

            {
                List<Test.SmallStruct> p1 = null;
                List<Test.SmallStruct> p2 = initial.opSmallStructList(p1, out List<Test.SmallStruct> p3);
                test(p2 is null && p3 is null);

                p1 = [];
                for (int i = 0; i < 10; ++i)
                {
                    p1.Add(new Test.SmallStruct());
                }
                p2 = initial.opSmallStructList(p1, out p3);
                test(ListsEqual(p2, p1));

                var result = await initial.opSmallStructListAsync(p1);
                test(ListsEqual(result.returnValue, p1) && ListsEqual(result.p3, p1));

                p2 = initial.opSmallStructList(null, out p3);
                test(p2 is null && p3 is null); // Ensure out parameter is cleared.

                os = new OutputStream(communicator);
                os.startEncapsulation();
                os.writeOptional(2, OptionalFormat.VSize);
                os.writeSize(p1.Count + (p1.Count > 254 ? 5 : 1));
                Test.SmallStructListHelper.write(os, p1);
                os.endEncapsulation();
                inEncaps = os.finished();
                initial.ice_invoke("opSmallStructList", OperationMode.Normal, inEncaps, out outEncaps);
                @in = new InputStream(communicator, outEncaps);
                @in.startEncapsulation();
                test(@in.readOptional(1, OptionalFormat.VSize));
                @in.skipSize();
                List<Test.SmallStruct> arr = Test.SmallStructListHelper.read(@in);
                test(ListsEqual(arr, p1));
                test(@in.readOptional(3, OptionalFormat.VSize));
                @in.skipSize();
                arr = Test.SmallStructListHelper.read(@in);
                test(ListsEqual(arr, p1));
                @in.endEncapsulation();

                @in = new InputStream(communicator, outEncaps);
                @in.startEncapsulation();
                @in.endEncapsulation();
            }

            {
                Test.FixedStruct[] p1 = null;
                Test.FixedStruct[] p2 = initial.opFixedStructSeq(p1, out Test.FixedStruct[] p3);
                test(p2 is null && p3 is null);

                p1 = new Test.FixedStruct[10];
                for (int i = 0; i < p1.Length; ++i)
                {
                    p1[i] = new Test.FixedStruct();
                }
                p2 = initial.opFixedStructSeq(p1, out p3);
                test(ArraysEqual(p2, p1) && ArraysEqual(p3, p1));

                var result = await initial.opFixedStructSeqAsync(p1);
                test(ArraysEqual(result.returnValue, p1) && ArraysEqual(result.p3, p1));

                p2 = initial.opFixedStructSeq(null, out p3);
                test(p2 is null && p3 is null); // Ensure out parameter is cleared.

                os = new OutputStream(communicator);
                os.startEncapsulation();
                os.writeOptional(2, OptionalFormat.VSize);
                os.writeSize((p1.Length * 4) + (p1.Length > 254 ? 5 : 1));
                Test.FixedStructSeqHelper.write(os, p1);
                os.endEncapsulation();
                inEncaps = os.finished();
                initial.ice_invoke("opFixedStructSeq", OperationMode.Normal, inEncaps, out outEncaps);
                @in = new InputStream(communicator, outEncaps);
                @in.startEncapsulation();
                test(@in.readOptional(1, OptionalFormat.VSize));
                @in.skipSize();
                Test.FixedStruct[] arr = Test.FixedStructSeqHelper.read(@in);
                test(ArraysEqual(arr, p1));
                test(@in.readOptional(3, OptionalFormat.VSize));
                @in.skipSize();
                arr = Test.FixedStructSeqHelper.read(@in);
                test(ArraysEqual(arr, p1));
                @in.endEncapsulation();

                @in = new InputStream(communicator, outEncaps);
                @in.startEncapsulation();
                @in.endEncapsulation();
            }

            {
                LinkedList<Test.FixedStruct> p1 = null;
                LinkedList<Test.FixedStruct> p2 = initial.opFixedStructList(p1, out LinkedList<Test.FixedStruct> p3);
                test(p2 is null && p3 is null);

                p1 = new LinkedList<Test.FixedStruct>();
                for (int i = 0; i < 10; ++i)
                {
                    p1.AddLast(new Test.FixedStruct());
                }
                p2 = initial.opFixedStructList(p1, out p3);
                test(ListsEqual(p2, p1) && ListsEqual(p3, p1));

                var result = await initial.opFixedStructListAsync(p1);
                test(ListsEqual(result.returnValue, p1) && ListsEqual(result.p3, p1));

                p2 = initial.opFixedStructList(null, out p3);
                test(p2 is null && p3 is null); // Ensure out parameter is cleared.

                os = new OutputStream(communicator);
                os.startEncapsulation();
                os.writeOptional(2, OptionalFormat.VSize);
                os.writeSize((p1.Count * 4) + (p1.Count > 254 ? 5 : 1));
                Test.FixedStructListHelper.write(os, p1);
                os.endEncapsulation();
                inEncaps = os.finished();
                initial.ice_invoke("opFixedStructList", OperationMode.Normal, inEncaps, out outEncaps);
                @in = new InputStream(communicator, outEncaps);
                @in.startEncapsulation();
                test(@in.readOptional(1, OptionalFormat.VSize));
                @in.skipSize();
                LinkedList<Test.FixedStruct> arr = Test.FixedStructListHelper.read(@in);
                test(ListsEqual(arr, p1));
                test(@in.readOptional(3, OptionalFormat.VSize));
                @in.skipSize();
                arr = Test.FixedStructListHelper.read(@in);
                test(ListsEqual(arr, p1));
                @in.endEncapsulation();

                @in = new InputStream(communicator, outEncaps);
                @in.startEncapsulation();
                @in.endEncapsulation();
            }

            {
                Test.VarStruct[] p1 = null;
                Test.VarStruct[] p2 = initial.opVarStructSeq(p1, out Test.VarStruct[] p3);
                test(p2 is null && p3 is null);

                p1 = new Test.VarStruct[10];
                for (int i = 0; i < p1.Length; ++i)
                {
                    p1[i] = new Test.VarStruct("");
                }
                p2 = initial.opVarStructSeq(p1, out p3);
                test(ArraysEqual(p2, p1) && ArraysEqual(p3, p1));

                var result = await initial.opVarStructSeqAsync(p1);
                test(ArraysEqual(result.returnValue, p1) && ArraysEqual(result.p3, p1));

                p2 = initial.opVarStructSeq(null, out p3);
                test(p2 is null && p3 is null); // Ensure out parameter is cleared.

                os = new OutputStream(communicator);
                os.startEncapsulation();
                os.writeOptional(2, OptionalFormat.FSize);
                int pos = os.startSize();
                Test.VarStructSeqHelper.write(os, p1);
                os.endSize(pos);
                os.endEncapsulation();
                inEncaps = os.finished();
                initial.ice_invoke("opVarStructSeq", OperationMode.Normal, inEncaps, out outEncaps);
                @in = new InputStream(communicator, outEncaps);
                @in.startEncapsulation();
                test(@in.readOptional(1, OptionalFormat.FSize));
                @in.skip(4);
                Test.VarStruct[] arr = Test.VarStructSeqHelper.read(@in);
                test(ArraysEqual(arr, p1));
                test(@in.readOptional(3, OptionalFormat.FSize));
                @in.skip(4);
                arr = Test.VarStructSeqHelper.read(@in);
                test(ArraysEqual(arr, p1));
                @in.endEncapsulation();

                @in = new InputStream(communicator, outEncaps);
                @in.startEncapsulation();
                @in.endEncapsulation();
            }

            {
                Dictionary<int, int> p1 = null;
                Dictionary<int, int> p2 = initial.opIntIntDict(p1, out Dictionary<int, int> p3);
                test(p2 is null && p3 is null);

                p1 = new Dictionary<int, int>
                    {
                        { 1, 2 },
                        { 2, 3 }
                    };
                p2 = initial.opIntIntDict(p1, out p3);
                test(MapsEqual(p2, p1) && MapsEqual(p3, p1));

                var result = await initial.opIntIntDictAsync(p1);
                test(MapsEqual(result.returnValue, p1) && MapsEqual(p3, p1));

                p2 = initial.opIntIntDict(null, out p3);
                test(p2 is null && p3 is null); // Ensure out parameter is cleared.

                os = new OutputStream(communicator);
                os.startEncapsulation();
                os.writeOptional(2, OptionalFormat.VSize);
                os.writeSize((p1.Count * 8) + (p1.Count > 254 ? 5 : 1));
                Test.IntIntDictHelper.write(os, p1);
                os.endEncapsulation();
                inEncaps = os.finished();
                initial.ice_invoke("opIntIntDict", OperationMode.Normal, inEncaps, out outEncaps);
                @in = new InputStream(communicator, outEncaps);
                @in.startEncapsulation();
                test(@in.readOptional(1, OptionalFormat.VSize));
                @in.skipSize();
                Dictionary<int, int> m = Test.IntIntDictHelper.read(@in);
                test(MapsEqual(m, p1));
                test(@in.readOptional(3, OptionalFormat.VSize));
                @in.skipSize();
                m = Test.IntIntDictHelper.read(@in);
                test(MapsEqual(m, p1));
                @in.endEncapsulation();

                @in = new InputStream(communicator, outEncaps);
                @in.startEncapsulation();
                @in.endEncapsulation();
            }

            {
                Dictionary<string, int> p1 = null;
                Dictionary<string, int> p2 = initial.opStringIntDict(p1, out Dictionary<string, int> p3);
                test(p2 is null && p3 is null);

                p1 = new Dictionary<string, int>
                    {
                        { "1", 1 },
                        { "2", 2 }
                    };
                p2 = initial.opStringIntDict(p1, out p3);
                test(MapsEqual(p2, p1) && MapsEqual(p3, p1));

                var result = await initial.opStringIntDictAsync(p1);
                test(MapsEqual(result.returnValue, p1) && MapsEqual(result.p3, p1));

                p2 = initial.opStringIntDict(null, out p3);
                test(p2 is null && p3 is null); // Ensure out parameter is cleared.

                os = new OutputStream(communicator);
                os.startEncapsulation();
                os.writeOptional(2, OptionalFormat.FSize);
                int pos = os.startSize();
                Test.StringIntDictHelper.write(os, p1);
                os.endSize(pos);
                os.endEncapsulation();
                inEncaps = os.finished();
                initial.ice_invoke("opStringIntDict", OperationMode.Normal, inEncaps, out outEncaps);
                @in = new InputStream(communicator, outEncaps);
                @in.startEncapsulation();
                test(@in.readOptional(1, OptionalFormat.FSize));
                @in.skip(4);
                Dictionary<string, int> m = Test.StringIntDictHelper.read(@in);
                test(MapsEqual(m, p1));
                test(@in.readOptional(3, OptionalFormat.FSize));
                @in.skip(4);
                m = Test.StringIntDictHelper.read(@in);
                test(MapsEqual(m, p1));
                @in.endEncapsulation();

                @in = new InputStream(communicator, outEncaps);
                @in.startEncapsulation();
                @in.endEncapsulation();

                var f = new Test.F();
                f.fsf = new Test.FixedStruct(56);
                f.fse = f.fsf.Value;

                os = new OutputStream(communicator);
                os.startEncapsulation();
                os.writeOptional(2, OptionalFormat.VSize);
                os.writeSize(4);
                Test.FixedStruct.ice_write(os, f.fse);
                os.endEncapsulation();
                inEncaps = os.finished();

                @in = new InputStream(communicator, inEncaps);
                @in.startEncapsulation();
                test(@in.readOptional(2, OptionalFormat.VSize));
                @in.skipSize();
                var fs1 = Test.FixedStruct.ice_read(@in);
                @in.endEncapsulation();
                test(fs1.m == 56);
            }
            output.WriteLine("ok");

            output.Write("testing exception optionals... ");
            output.Flush();
            {
                try
                {
                    int? a = null;
                    string b = null;
                    initial.opOptionalException(a, b);
                }
                catch (Test.OptionalException ex)
                {
                    test(ex.a is null);
                    test(ex.b is null);
                }

                try
                {
                    int? a = 30;
                    string b = "test";
                    initial.opOptionalException(a, b);
                }
                catch (Test.OptionalException ex)
                {
                    test(ex.a.Value == 30);
                    test(ex.b == "test");
                }

                try
                {
                    //
                    // Use the 1.0 encoding with an exception whose only data members are optional.
                    //
                    var initial2 = (Test.InitialPrx)initial.ice_encodingVersion(Ice.Util.Encoding_1_0);
                    int? a = 30;
                    string b = "test";
                    initial2.opOptionalException(a, b);
                }
                catch (Test.OptionalException ex)
                {
                    test(ex.a is null);
                    test(ex.b is null);
                }

                try
                {
                    int? a = null;
                    string b = null;
                    initial.opDerivedException(a, b);
                }
                catch (Test.DerivedException ex)
                {
                    test(ex.a is null);
                    test(ex.b is null);
                    test(ex.ss is null);
                    test(ex.d1 == "d1");
                    test(ex.d2 == "d2");
                }

                try
                {
                    int? a = 30;
                    string b = "test2";
                    initial.opDerivedException(a, b);
                }
                catch (Test.DerivedException ex)
                {
                    test(ex.a.Value == 30);
                    test(ex.b == "test2");
                    test(ex.ss == "test2");
                    test(ex.d1 == "d1");
                    test(ex.d2 == "d2");
                }

                try
                {
                    int? a = null;
                    string b = null;
                    initial.opRequiredException(a, b);
                }
                catch (Test.RequiredException ex)
                {
                    test(ex.a is null);
                    test(ex.b is null);
                    test(ex.ss == "test");
                }

                try
                {
                    int? a = 30;
                    string b = "test2";
                    initial.opRequiredException(a, b);
                }
                catch (Test.RequiredException ex)
                {
                    test(ex.a.Value == 30);
                    test(ex.b == "test2");
                    test(ex.ss == "test2");
                }
            }
            output.WriteLine("ok");

            output.Write("testing optionals with marshaled results... ");
            output.Flush();
            {
                test(initial.opMStruct1() is not null);
                test(initial.opMDict1() is not null);
                test(initial.opMSeq1() is not null);

                {
                    Test.SmallStruct? p1, p3;
                    p3 = initial.opMStruct2(null, out Test.SmallStruct? p2);
                    test(p2 is null && p3 is null);

                    p1 = new Test.SmallStruct();
                    p3 = initial.opMStruct2(p1, out p2);
                    test(p2.Value.Equals(p1.Value) && p3.Value.Equals(p1.Value));
                }
                {
                    string[] p1, p3;
                    p3 = initial.opMSeq2(null, out string[] p2);
                    test(p2 is null && p3 is null);

                    p1 = ["hello"];
                    p3 = initial.opMSeq2(p1, out p2);
                    test(Enumerable.SequenceEqual(p2, p1) &&
                         Enumerable.SequenceEqual(p3, p1));
                }
                {
                    Dictionary<string, int> p1, p3;
                    p3 = initial.opMDict2(null, out Dictionary<string, int> p2);
                    test(p2 is null && p3 is null);

                    p1 = new Dictionary<string, int>
                    {
                        ["test"] = 54
                    };
                    p3 = initial.opMDict2(p1, out p2);
                    test(Internal.DictionaryExtensions.DictionaryEqual(p2, p1) &&
                         Internal.DictionaryExtensions.DictionaryEqual(p3, p1));
                }
            }
            output.WriteLine("ok");

            return initial;
        }

        internal static bool ArraysEqual<T>(T[] a1, T[] a2)
        {
            if (ReferenceEquals(a1, a2))
            {
                return true;
            }

            if (a1 is null || a2 is null)
            {
                return false;
            }

            if (a1.Length != a2.Length)
            {
                return false;
            }

            EqualityComparer<T> comparer = EqualityComparer<T>.Default;
            for (int i = 0; i < a1.Length; ++i)
            {
                if (!comparer.Equals(a1[i], a2[i]))
                {
                    return false;
                }
            }

            return true;
        }

        internal static bool ListsEqual<T>(ICollection<T> a1, ICollection<T> a2)
        {
            if (ReferenceEquals(a1, a2))
            {
                return true;
            }

            if (a1 is null || a2 is null)
            {
                return false;
            }

            if (a1.Count != a2.Count)
            {
                return false;
            }

            EqualityComparer<T> comparer = EqualityComparer<T>.Default;
            IEnumerator<T> a1i = a1.GetEnumerator();
            IEnumerator<T> a2i = a2.GetEnumerator();
            while (a1i.MoveNext() && a2i.MoveNext())
            {
                if (!comparer.Equals(a1i.Current, a2i.Current))
                {
                    return false;
                }
            }

            return true;
        }

        internal static bool MapsEqual<K, V>(Dictionary<K, V> d1, Dictionary<K, V> d2)
        {
            if (ReferenceEquals(d1, d2))
            {
                return true;
            }

            if (d1 is null || d2 is null)
            {
                return false;
            }

            if (d1.Count != d2.Count)
            {
                return false;
            }

            EqualityComparer<V> valueComparer = EqualityComparer<V>.Default;
            foreach (K key in d1.Keys)
            {
                if (!d2.ContainsKey(key) || !valueComparer.Equals(d1[key], d2[key]))
                {
                    return false;
                }
            }

            return true;
        }

        internal static void Populate<T>(T[] arr, T value)
        {
            for (int i = 0; i < arr.Length; ++i)
            {
                arr[i] = value;
            }
        }

        private class TestValueReader : Value
        {
            public override void iceRead(InputStream @in)
            {
                @in.startValue();
                @in.startSlice();
                @in.endSlice();
                @in.endValue();
            }

            public override void iceWrite(Ice.OutputStream outS) => Debug.Assert(false);
        }

        private class BValueReader : Ice.Value
        {
            public override void iceRead(Ice.InputStream @in)
            {
                @in.startValue();
                // ::Test::B
                @in.startSlice();
                @in.readInt();
                @in.endSlice();
                // ::Test::A
                @in.startSlice();
                @in.readInt();
                @in.endSlice();
                @in.endValue();
            }

            public override void iceWrite(Ice.OutputStream outS) => Debug.Assert(false);
        }

        private class CValueReader : Ice.Value
        {
            public override void iceRead(Ice.InputStream @in)
            {
                @in.startValue();
                // ::Test::C
                @in.startSlice();
                @in.skipSlice();
                // ::Test::B
                @in.startSlice();
                @in.readInt();
                @in.endSlice();
                // ::Test::A
                @in.startSlice();
                @in.readInt();
                @in.endSlice();
                @in.endValue();
            }

            public override void iceWrite(Ice.OutputStream outS) => Debug.Assert(false);
        }

        private class DValueWriter : Value
        {
            public override void iceRead(Ice.InputStream @in) => Debug.Assert(false);

            public override void iceWrite(Ice.OutputStream @out)
            {
                @out.startValue(null);
                // ::Test::D
                @out.startSlice("::Test::D", -1, false);
                string s = "test";
                @out.writeString(s);
                @out.writeOptional(1, Ice.OptionalFormat.FSize);
                string[] o = ["test1", "test2", "test3", "test4"];
                int pos = @out.startSize();
                @out.writeStringSeq(o);
                @out.endSize(pos);
                var a = new Test.A();
                a.mc = 18;
                @out.writeValue(a);
                @out.endSlice();
                // ::Test::B
                @out.startSlice(Test.B.ice_staticId(), -1, false);
                int v = 14;
                @out.writeInt(v);
                @out.endSlice();
                // ::Test::A
                @out.startSlice(Test.A.ice_staticId(), -1, true);
                @out.writeInt(v);
                @out.endSlice();
                @out.endValue();
            }
        }

        private class DValueReader : Ice.Value
        {
            public override void iceRead(Ice.InputStream @in)
            {
                @in.startValue();
                // ::Test::D
                @in.startSlice();
                string s = @in.readString();
                test(s == "test");
                test(@in.readOptional(1, Ice.OptionalFormat.FSize));
                @in.skip(4);
                string[] o = @in.readStringSeq();
                test(o.Length == 4 &&
                     o[0] == "test1" && o[1] == "test2" && o[2] == "test3" && o[3] == "test4");
                @in.readValue(_a.invoke);
                @in.endSlice();
                // ::Test::B
                @in.startSlice();
                @in.readInt();
                @in.endSlice();
                // ::Test::A
                @in.startSlice();
                @in.readInt();
                @in.endSlice();
                @in.endValue();
            }

            public override void iceWrite(Ice.OutputStream @out) => Debug.Assert(false);

            internal void check() => test(((Test.A)_a.obj).mc.Value == 18);

            private readonly ReadValueCallbackI _a = new();
        }

        private class FValueReader : Ice.Value
        {
            public override void iceRead(InputStream @in)
            {
                _f = new Test.F();
                @in.startValue();
                @in.startSlice();
                // Don't read fsf on purpose
                // @in.read(1, _f.fsf);
                @in.endSlice();
                @in.startSlice();
                _f.fse = Test.FixedStruct.ice_read(@in);
                @in.endSlice();
                @in.endValue();
            }

            public override void iceWrite(Ice.OutputStream @out) => Debug.Assert(false);

            public Test.F getF() => _f;

            private Test.F _f;
        }

        private class FactoryI
        {
            public Ice.Value create(string typeId)
            {
                if (!_enabled)
                {
                    return null;
                }

                if (typeId.Equals(Test.OneOptional.ice_staticId()))
                {
                    return new TestValueReader();
                }
                else if (typeId.Equals(Test.MultiOptional.ice_staticId()))
                {
                    return new TestValueReader();
                }
                else if (typeId.Equals(Test.B.ice_staticId()))
                {
                    return new BValueReader();
                }
                else if (typeId.Equals(Test.C.ice_staticId()))
                {
                    return new CValueReader();
                }
                else if (typeId == "::Test::D")
                {
                    return new DValueReader();
                }
                else if (typeId == "::Test::F")
                {
                    return new FValueReader();
                }

                return null;
            }

            internal void setEnabled(bool enabled) => _enabled = enabled;

            private bool _enabled;
        }

        private class ReadValueCallbackI
        {
            public void invoke(Ice.Value obj) => this.obj = obj;

            internal Ice.Value obj;
        }
    }
}

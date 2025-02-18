// Copyright (c) ZeroC, Inc.

import { Ice } from "@zeroc/ice";
import { Test } from "./Test.js";
import { TestHelper } from "../../Common/TestHelper.js";

const test = TestHelper.test;

import { Test as ClientPrivate } from "./ClientPrivate.js";

const ArrayUtil = Ice.ArrayUtil;

export class Client extends TestHelper {
    async allTests() {
        const communicator = this.communicator();
        const out = this.getWriter();
        out.write("testing stringToProxy... ");
        const ref = "initial:" + this.getTestEndpoint();
        const base = communicator.stringToProxy(ref);
        test(base !== null);
        out.writeLine("ok");

        let oo1 = new Test.OneOptional();
        oo1.a = 15;

        out.write("testing checked cast... ");
        const initial = await Test.InitialPrx.checkedCast(base);
        test(initial !== null);
        test(initial.equals(base));
        out.writeLine("ok");

        out.write("testing marshaling... ");

        const oo4 = (await initial.pingPong(new Test.OneOptional())) as Test.OneOptional;
        test(oo4.a === undefined);

        const oo5 = (await initial.pingPong(oo1)) as Test.OneOptional;
        test(oo5.a === oo1.a);

        const mo4 = (await initial.pingPong(new Test.MultiOptional())) as Test.MultiOptional;
        test(mo4.a === undefined);
        test(mo4.b === undefined);
        test(mo4.c === undefined);
        test(mo4.d === undefined);
        test(mo4.e === undefined);
        test(mo4.f === undefined);
        test(mo4.g === undefined);
        test(mo4.h === undefined);
        test(mo4.i === undefined);
        test(mo4.j === undefined);
        test(mo4.bs === undefined);
        test(mo4.ss === undefined);
        test(mo4.iid === undefined);
        test(mo4.sid === undefined);
        test(mo4.fs === undefined);
        test(mo4.vs === undefined);

        test(mo4.shs === undefined);
        test(mo4.es === undefined);
        test(mo4.fss === undefined);
        test(mo4.vss === undefined);
        test(mo4.mips === undefined);

        test(mo4.ied === undefined);
        test(mo4.ifsd === undefined);
        test(mo4.ivsd === undefined);
        test(mo4.imipd === undefined);

        test(mo4.bos === undefined);

        test(mo4.ser === undefined);

        const mo1 = new Test.MultiOptional();
        mo1.a = 15;
        mo1.b = true;
        mo1.c = 19;
        mo1.d = 78;
        mo1.e = new Ice.Long(0, 99);
        mo1.f = 5.5;
        mo1.g = 1.0;
        mo1.h = "test";
        mo1.i = Test.MyEnum.MyEnumMember;
        mo1.j = new Test.MyInterfacePrx(communicator, "test");
        mo1.bs = new Uint8Array([5]);
        mo1.ss = ["test", "test2"];
        mo1.iid = new Map();
        mo1.iid.set(4, 3);
        mo1.sid = new Map();
        mo1.sid.set("test", 10);
        mo1.fs = new Test.FixedStruct();
        mo1.fs.m = 78;
        mo1.vs = new Test.VarStruct();
        mo1.vs.m = "hello";

        mo1.shs = [1];
        mo1.es = [Test.MyEnum.MyEnumMember, Test.MyEnum.MyEnumMember];
        mo1.fss = [mo1.fs];
        mo1.vss = [mo1.vs];
        mo1.mips = [new Test.MyInterfacePrx(communicator, "test")];

        mo1.ied = new Map();
        mo1.ied.set(4, Test.MyEnum.MyEnumMember);
        mo1.ifsd = new Map();
        mo1.ifsd.set(4, mo1.fs);
        mo1.ivsd = new Map();
        mo1.ivsd.set(5, mo1.vs);
        mo1.imipd = new Map();
        mo1.imipd.set(5, new Test.MyInterfacePrx(communicator, "test"));

        mo1.bos = [false, true, false];

        const mo5 = (await initial.pingPong(mo1)) as Test.MultiOptional;

        test(mo1.a == mo5.a);
        test(mo1.b == mo5.b);
        test(mo1.c == mo5.c);
        test(mo1.d == mo5.d);
        test(mo1.e.equals(mo5.e));
        test(mo1.f == mo5.f);
        test(mo1.g == mo5.g);
        test(mo1.h == mo5.h);
        test(mo1.i == mo5.i);
        test(mo1.j.equals(mo5.j));
        test(ArrayUtil.equals(mo5.bs, mo1.bs));
        test(ArrayUtil.equals(mo5.ss, mo1.ss));
        test(mo5.iid.get(4) == 3);
        test(mo5.sid.get("test") == 10);
        test(mo5.fs.equals(mo1.fs));
        test(mo5.vs.equals(mo1.vs));
        test(ArrayUtil.equals(mo5.shs, mo1.shs));
        test(mo5.es[0] == Test.MyEnum.MyEnumMember && mo5.es[1] == Test.MyEnum.MyEnumMember);
        test(mo5.fss[0].equals(new Test.FixedStruct(78)));
        test(mo5.vss[0].equals(new Test.VarStruct("hello")));
        test(mo5.mips[0].equals(communicator.stringToProxy("test")));

        test(mo5.ied.get(4) == Test.MyEnum.MyEnumMember);
        test(mo5.ifsd.get(4).equals(new Test.FixedStruct(78)));
        test(mo5.ivsd.get(5).equals(new Test.VarStruct("hello")));
        test(mo5.imipd.get(5).equals(communicator.stringToProxy("test")));

        test(ArrayUtil.equals(mo5.bos, [false, true, false]));

        // Clear the first half of the optional parameters
        const mo6 = new Test.MultiOptional();
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

        const mo7 = (await initial.pingPong(mo6)) as Test.MultiOptional;
        test(mo7.a === undefined);
        test(mo7.b == mo1.b);
        test(mo7.c === undefined);
        test(mo7.d == mo1.d);
        test(mo7.e === undefined);
        test(mo7.f == mo1.f);
        test(mo7.g === undefined);
        test(mo7.h == mo1.h);
        test(mo7.i === undefined);
        test(mo7.j.equals(mo1.j));
        test(ArrayUtil.equals(mo7.bs, mo1.bs));
        test(mo7.ss === undefined);
        test(mo7.iid.get(4) == 3);
        test(mo7.sid === undefined);
        test(mo7.fs.equals(mo1.fs));
        test(mo7.vs === undefined);
        test(ArrayUtil.equals(mo7.shs, mo1.shs));
        test(mo7.es === undefined);
        test(mo7.fss[0].equals(new Test.FixedStruct(78)));
        test(mo7.vss === undefined);
        test(mo7.mips === undefined);

        test(mo7.ied === undefined);
        test(mo7.ifsd.get(4).equals(new Test.FixedStruct(78)));
        test(mo7.ivsd === undefined);
        test(mo7.imipd === undefined);

        test(ArrayUtil.equals(mo7.bos, [false, true, false]));

        // Clear the second half of the optional parameters
        const mo8 = new Test.MultiOptional();
        mo8.a = mo1.a;
        mo8.c = mo1.c;
        mo8.e = mo1.e;
        mo8.g = mo1.g;
        mo8.i = mo1.i;
        mo8.ss = mo1.ss;
        mo8.sid = mo1.sid;
        mo8.vs = mo1.vs;

        mo8.es = mo1.es;
        mo8.vss = mo1.vss;
        mo8.mips = mo1.mips;

        mo8.ied = mo1.ied;
        mo8.ivsd = mo1.ivsd;
        mo8.imipd = mo1.imipd;

        const mo9 = (await initial.pingPong(mo8)) as Test.MultiOptional;

        test(mo9.a == mo1.a);
        test(mo9.b === undefined);
        test(mo9.c == mo1.c);
        test(mo9.d === undefined);
        test(mo9.e.equals(mo1.e));
        test(mo9.f === undefined);
        test(mo9.g == mo1.g);
        test(mo9.h === undefined);
        test(mo9.i == mo1.i);
        test(mo9.j === undefined);
        test(mo9.bs === undefined);
        test(ArrayUtil.equals(mo9.ss, mo1.ss));
        test(mo9.iid === undefined);
        test(mo9.sid.get("test") == 10);
        test(mo9.fs === undefined);
        test(mo9.vs.equals(mo1.vs));

        test(mo9.shs === undefined);
        test(mo9.es[0] == Test.MyEnum.MyEnumMember && mo9.es[1] == Test.MyEnum.MyEnumMember);
        test(mo9.fss === undefined);
        test(mo9.vss[0].equals(new Test.VarStruct("hello")));
        test(mo9.mips[0].equals(communicator.stringToProxy("test")));

        test(mo9.ied.get(4) == Test.MyEnum.MyEnumMember);
        test(mo9.ifsd === undefined);
        test(mo9.ivsd.get(5).equals(new Test.VarStruct("hello")));
        test(mo9.imipd.get(5).equals(communicator.stringToProxy("test")));

        test(mo9.bos === undefined);

        let g = new Test.G();
        g.gg1Opt = new Test.G1("gg1Opt");
        g.gg2 = new Test.G2(new Ice.Long(0, 10));
        g.gg2Opt = new Test.G2(new Ice.Long(0, 20));
        g.gg1 = new Test.G1("gg1");

        g = (await initial.opG(g)) as Test.G;

        test(g.gg1Opt.a == "gg1Opt");
        test(g.gg2.a.equals(new Ice.Long(0, 10)));
        test(g.gg2Opt.a.equals(new Ice.Long(0, 20)));
        test(g.gg1.a == "gg1");

        const init2 = ClientPrivate.Initial2Prx.uncheckedCast(initial);
        await init2.opVoid(5, "test");
        out.writeLine("ok");

        out.write("testing marshaling of large containers with fixed size elements... ");
        let mc = new Test.MultiOptional();

        mc.bs = new Uint8Array(1000);
        mc.shs = new Array(300);

        mc.fss = [];
        for (let i = 0; i < 300; ++i) {
            mc.fss[i] = new Test.FixedStruct();
        }

        mc.ifsd = new Map();
        for (let i = 0; i < 300; ++i) {
            mc.ifsd.set(i, new Test.FixedStruct());
        }
        mc = (await initial.pingPong(mc)) as Test.MultiOptional;

        test(mc.bs.length == 1000);
        test(mc.shs.length == 300);
        test(mc.fss.length == 300);
        test(mc.ifsd.size == 300);

        out.writeLine("ok");

        out.write("testing tag marshaling... ");
        let b = (await initial.pingPong(new Test.B())) as Test.B;

        test(b.ma === undefined);
        test(b.mb === undefined);
        test(b.mc === undefined);

        b.ma = 10;
        b.mb = 11;
        b.mc = 12;
        b.md = 13;

        b = (await initial.pingPong(b)) as Test.B;

        test(b.ma == 10);
        test(b.mb == 11);
        test(b.mc == 12);
        test(b.md == 13);
        out.writeLine("ok");

        out.write("testing marshaling of objects with optional members... ");
        let f = new Test.F();
        f.fsf = new Test.FixedStruct();
        f.fse = f.fsf;
        f = (await initial.pingPong(f)) as Test.F;
        test(f.fse.equals(f.fsf));
        out.writeLine("ok");

        out.write("testing optional with default values... ");

        let wd = (await initial.pingPong(new Test.WD())) as Test.WD;
        test(wd.a == 5);
        test(wd.s == "test");
        wd.a = undefined;
        wd.s = undefined;
        wd = (await initial.pingPong(wd)) as Test.WD;

        test(wd.a === undefined);
        test(wd.s === undefined);
        out.writeLine("ok");

        out.write("testing optional parameters... ");
        {
            let [p1, p2] = await initial.opByte(); // same as initial.opByte(undefined);
            test(p1 === undefined);
            test(p2 === undefined);
            [p1, p2] = await initial.opByte(56);
            test(p1 === 56);
            test(p2 === 56);
        }

        {
            let [p1, p2] = await initial.opBool();
            test(p1 === undefined);
            test(p2 === undefined);
            [p1, p2] = await initial.opBool(true);
            test(p1 === true);
            test(p2 === true);
        }

        {
            let [p1, p2] = await initial.opShort();
            test(p1 === undefined);
            test(p2 === undefined);
            [p1, p2] = await initial.opShort(56);
            test(p1 === 56);
            test(p2 === 56);
        }

        {
            let [p1, p2] = await initial.opInt();
            test(p1 === undefined);
            test(p2 === undefined);
            [p1, p2] = await initial.opInt(56);
            test(p1 === 56);
            test(p2 === 56);
        }

        {
            let [p1, p2] = await initial.opLong();
            test(p1 === undefined);
            test(p2 === undefined);
            [p1, p2] = await initial.opLong(new Ice.Long(0, 56));
            test(p1.equals(new Ice.Long(0, 56)));
            test(p2.equals(new Ice.Long(0, 56)));
        }

        {
            let [p1, p2] = await initial.opFloat();
            test(p1 === undefined);
            test(p2 === undefined);
            [p1, p2] = await initial.opFloat(1.0);
            test(p1 === 1.0);
            test(p2 === 1.0);
        }

        {
            let [p1, p2] = await initial.opDouble();
            test(p1 === undefined);
            test(p2 === undefined);
            [p1, p2] = await initial.opDouble(1.0);
            test(p1 === 1.0);
            test(p2 === 1.0);
        }

        {
            let [p1, p2] = await initial.opString();
            test(p1 === undefined);
            test(p2 === undefined);
            [p1, p2] = await initial.opString("test");
            test(p1 === "test");
            test(p2 === "test");
        }

        {
            let [p1, p2] = await initial.opMyEnum();
            test(p1 === undefined);
            test(p2 === undefined);
            [p1, p2] = await initial.opMyEnum(Test.MyEnum.MyEnumMember);
            test(p1 === Test.MyEnum.MyEnumMember);
            test(p2 === Test.MyEnum.MyEnumMember);
            [p1, p2] = await initial.opMyEnum(null); // Test null enum
            test(p1 === undefined);
            test(p2 === undefined);
        }

        {
            let [p1, p2] = await initial.opSmallStruct();
            test(p1 === undefined);
            test(p2 === undefined);
            [p1, p2] = await initial.opSmallStruct(new Test.SmallStruct(56));
            test(p1.equals(new Test.SmallStruct(56)));
            test(p2.equals(new Test.SmallStruct(56)));
            [p1, p2] = await initial.opSmallStruct(null); // Test null struct
            test(p1 === undefined);
            test(p2 === undefined);
        }

        {
            let [p1, p2] = await initial.opFixedStruct();
            test(p1 === undefined);
            test(p2 === undefined);
            [p1, p2] = await initial.opFixedStruct(new Test.FixedStruct(56));
            test(p1.equals(new Test.FixedStruct(56)));
            test(p2.equals(new Test.FixedStruct(56)));
        }

        {
            let [p1, p2] = await initial.opVarStruct();
            test(p1 === undefined);
            test(p2 === undefined);
            [p1, p2] = await initial.opVarStruct(new Test.VarStruct("test"));
            test(p1.equals(new Test.VarStruct("test")));
            test(p2.equals(new Test.VarStruct("test")));
        }

        {
            let [p1, p2] = await initial.opOneOptional(new Test.OneOptional());
            test(p1 === p2);
            test(p2.a === undefined);
            [p1, p2] = await initial.opOneOptional(new Test.OneOptional(58));
            test(p1 === p2);
            test(p2.a === 58);
        }

        {
            let [p1, p2] = await initial.opMyInterfaceProxy();
            test(p1 === undefined);
            test(p2 === undefined);
            [p1, p2] = await initial.opMyInterfaceProxy(new Test.MyInterfacePrx(communicator, "test"));
            test(p1.equals(new Test.MyInterfacePrx(communicator, "test")));
            test(p2.equals(new Test.MyInterfacePrx(communicator, "test")));
        }

        {
            let [p1, p2] = await initial.opByteSeq();
            test(p1 === undefined);
            test(p2 === undefined);

            let data: number[] = [];
            for (let i = 0; i < 100; ++i) {
                data[i] = 56;
            }
            [p1, p2] = await initial.opByteSeq(new Uint8Array(data));

            test(p1.length === 100);
            test(p2.length === 100);
            for (let i = 0; i < 100; ++i) {
                test(p1[i] === 56);
                test(p2[i] === 56);
            }
        }

        {
            let [p1, p2] = await initial.opBoolSeq();
            test(p1 === undefined);
            test(p2 === undefined);

            let data: boolean[] = [];
            for (let i = 0; i < 100; ++i) {
                data[i] = true;
            }
            [p1, p2] = await initial.opBoolSeq(data);
            test(p1.length === 100);
            test(p2.length === 100);
            for (let i = 0; i < 100; ++i) {
                test(p1[i] === true);
                test(p2[i] === true);
            }
        }

        {
            let [p1, p2] = await initial.opShortSeq();
            test(p1 === undefined);
            test(p2 === undefined);

            let data: number[] = [];
            for (let i = 0; i < 100; ++i) {
                data[i] = 56;
            }
            [p1, p2] = await initial.opShortSeq(data);
            test(p1.length === 100);
            test(p2.length === 100);
            for (let i = 0; i < 100; ++i) {
                test(p1[i] === 56);
                test(p2[i] === 56);
            }
        }

        {
            let [p1, p2] = await initial.opIntSeq();
            test(p1 === undefined);
            test(p2 === undefined);

            let data: number[] = [];
            for (let i = 0; i < 100; ++i) {
                data[i] = 56;
            }
            [p1, p2] = await initial.opIntSeq(data);
            test(p1.length === 100);
            test(p2.length === 100);
            for (let i = 0; i < 100; ++i) {
                test(p1[i] === 56);
                test(p2[i] === 56);
            }
        }

        {
            let [p1, p2] = await initial.opLongSeq();
            test(p1 === undefined);
            test(p2 === undefined);
            let data: Ice.Long[] = [];
            for (let i = 0; i < 100; ++i) {
                data[i] = new Ice.Long(0, 56);
            }
            [p1, p2] = await initial.opLongSeq(data);
            test(p1.length === 100);
            test(p2.length === 100);
            for (let i = 0; i < 100; ++i) {
                test(p1[i].equals(new Ice.Long(0, 56)));
                test(p2[i].equals(new Ice.Long(0, 56)));
            }
        }

        {
            let [p1, p2] = await initial.opFloatSeq();
            test(p1 === undefined);
            test(p2 === undefined);
            let data: number[] = [];
            for (let i = 0; i < 100; ++i) {
                data[i] = 1.0;
            }
            [p1, p2] = await initial.opFloatSeq(data);
            test(p1.length === 100);
            test(p2.length === 100);
            for (let i = 0; i < 100; ++i) {
                test(p1[i] === 1.0);
                test(p2[i] === 1.0);
            }
        }

        {
            let [p1, p2] = await initial.opDoubleSeq();
            test(p1 === undefined);
            test(p2 === undefined);
            let data: number[] = [];
            for (let i = 0; i < 100; ++i) {
                data[i] = 1.0;
            }
            [p1, p2] = await initial.opDoubleSeq(data);
            test(p1.length === 100);
            test(p2.length === 100);
            for (let i = 0; i < 100; ++i) {
                test(p1[i] === 1.0);
                test(p2[i] === 1.0);
            }
        }

        {
            let [p1, p2] = await initial.opStringSeq();
            test(p1 === undefined);
            test(p2 === undefined);
            let data: string[] = [];
            for (let i = 0; i < 100; ++i) {
                data[i] = "test1";
            }
            [p1, p2] = await initial.opStringSeq(data);

            test(p1.length === 100);
            test(p2.length === 100);
            for (let i = 0; i < 100; ++i) {
                test(p1[i] == "test1");
                test(p2[i] == "test1");
            }
        }

        {
            let [p1, p2] = await initial.opSmallStructSeq();
            test(p1 === undefined);
            test(p2 === undefined);
            let data: Test.SmallStruct[] = [];
            for (let i = 0; i < 100; ++i) {
                data[i] = new Test.SmallStruct();
            }
            [p1, p2] = await initial.opSmallStructSeq(data);

            test(p1.length === 100);
            test(p2.length === 100);
            for (let i = 0; i < 100; ++i) {
                test(p1[i].equals(new Test.SmallStruct()));
                test(p2[i].equals(new Test.SmallStruct()));
            }
        }

        {
            let [p1, p2] = await initial.opFixedStructSeq();
            test(p1 === undefined);
            test(p2 === undefined);
            let data: Test.FixedStruct[] = [];
            for (let i = 0; i < 100; ++i) {
                data[i] = new Test.FixedStruct();
            }
            [p1, p2] = await initial.opFixedStructSeq(data);

            test(p1.length === 100);
            test(p2.length === 100);
            for (let i = 0; i < 100; ++i) {
                test(p1[i].equals(new Test.FixedStruct()));
                test(p2[i].equals(new Test.FixedStruct()));
            }
        }

        {
            let [p1, p2] = await initial.opVarStructSeq();
            test(p1 === undefined);
            test(p2 === undefined);
            let data: Test.VarStruct[] = [];
            for (let i = 0; i < 100; ++i) {
                data[i] = new Test.VarStruct("");
            }
            [p1, p2] = await initial.opVarStructSeq(data);
            test(p1.length === 100);
            test(p2.length === 100);
            for (let i = 0; i < 100; ++i) {
                test(p1[i].equals(new Test.VarStruct("")));
                test(p2[i].equals(new Test.VarStruct("")));
            }
        }

        {
            let [p1, p2] = await initial.opIntIntDict();
            test(p1 === undefined);
            test(p2 === undefined);
            let data: Map<number, number> = new Map();
            data.set(1, 2);
            data.set(2, 3);
            [p1, p2] = await initial.opIntIntDict(data);
            test(Ice.MapUtil.equals(p1, p2));
        }

        {
            let [p1, p2] = await initial.opStringIntDict();
            test(p1 === undefined);
            test(p2 === undefined);
            let data: Map<string, number> = new Map();
            data.set("1", 1);
            data.set("2", 2);
            [p1, p2] = await initial.opStringIntDict(data);
            test(Ice.MapUtil.equals(p1, p2));
        }
        out.writeLine("ok");

        out.write("testing exception optionals... ");

        try {
            await initial.opOptionalException();
            test(false);
        } catch (ex) {
            test(ex instanceof Test.OptionalException, ex);
            test(ex.a === undefined);
            test(ex.b === undefined);
        }

        try {
            await initial.opOptionalException(30, "test");
            test(false);
        } catch (ex) {
            test(ex instanceof Test.OptionalException, ex);
            test(ex.a === 30);
            test(ex.b == "test");
        }

        try {
            await initial.opDerivedException();
            test(false);
        } catch (ex) {
            test(ex instanceof Test.DerivedException, ex);
            test(ex.a === undefined);
            test(ex.b === undefined);
            test(ex.ss === undefined);
            test(ex.d1 == "d1");
            test(ex.d2 == "d2");
        }

        try {
            await initial.opDerivedException(30, "test2");
            test(false);
        } catch (ex) {
            test(ex instanceof Test.DerivedException, ex);
            test(ex.a === 30);
            test(ex.b == "test2");
            test(ex.ss == "test2");
        }

        out.writeLine("ok");

        out.write("testing optionals with marshaled results... ");

        test((await initial.opMStruct1()) !== undefined);
        test((await initial.opMDict1()) !== undefined);
        test((await initial.opMSeq1()) !== undefined);

        {
            let [p3, p2] = await initial.opMStruct2();
            test(p3 === undefined && p2 == undefined);

            const p1 = new Test.SmallStruct();
            [p3, p2] = await initial.opMStruct2(p1);
            test(p2.equals(p1) && p3.equals(p1));
        }

        {
            let [p3, p2] = await initial.opMSeq2();
            test(p2 === undefined && p3 === undefined);

            const p1 = ["hello"];
            [p3, p2] = await initial.opMSeq2(p1);
            test(ArrayUtil.equals(p2, p1) && ArrayUtil.equals(p3, p1));
        }

        {
            let [p3, p2] = await initial.opMDict2();
            test(p2 === undefined && p3 === undefined);

            const p1 = new Map();
            p1.set("test", 54);
            [p3, p2] = await initial.opMDict2(p1);
            test(Ice.MapUtil.equals(p2, p1) && Ice.MapUtil.equals(p3, p1));
        }

        out.writeLine("ok");

        await initial.shutdown();
    }

    async run(args: string[]) {
        let communicator: Ice.Communicator | null = null;
        try {
            [communicator] = this.initialize(args);
            await this.allTests();
        } finally {
            if (communicator) {
                await communicator.destroy();
            }
        }
    }
}

// Copyright (c) ZeroC, Inc.

import { Ice } from "ice";
import { Test as Test_Test } from "./Test.js";
import { Test as Test_Forward } from "./Forward.js";

const Test = {
    ...Test_Test,
    ...Test_Forward,
};
Test.Inner = Test_Test.Inner;

import { TestHelper } from "../../Common/TestHelper.js";

const test = TestHelper.test;

class BI extends Test.B {
    ice_preMarshal() {
        this.preMarshalInvoked = true;
    }

    ice_postUnmarshal() {
        this.postUnmarshalInvoked = true;
    }
}

class CI extends Test.C {
    ice_preMarshal() {
        this.preMarshalInvoked = true;
    }

    ice_postUnmarshal() {
        this.postUnmarshalInvoked = true;
    }
}

class DI extends Test.D {
    ice_preMarshal() {
        this.preMarshalInvoked = true;
    }

    ice_postUnmarshal() {
        this.postUnmarshalInvoked = true;
    }
}

class EI extends Test.E {
    constructor() {
        super(1, "hello");
    }

    checkValues() {
        return this.i == 1 && this.s == "hello";
    }
}

class FI extends Test.F {
    constructor(e?: Test_Test.E) {
        super(e, e);
    }

    checkValues() {
        return this.e1 !== null && this.e1 === this.e2;
    }
}

function MyValueFactory(type: string) {
    switch (type) {
        case "::Test::B":
            return new BI();
        case "::Test::C":
            return new CI();
        case "::Test::D":
            return new DI();
        case "::Test::E":
            return new EI();
        case "::Test::F":
            return new FI();
        case "::Test::Inner::A":
            return new Test.Inner.A();
        case "::Test::Inner::Sub::A":
            return new Test.Inner.Sub.A();
        default:
            break;
    }
    return null;
}

export class Client extends TestHelper {
    async allTests() {
        const out = this.getWriter();
        const communicator = this.communicator();

        communicator.getValueFactoryManager().add(MyValueFactory, "::Test::B");
        communicator.getValueFactoryManager().add(MyValueFactory, "::Test::C");
        communicator.getValueFactoryManager().add(MyValueFactory, "::Test::D");
        communicator.getValueFactoryManager().add(MyValueFactory, "::Test::E");
        communicator.getValueFactoryManager().add(MyValueFactory, "::Test::F");
        communicator.getValueFactoryManager().add(MyValueFactory, "::Test::Inner::A");
        communicator.getValueFactoryManager().add(MyValueFactory, "::Test::Inner::Sub::A");

        const initial = new Test.InitialPrx(communicator, `initial:${this.getTestEndpoint()}`);

        out.write("getting B1... ");
        let b1 = await initial!.getB1();
        test(b1 !== null);
        out.writeLine("ok");

        out.write("getting B2... ");
        let b2 = await initial!.getB2();
        test(b2 !== null);
        out.writeLine("ok");

        out.write("getting C... ");
        let c = await initial!.getC();
        test(c !== null);
        out.writeLine("ok");

        out.write("getting D... ");
        let d = await initial!.getD();
        test(d !== null);
        out.writeLine("ok");

        out.write("checking consistency... ");
        test(b1 !== b2);

        test(b1!.theB === b1);
        test(b1!.theC === null);
        test(b1!.theA instanceof Test_Test.B);
        test((b1!.theA as Test_Test.B).theA === b1!.theA);
        test(b1!.theA.theB === b1);
        test(b1!.theA.theC instanceof Test.C);
        test(b1!.theA.theC.theB === b1!.theA);

        test(b1!.preMarshalInvoked);
        test(b1!.postUnmarshalInvoked);
        test(b1!.theA.preMarshalInvoked);
        test(b1!.theA.postUnmarshalInvoked);
        test(b1!.theA.theC.preMarshalInvoked);
        test(b1!.theA.theC.postUnmarshalInvoked);

        // More tests possible for b2 and d, but I think this is already
        // sufficient.
        test(b2!.theA === b2);
        test(d!.theC === null);
        out.writeLine("ok");
        out.write("getting B1, B2, C, and D all at once... ");

        [b1, b2, c, d] = await initial!.getAll();

        test(b1 !== null);
        test(b2 !== null);
        test(c !== null);
        test(d !== null);
        out.writeLine("ok");

        out.write("checking consistency... ");
        test(b1 !== b2);
        test(b1!.theA === b2);
        test(b1!.theB === b1);
        test(b1!.theC === null);
        test(b2!.theA === b2);
        test(b2!.theB === b1);
        test(b2!.theC === c);
        test(c!.theB === b2);
        test(d!.theA === b1);
        test(d!.theB === b2);
        test(d!.theC === null);
        test(d!.preMarshalInvoked);
        test(d!.postUnmarshalInvoked);
        test(d!.theA.preMarshalInvoked);
        test(d!.theA.postUnmarshalInvoked);
        test(d!.theB.preMarshalInvoked);
        test(d!.theB.postUnmarshalInvoked);
        test(d!.theB.theC.preMarshalInvoked);
        test(d!.theB.theC.postUnmarshalInvoked);
        out.writeLine("ok");

        out.write("testing protected members... ");
        const e = await initial!.getE();
        test((e as EI).checkValues());

        const f = (await initial!.getF()) as FI;
        test(f.checkValues());
        test((f!.e2 as EI).checkValues());
        out.writeLine("ok");

        out.write("getting K...");
        const k = await initial!.getK();
        test(k !== null);
        test((k!.value as Test_Test.L).data == "l");
        out.writeLine("ok");

        out.write("test Value as parameter...");
        {
            let [v1, v2]: [Ice.Value | null, Ice.Value | null] = await initial!.opValue(new Test.L("l"));
            test((v1 as Test_Test.L).data == "l");
            test((v2 as Test_Test.L).data == "l");

            let [seq1, seq2]: [Ice.Value[], Ice.Value[]] = await initial!.opValueSeq([new Test.L("l")]);
            test((seq1[0] as Test_Test.L).data == "l");
            test((seq2[0] as Test_Test.L).data == "l");

            let [map1, map2]: [Map<string, Ice.Value>, Map<string, Ice.Value>] = await initial!.opValueMap(
                new Map([["l", new Test.L("l")]]),
            );
            test((map1.get("l") as Test_Test.L).data == "l");
            test((map2.get("l") as Test_Test.L).data == "l");
        }
        out.writeLine("ok");

        out.write("getting D1... ");
        const d1 = await initial!.getD1(
            new Test.D1(new Test.A1("a1"), new Test.A1("a2"), new Test.A1("a3"), new Test.A1("a4")),
        );

        test(d1!.a1.name == "a1");
        test(d1!.a2.name == "a2");
        test(d1!.a3.name == "a3");
        test(d1!.a4.name == "a4");
        out.writeLine("ok");

        out.write("throw EDerived... ");
        try {
            await initial!.throwEDerived();
            test(false);
        } catch (ex) {
            test(ex instanceof Test.EDerived, ex);
            test(ex.a1.name == "a1");
            test(ex.a2.name == "a2");
            test(ex.a3.name == "a3");
            test(ex.a4.name == "a4");
        }
        out.writeLine("ok");

        out.write("setting G... ");
        try {
            await initial!.setG(new Test.G(new Test.S("hello"), "g"));
        } catch (ex) {
            test(ex instanceof Ice.OperationNotExistException, ex);
        }
        out.writeLine("ok");

        out.write("testing sequences... ");
        let [retS, outS] = await initial!.opBaseSeq([]);
        [retS, outS] = await initial!.opBaseSeq([new Test.Base(new Test.S(), "")]);
        test(retS.length === 1 && outS.length === 1);
        out.writeLine("ok");

        out.write("testing recursive types... ");

        const top = new Test.Recursive();
        let p = top;
        let maxDepth = 100;

        try {
            for (let i = 0; i <= maxDepth; ++i) {
                p.v = new Test.Recursive();
                p = p.v;
            }
            await initial!.setRecursive(top);
            test(false);
        } catch (ex) {
            // Ice.UnknownLocalException: Expected marshal exception from the server (max class graph depth reached)
            test(ex instanceof Ice.UnknownLocalException, ex);
        }
        await initial!.setRecursive(new Test.Recursive());
        out.writeLine("ok");

        out.write("testing compact ID... ");
        test((await initial!.getCompact()) !== null);
        out.writeLine("ok");

        out.write("testing marshaled results...");
        b1 = await initial!.getMB();
        test(b1 !== null && b1!.theB === b1);
        b1 = await initial!.getAMDMB();
        test(b1 !== null && b1!.theB === b1);
        out.writeLine("ok");

        out.write("testing UnexpectedObjectException... ");
        const uoet = new Test.UnexpectedObjectExceptionTestPrx(communicator, "uoet:" + this.getTestEndpoint());
        try {
            await uoet.op();
            test(false);
        } catch (ex) {
            test(ex instanceof Ice.MarshalException, ex);
            test(ex.message.includes("'::Test::AlsoEmpty'"));
            test(ex.message.includes("'::Test::Empty'"));
        }
        out.writeLine("ok");

        out.write("testing inner modules... ");
        let innerA = await initial!.getInnerA();
        test(innerA instanceof Test.Inner.A);
        test(innerA!.theA instanceof Test.B);
        let innerSubA = await initial!.getInnerSubA();
        test(innerSubA instanceof Test.Inner.Sub.A);
        test(innerSubA!.theA instanceof Test.Inner.A);

        try {
            await initial!.throwInnerEx();
            test(false);
        } catch (ex) {
            test(ex instanceof Test.Inner.Ex, ex);
            test(ex.reason == "Inner::Ex");
        }

        try {
            await initial!.throwInnerSubEx();
            test(false);
        } catch (ex) {
            test(ex instanceof Test.Inner.Sub.Ex, ex);
            test(ex.reason == "Inner::Sub::Ex");
        }
        out.writeLine("ok");

        out.write("testing class containing complex dictionary... ");
        {
            const m = new Test.M();
            m.v = new Test.LMap();
            const k1 = new Test.StructKey(1, "1");
            m.v.set(k1, new Test.L("one"));
            const k2 = new Test.StructKey(2, "2");
            m.v.set(k2, new Test.L("two"));

            const [m1, m2] = await initial!.opM(m);

            test(m1!.v.size == 2);
            test(m2!.v.size == 2);

            test(m1!.v!.get(k1)!.data == "one");
            test(m2!.v!.get(k1)!.data == "one");

            test(m1!.v!.get(k2)!.data == "two");
            test(m2!.v!.get(k2)!.data == "two");
        }
        out.writeLine("ok");

        out.write("testing forward declarations... ");
        {
            const [f11, f12] = await initial!.opF1(new Test.F1("F11"));
            test(f11.name == "F11");
            test(f12.name == "F12");

            const [f21, f22] = await initial!.opF2(
                Test.F2Prx.uncheckedCast(communicator.stringToProxy("F21:" + this.getTestEndpoint())),
            );
            test(f21.ice_getIdentity().name == "F21");
            await f21.op();
            test(f22.ice_getIdentity().name == "F22");

            const hasF3 = await initial!.hasF3();
            if (hasF3) {
                const [f31, f32] = await initial!.opF3(
                    new Test.F3(new Test.F1("F11"), Test.F2Prx.uncheckedCast(communicator.stringToProxy("F21"))),
                );
                test(f31!.f1.name == "F11");
                test(f31!.f2.ice_getIdentity().name == "F21");

                test(f32!.f1.name == "F12");
                test(f32!.f2.ice_getIdentity().name == "F22");
            }
        }
        out.writeLine("ok");

        out.write("testing sending class cycle... ");
        {
            const top = new Test.Recursive();
            let bottom = top;
            const maxDepth = 10;
            for (let i = 1; i < maxDepth; ++i) {
                bottom.v = new Test.Recursive();
                bottom = bottom.v;
            }
            await initial.setRecursive(top);

            // Adding one more level would exceed the max class graph depth
            bottom.v = new Test.Recursive();
            bottom = bottom.v;
            try {
                await initial.setRecursive(top);
                test(false);
            } catch (ex) {
                // Expected marshal exception from the server (max class graph depth reached)
                test(ex instanceof Ice.UnknownLocalException, ex);
            }
        }
        out.writeLine("ok");

        await initial!.shutdown();
    }

    async run(args: string[]) {
        let communicator: Ice.Communicator | null = null;
        try {
            const [properties] = this.createTestProperties(args);
            properties.setProperty("Ice.Warn.Connections", "0");
            [communicator] = this.initialize(properties);
            await this.allTests();
        } finally {
            if (communicator) {
                await communicator.destroy();
            }
        }
    }
}

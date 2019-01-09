// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

(function(module, require, exports)
{
    const Ice = require("ice").Ice;
    const Test = require("Test").Test;
    const TestHelper = require("TestHelper").TestHelper;
    const test = TestHelper.test;

    class BI extends Test.B
    {
        ice_preMarshal()
        {
            this.preMarshalInvoked = true;
        }

        ice_postUnmarshal()
        {
            this.postUnmarshalInvoked = true;
        }
    }

    class CI extends Test.C
    {
        ice_preMarshal()
        {
            this.preMarshalInvoked = true;
        }

        ice_postUnmarshal()
        {
            this.postUnmarshalInvoked = true;
        }
    }

    class DI extends Test.D
    {
        ice_preMarshal()
        {
            this.preMarshalInvoked = true;
        }

        ice_postUnmarshal()
        {
            this.postUnmarshalInvoked = true;
        }
    }

    class EI extends Test.E
    {
        constructor()
        {
            super(1, "hello");
        }

        checkValues()
        {
            return this.i == 1 && this.s == "hello";
        }
    }

    class FI extends Test.F
    {
        constructor(e)
        {
            super(e, e);
        }

        checkValues()
        {
            return this.e1 !== null && this.e1 === this.e2;
        }
    }

    class HI extends Test.H
    {
    }

    class II extends Ice.InterfaceByValue
    {
        constructor()
        {
            super(Test.I.ice_staticId());
        }
    }

    class JI extends Ice.InterfaceByValue
    {
        constructor()
        {
            super(Test.J.ice_staticId());
        }
    }

    function MyValueFactory(type)
    {
        switch(type)
        {
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
            case "::Test::I":
                return new II();
            case "::Test::J":
                return new JI();
            case "::Test::H":
                return new HI();
            case "::Test::Inner::A":
                return new Test.Inner.A();
            case "::Test::Inner::Sub::A":
                return new Test.Inner.Sub.A();
            default:
                break;
        }
        return null;
    }

    class MyObjectFactory
    {

        create(type)
        {
            return null;
        }

        destroy()
        {
        }
    }

    class Client extends TestHelper
    {
        async allTests()
        {
            const out = this.getWriter();
            const communicator = this.communicator();

            communicator.getValueFactoryManager().add(MyValueFactory, "::Test::B");
            communicator.getValueFactoryManager().add(MyValueFactory, "::Test::C");
            communicator.getValueFactoryManager().add(MyValueFactory, "::Test::D");
            communicator.getValueFactoryManager().add(MyValueFactory, "::Test::E");
            communicator.getValueFactoryManager().add(MyValueFactory, "::Test::F");
            communicator.getValueFactoryManager().add(MyValueFactory, "::Test::I");
            communicator.getValueFactoryManager().add(MyValueFactory, "::Test::J");
            communicator.getValueFactoryManager().add(MyValueFactory, "::Test::H");
            communicator.getValueFactoryManager().add(MyValueFactory, "::Test::Inner::A");
            communicator.getValueFactoryManager().add(MyValueFactory, "::Test::Inner::Sub::A");

            communicator.addObjectFactory(new MyObjectFactory(), "TestOF");

            out.write("testing stringToProxy... ");
            let ref = "initial:" + this.getTestEndpoint();
            let base = communicator.stringToProxy(ref);
            test(base !== null);
            out.writeLine("ok");

            out.write("testing checked cast... ");
            const initial = await Test.InitialPrx.checkedCast(base);
            test(initial !== null);
            test(initial.equals(base));
            out.writeLine("ok");

            out.write("getting B1... ");
            let b1 = await initial.getB1();
            test(b1 !== null);
            out.writeLine("ok");

            out.write("getting B2... ");
            let b2 = await initial.getB2();
            test(b2 !== null);
            out.writeLine("ok");

            out.write("getting C... ");
            let c = await initial.getC();
            test(c !== null);
            out.writeLine("ok");

            out.write("getting D... ");
            let d = await initial.getD();
            test(d !== null);
            out.writeLine("ok");

            out.write("checking consistency... ");
            test(b1 !== b2);

            test(b1.theB === b1);
            test(b1.theC === null);
            test(b1.theA instanceof Test.B);
            test(b1.theA.theA === b1.theA);
            test(b1.theA.theB === b1);
            test(b1.theA.theC instanceof Test.C);
            test(b1.theA.theC.theB === b1.theA);

            test(b1.preMarshalInvoked);
            test(b1.postUnmarshalInvoked);
            test(b1.theA.preMarshalInvoked);
            test(b1.theA.postUnmarshalInvoked);
            test(b1.theA.theC.preMarshalInvoked);
            test(b1.theA.theC.postUnmarshalInvoked);

            // More tests possible for b2 and d, but I think this is already
            // sufficient.
            test(b2.theA === b2);
            test(d.theC === null);
            out.writeLine("ok");
            out.write("getting B1, B2, C, and D all at once... ");

            [b1, b2, c, d] = await initial.getAll();

            test(b1);
            test(b2);
            test(c);
            test(d);
            out.writeLine("ok");

            out.write("checking consistency... ");
            test(b1 !== b2);
            test(b1.theA === b2);
            test(b1.theB === b1);
            test(b1.theC === null);
            test(b2.theA === b2);
            test(b2.theB === b1);
            test(b2.theC === c);
            test(c.theB === b2);
            test(d.theA === b1);
            test(d.theB === b2);
            test(d.theC === null);
            test(d.preMarshalInvoked);
            test(d.postUnmarshalInvoked);
            test(d.theA.preMarshalInvoked);
            test(d.theA.postUnmarshalInvoked);
            test(d.theB.preMarshalInvoked);
            test(d.theB.postUnmarshalInvoked);
            test(d.theB.theC.preMarshalInvoked);
            test(d.theB.theC.postUnmarshalInvoked);
            out.writeLine("ok");

            out.write("testing protected members... ");
            const e = await initial.getE();
            test(e.checkValues());

            const f = await initial.getF();
            test(f.checkValues());
            test(f.e2.checkValues());
            out.writeLine("ok");

            out.write("getting I, J and H... ");
            const i = await initial.getI();
            test(i);
            const j = await initial.getJ();
            test(j);
            const h = await initial.getH();
            test(h);
            out.writeLine("ok");

            out.write("getting K...");
            const k = await initial.getK();
            test(k);
            test(k.value.data == "l");
            out.writeLine("ok");

            out.write("test Value as parameter...");
            {
                let [v1, v2] = await initial.opValue(new Test.L("l"));
                test(v1.data == "l");
                test(v2.data == "l");

                [v1, v2] = await initial.opValueSeq([new Test.L("l")]);
                test(v1[0].data == "l");
                test(v2[0].data == "l");

                [v1, v2] = await initial.opValueMap(new Map([["l", new Test.L("l")]]));
                test(v1.get("l").data == "l");
                test(v2.get("l").data == "l");
            }
            out.writeLine("ok");

            out.write("getting D1... ");
            const d1 = await initial.getD1(new Test.D1(new Test.A1("a1"),
                                                       new Test.A1("a2"),
                                                       new Test.A1("a3"),
                                                       new Test.A1("a4")));

            test(d1.a1.name == "a1");
            test(d1.a2.name == "a2");
            test(d1.a3.name == "a3");
            test(d1.a4.name == "a4");
            out.writeLine("ok");

            out.write("throw EDerived... ");
            try
            {
                await initial.throwEDerived();
                test(false);
            }
            catch(ex)
            {
                test(ex instanceof Test.EDerived, ex);
                test(ex.a1.name == "a1");
                test(ex.a2.name == "a2");
                test(ex.a3.name == "a3");
                test(ex.a4.name == "a4");
            }
            out.writeLine("ok");

            out.write("setting G... ");
            try
            {
                await initial.setG(new Test.G(new Test.S("hello"), "g"));
            }
            catch(ex)
            {
                test(ex instanceof Ice.OperationNotExistException, ex);
            }
            out.writeLine("ok");

            out.write("setting I... ");
            await initial.setI(i);
            await initial.setI(j);
            await initial.setI(h);
            out.writeLine("ok");

            out.write("testing sequences... ");
            let [retS, outS] = await initial.opBaseSeq([]);
            [retS, outS] = await initial.opBaseSeq([new Test.Base(new Test.S(), "")]);
            test(retS.length === 1 && outS.length === 1);
            out.writeLine("ok");

            out.write("testing recursive types... ");

            const top = new Test.Recursive();
            let p = top;
            let depth = 0;

            try
            {
                for(; depth <= 1000; ++depth)
                {
                    p.v = new Test.Recursive();
                    p = p.v;
                    if((depth < 10 && (depth % 10) == 0) ||
                       (depth < 1000 && (depth % 100) == 0) ||
                       (depth < 10000 && (depth % 1000) == 0) ||
                       (depth % 10000) == 0)
                    {
                        await initial.setRecursive(top);
                    }
                }
                test(!(await initial.supportsClassGraphDepthMax()));
            }
            catch(ex)
            {
                //
                // Ice.UnknownLocalException: Expected marshal exception from the server (max class graph depth reached)
                // Ice.UnknownException: Expected stack overflow from the server (Java only)
                // Error: Expected, JavaScript stack overflow
                //
                test((ex instanceof Ice.UnknownLocalException) ||
                     (ex instanceof Ice.UnknownException) ||
                     (ex instanceof Error), ex);
            }
            await initial.setRecursive(new Test.Recursive());
            out.writeLine("ok");

            out.write("testing compact ID... ");
            test(await initial.getCompact() !== null);
            out.writeLine("ok");

            out.write("testing marshaled results...");
            b1 = await initial.getMB();
            test(b1 !== null && b1.theB === b1);
            b1 = await initial.getAMDMB();
            test(b1 !== null && b1.theB === b1);
            out.writeLine("ok");

            out.write("testing UnexpectedObjectException... ");
            ref = "uoet:" + this.getTestEndpoint();
            base = communicator.stringToProxy(ref);
            test(base !== null);

            const uoet = Test.UnexpectedObjectExceptionTestPrx.uncheckedCast(base);
            test(uoet !== null);
            try
            {
                await uoet.op();
                test(false);
            }
            catch(ex)
            {
                test(ex instanceof Ice.UnexpectedObjectException, ex);
                test(ex.type == "::Test::AlsoEmpty");
                test(ex.expectedType == "::Test::Empty");
            }
            out.writeLine("ok");

            out.write("testing inner modules... ");
            let innerA = await initial.getInnerA();
            test(innerA instanceof Test.Inner.A);
            test(innerA.theA instanceof Test.B);
            innerA = await initial.getInnerSubA();
            test(innerA instanceof Test.Inner.Sub.A);
            test(innerA.theA instanceof Test.Inner.A);

            try
            {
                await initial.throwInnerEx();
                test(false);
            }
            catch(ex)
            {
                test(ex instanceof Test.Inner.Ex, ex);
                test(ex.reason == "Inner::Ex");
            }

            try
            {
                await initial.throwInnerSubEx();
                test(false);
            }
            catch(ex)
            {
                test(ex instanceof Test.Inner.Sub.Ex, ex);
                test(ex.reason == "Inner::Sub::Ex");
            }
            out.writeLine("ok");

            out.write("testing getting ObjectFactory... ");
            test(communicator.findObjectFactory("TestOF") !== null);
            out.writeLine("ok");

            out.write("testing getting ObjectFactory as ValueFactory... ");
            test(communicator.getValueFactoryManager().find("TestOF") !== null);
            out.writeLine("ok");

            out.write("testing class containing complex dictionary... ");
            {
                const m = new Test.M();
                m.v = new Test.LMap();
                const k1 = new Test.StructKey(1, "1");
                m.v.set(k1, new Test.L("one"));
                const k2 = new Test.StructKey(2, "2");
                m.v.set(k2, new Test.L("two"));

                const [m1, m2] = await initial.opM(m);

                test(m1.v.size == 2);
                test(m2.v.size == 2);

                test(m1.v.get(k1).data == "one");
                test(m2.v.get(k1).data == "one");

                test(m1.v.get(k2).data == "two");
                test(m2.v.get(k2).data == "two");

            }
            out.writeLine("ok");

            await initial.shutdown();
        }

        async run(args)
        {
            let communicator;
            try
            {
                const [properties] = this.createTestProperties(args);
                properties.setProperty("Ice.Warn.Connections", "0");
                [communicator] = this.initialize(properties);
                await this.allTests();
            }
            finally
            {
                if(communicator)
                {
                    await communicator.destroy();
                }
            }
        }
    }
    exports.Client = Client;
}(typeof global !== "undefined" && typeof global.process !== "undefined" ? module : undefined,
  typeof global !== "undefined" && typeof global.process !== "undefined" ? require :
  (typeof WorkerGlobalScope !== "undefined" && self instanceof WorkerGlobalScope) ? self.Ice._require : window.Ice._require,
  typeof global !== "undefined" && typeof global.process !== "undefined" ? exports :
  (typeof WorkerGlobalScope !== "undefined" && self instanceof WorkerGlobalScope) ? self : window));

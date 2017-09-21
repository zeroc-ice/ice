// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

(function(module, require, exports)
{
    var Ice = require("ice").Ice;
    var Test = require("Test").Test;

    function loop(fn, repetitions) {
        var i = 0;
        var next = function next() {
            while (i++ < repetitions) {
                var r = fn.call(null, i);
                if (r) {
                    return r.then(next);
                }
            }
        };
        return next();
    }

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

    var allTests = function(out, communicator)
    {
        var factory, ref, base, initial, b1, b2, c, d, i, j, h;

        var p = new Ice.Promise();
        var test = function(b)
        {
            if(!b)
            {
                try
                {
                    throw new Error("test failed");
                }
                catch(err)
                {
                    p.reject(err);
                    throw err;
                }
            }
        };

        Ice.Promise.try(
            function()
            {
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
                ref = "initial:default -p 12010";
                base = communicator.stringToProxy(ref);
                test(base !== null);
                out.writeLine("ok");

                out.write("testing checked cast... ");
                return Test.InitialPrx.checkedCast(base);
            }
        ).then(obj =>
            {
                initial = obj;
                test(initial !== null);
                test(initial.equals(base));
                out.writeLine("ok");
            }
        ).then(() =>
            {
                out.write("getting B1... ");
                return initial.getB1();
            }
        ).then(obj =>
            {
                b1 = obj;
                test(b1 !== null);
                out.writeLine("ok");
                out.write("getting B2... ");
                return initial.getB2();
            }
        ).then(obj =>
            {
                b2 = obj;
                test(b2 !== null);
                out.writeLine("ok");
                out.write("getting C... ");
                return initial.getC();
            }
        ).then(obj =>
            {
                c = obj;
                test(c !== null);
                out.writeLine("ok");
                out.write("getting D... ");
                return initial.getD();
            }
        ).then(obj =>
            {
                d = obj;
                test(d !== null);
                out.writeLine("ok");
            }
        ).then(() =>
            {
                out.write("checking consistency... ");
                test(b1 !== b2);
                //test(b1 != c);
                //test(b1 != d);
                //test(b2 != c);
                //test(b2 != d);
                //test(c != d);

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

                return initial.getAll();
            }
        ).then(r =>
            {
                var [b1, b2, c, d] = r;
                test(b1);
                test(b2);
                test(c);
                test(d);
                out.writeLine("ok");

                out.write("checking consistency... ");
                test(b1 !== b2);
                //test(b1 != c);
                //test(b1 != d);
                //test(b2 != c);
                //test(b2 != d);
                //test(c != d);
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
                return initial.getE();
            }
        ).then(e =>
            {
                test(e.checkValues());
                return initial.getF();
            }
        ).then(f =>
            {
                test(f.checkValues());
                test(f.e2.checkValues());
                out.writeLine("ok");
                out.write("getting I, J and H... ");
                return initial.getI();
            }
        ).then(obj =>
            {
                i = obj;
                test(i);
                return initial.getJ();
            }
        ).then(obj =>
            {
                j = obj;
                test(j);
                return initial.getH();
            }
        ).then(obj =>
            {
                h = obj;
                test(h);
                out.writeLine("ok");
                out.write("getting D1... ");
                return initial.getD1(new Test.D1(new Test.A1("a1"),
                                                 new Test.A1("a2"),
                                                 new Test.A1("a3"),
                                                 new Test.A1("a4")));
            }
        ).then(d1 =>
            {
                test(d1.a1.name == "a1");
                test(d1.a2.name == "a2");
                test(d1.a3.name == "a3");
                test(d1.a4.name == "a4");
                out.writeLine("ok");
                out.write("throw EDerived... ");
                return initial.throwEDerived();
            }
        ).then(() => test(false),
            ex =>
            {
                test(ex instanceof Test.EDerived);
                test(ex.a1.name == "a1");
                test(ex.a2.name == "a2");
                test(ex.a3.name == "a3");
                test(ex.a4.name == "a4");
                out.writeLine("ok");
                out.write("setting I... ");
                return initial.setI(i);
            }
        ).then(() => initial.setI(j)
        ).then(() => initial.setI(h)
        ).then(() =>
            {
                out.writeLine("ok");
                out.write("testing sequences... ");
                return initial.opBaseSeq([]);
            }
        ).then(r =>
            {
                var [retS, outS] = r;
                return initial.opBaseSeq([new Test.Base(new Test.S(), "")]);
            }
        ).then(r =>
            {
                var [retS, outS] = r;
                test(retS.length === 1 && outS.length === 1);
                out.writeLine("ok");

                out.write("testing recursive types... ");

                var top = new Test.Recursive();
                var p = top;
                return loop(depth => {
                    p.v = new Test.Recursive();
                    p = p.v;
                    if((depth < 10 && (depth % 10) === 0) ||
                       (depth < 1000 && (depth % 100) === 0) ||
                       (depth < 10000 && (depth % 1000) === 0) ||
                       (depth % 10000) === 0)
                    {
                        return initial.setRecursive(top);
                    }
                    return null;
                }, 20001);
            }
        ).then(() =>
            {
                return initial.supportsClassGraphDepthMax().then(function(v) { test(!v); });
            },
            (ex) =>
            {
                if(ex instanceof Ice.UnknownLocalException)
                {
                    // Expected marshal exception from the server (max class graph depth reached)
                }
                else if(ex instanceof Ice.UnknownException)
                {
                    // Expected stack overflow from the server (Java only)
                }
                else if(ex instanceof Error)
                {
                    // Expected, JavaScript stack overflow
                }
                else
                {
                    throw ex;
                }
            }
        ).then(()=>
            {
                return initial.setRecursive(new Test.Recursive());
            }
        ).then(()=>
            {
                out.writeLine("ok");

                out.write("testing compact ID... ");
                return initial.getCompact();
            }
        ).then(compact =>
            {
                test(compact !== null);
                out.writeLine("ok");

                out.write("testing marshaled results...");
                return initial.getMB();
            }
        ).then(b1 =>
            {
                test(b1 !== null && b1.theB === b1);
                return initial.getAMDMB();
            }
        ).then(b1 =>
            {
                test(b1 !== null && b1.theB === b1);
                out.writeLine("ok");

                out.write("testing UnexpectedObjectException... ");
                ref = "uoet:default -p 12010";
                base = communicator.stringToProxy(ref);
                test(base !== null);
                var uoet = Test.UnexpectedObjectExceptionTestPrx.uncheckedCast(base);
                test(uoet !== null);
                return uoet.op();
            }
        ).then(() =>
            {
                test(false);
            },
            ex =>
            {
                if(!(ex instanceof Ice.ObjectNotExistException))
                {
                    test(ex instanceof Ice.UnexpectedObjectException);
                    test(ex.type == "::Test::AlsoEmpty");
                    test(ex.expectedType == "::Test::Empty");
                }
            }
        ).then(() =>
            {
                out.writeLine("ok");
                out.write("testing inner modules... ");
                return initial.getInnerA();
            }
        ).then(innerA =>
            {
                test(innerA instanceof Test.Inner.A);
                test(innerA.theA instanceof Test.B);
                return initial.getInnerSubA();
            }
        ).then(innerA =>
            {
                test(innerA instanceof Test.Inner.Sub.A);
                test(innerA.theA instanceof Test.Inner.A);
                return initial.throwInnerEx();
            }
        ).then(() => test(false),
            ex =>
            {
                test(ex.reason == "Inner::Ex");
                return initial.throwInnerSubEx();
            }
        ).then(() => test(false),
            ex =>
            {
                test(ex.reason == "Inner::Sub::Ex");
                out.writeLine("ok");

                out.write("testing getting ObjectFactory... ");
                test(communicator.findObjectFactory("TestOF") !== null);
                out.writeLine("ok");
                out.write("testing getting ObjectFactory as ValueFactory... ");
                test(communicator.getValueFactoryManager().find("TestOF") !== null);
                out.writeLine("ok");

                return initial.shutdown();
            }
        ).then(p.resolve, p.reject);
        return p;
    };

    var run = function(out, id)
    {
        id.properties.setProperty("Ice.Warn.Connections", "0");
        var c = Ice.initialize(id);
        return Ice.Promise.try(() => allTests(out, c)).finally(() => c.destroy());
    };
    exports._test = run;
    exports._runServer = true;
}
(typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? module : undefined,
 typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? require : this.Ice._require,
 typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? exports : this));

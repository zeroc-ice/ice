// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

(function(module, require, exports)
{
    var Ice = require("ice").Ice;
    var Test = require("Test").Test;

    var Promise = Ice.Promise;

    var BI = function()
    {
        Test.B.call(this);
        this._postUnmarshalInvoked = false;
    };

    BI.prototype = new Test.B();
    BI.prototype.constructor = BI;

    BI.prototype.postUnmarshalInvoked = function(current)
    {
        return this._postUnmarshalInvoked;
    };

    BI.prototype.ice_preMarshal = function()
    {
        this.preMarshalInvoked = true;
    };

    BI.prototype.ice_postUnmarshal = function()
    {
        this._postUnmarshalInvoked = true;
    };

    var CI = function()
    {
        Test.C.call(this);
        this._postUnmarshalInvoked = false;
    };

    CI.prototype = new Test.C();
    CI.prototype.constructor = CI;

    CI.prototype.postUnmarshalInvoked = function(current)
    {
        return this._postUnmarshalInvoked;
    };

    CI.prototype.ice_preMarshal = function()
    {
        this.preMarshalInvoked = true;
    };

    CI.prototype.ice_postUnmarshal = function()
    {
        this._postUnmarshalInvoked = true;
    };

    var DI = function()
    {
        Test.D.call(this);
        this._postUnmarshalInvoked = false;
    };

    DI.prototype = new Test.D();

    DI.prototype.postUnmarshalInvoked = function(current)
    {
        return this._postUnmarshalInvoked;
    };

    DI.prototype.ice_preMarshal = function()
    {
        this.preMarshalInvoked = true;
    };

    DI.prototype.ice_postUnmarshal = function()
    {
        this._postUnmarshalInvoked = true;
    };

    var EI = function()
    {
        Test.E.call(this, 1, "hello");
    };

    EI.prototype = new Test.E();

    EI.prototype.constructor = EI;

    EI.prototype.checkValues = function(current)
    {
        return this.i == 1 && this.s == "hello";
    };

    var FI = function(e)
    {
        if(e !== undefined)
        {
            Test.F.call(this, e, e);
        }
    };

    FI.prototype = new Test.F();

    FI.prototype.checkValues = function(current)
    {
        return this.e1 !== null && this.e1 === this.e2;
    };

    var HI = function()
    {
        Test.H.call(this);
    };

    HI.prototype = new Test.H();
    HI.prototype.constructor = HI;

    var II = function()
    {
        Test.I.call(this);
    };
    II.prototype = new Test.I();
    II.prototype.constructor = II;

    var JI = function()
    {
        Test.J.call(this);
    };
    JI.prototype = new Test.J();
    JI.prototype.constructor = JI;

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

    var MyObjectFactory = function()
    {
        Ice.ObjectFactory.call(this);
    };

    MyObjectFactory.prototype = new Ice.ObjectFactory();

    MyObjectFactory.prototype.constructor = MyObjectFactory;

    MyObjectFactory.prototype.create = function(type)
    {
        return null;
    };

    MyObjectFactory.prototype.destroy = function()
    {
    };

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
                    p.fail(err);
                    throw err;
                }
            }
        };

        Promise.try(
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
        ).then(
            function(obj)
            {
                initial = obj;
                test(initial !== null);
                test(initial.equals(base));
                out.writeLine("ok");
            }
        ).then(
            function()
            {
                out.write("getting B1... ");
                return initial.getB1();
            }
        ).then(
            function(obj)
            {
                b1 = obj;
                test(b1 !== null);
                out.writeLine("ok");
                out.write("getting B2... ");
                return initial.getB2();
            }
        ).then(
            function(obj)
            {
                b2 = obj;
                test(b2 !== null);
                out.writeLine("ok");
                out.write("getting C... ");
                return initial.getC();
            }
        ).then(
            function(obj)
            {
                c = obj;
                test(c !== null);
                out.writeLine("ok");
                out.write("getting D... ");
                return initial.getD();
            }
        ).then(
            function(obj)
            {
                d = obj;
                test(d !== null);
                out.writeLine("ok");
            }
        ).then(
            function()
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
                test(b1.postUnmarshalInvoked(null));
                test(b1.theA.preMarshalInvoked);
                test(b1.theA.postUnmarshalInvoked(null));
                test(b1.theA.theC.preMarshalInvoked);
                test(b1.theA.theC.postUnmarshalInvoked(null));

                // More tests possible for b2 and d, but I think this is already
                // sufficient.
                test(b2.theA === b2);
                test(d.theC === null);
                out.writeLine("ok");
                out.write("getting B1, B2, C, and D all at once... ");

                return initial.getAll();
            }
        ).then(
            function(b1, b2, c, d)
            {
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
                test(d.postUnmarshalInvoked(null));
                test(d.theA.preMarshalInvoked);
                test(d.theA.postUnmarshalInvoked(null));
                test(d.theB.preMarshalInvoked);
                test(d.theB.postUnmarshalInvoked(null));
                test(d.theB.theC.preMarshalInvoked);
                test(d.theB.theC.postUnmarshalInvoked(null));
                out.writeLine("ok");

                out.write("testing protected members... ");
                return initial.getE();
            }
        ).then(
            function(e)
            {
                test(e.checkValues());
                return initial.getF();
            }
        ).then(
            function(f)
            {
                test(f.checkValues());
                test(f.e2.checkValues());
                out.writeLine("ok");
                out.write("getting I, J and H... ");
                return initial.getI();
            }
        ).then(
            function(obj)
            {
                i = obj;
                test(i);
                return initial.getJ();
            }
        ).then(
            function(obj)
            {
                j = obj;
                test(j);
                return initial.getH();
            }
        ).then(
            function(obj)
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
        ).then(
            function(d1)
            {
                test(d1.a1.name == "a1");
                test(d1.a2.name == "a2");
                test(d1.a3.name == "a3");
                test(d1.a4.name == "a4");
                out.writeLine("ok");
                out.write("throw EDerived... ");
                return initial.throwEDerived();
            }
        ).then(
            function()
            {
                test(false);
            },
            function(ex)
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
        ).then(
            function()
            {
                return initial.setI(j);
            }
        ).then(
            function()
            {
                return initial.setI(h);
            }
        ).then(
            function()
            {
                out.writeLine("ok");
                out.write("testing sequences... ");
                return initial.opBaseSeq([]);
            }
        ).then(
            function(retS, outS)
            {
                return initial.opBaseSeq([new Test.Base(new Test.S(), "")]);
            }
        ).then(
            function(retS, outS)
            {
                test(retS.length === 1 && outS.length === 1);
                out.writeLine("ok");
                out.write("testing compact ID... ");

                return initial.getCompact();
            }
        ).then(
            function(compact)
            {
                test(compact !== null);
                out.writeLine("ok");
                out.write("testing UnexpectedObjectException... ");
                ref = "uoet:default -p 12010";
                base = communicator.stringToProxy(ref);
                test(base !== null);
                var uoet = Test.UnexpectedObjectExceptionTestPrx.uncheckedCast(base);
                test(uoet !== null);
                return uoet.op();
            }
        ).then(
            function()
            {
                test(false);
            },
            function(ex)
            {
                test(ex instanceof Ice.UnexpectedObjectException);
                test(ex.type == "::Test::AlsoEmpty");
                test(ex.expectedType == "::Test::Empty");
            }
        ).then(
            function()
            {
                out.writeLine("ok");
                out.write("testing inner modules... ");
                return initial.getInnerA();
            }
        ).then(
            function(innerA)
            {
                test(innerA instanceof Test.Inner.A);
                test(innerA.theA instanceof Test.B);
                return initial.getInnerSubA();
            }
        ).then(
            function(innerA)
            {
                test(innerA instanceof Test.Inner.Sub.A);
                test(innerA.theA instanceof Test.Inner.A);
                return initial.throwInnerEx();
            }
        ).then(
            function()
            {
                test(false);
            },
            function(ex)
            {
                test(ex.reason == "Inner::Ex");
                return initial.throwInnerSubEx();
            }
        ).then(
            function()
            {
                test(false);
            },
            function(ex)
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
        ).then(
            function()
            {
                p.succeed();
            },
            function(ex)
            {
                p.fail(ex);
            }
        );
        return p;
    };

    var run = function(out, id)
    {
        var c = Ice.initialize(id);
        return Promise.try(
            function()
            {
                return allTests(out, c);
            }
        ).finally(
            function()
            {
                return c.destroy();
            }
        );
    };
    exports.__test__ = run;
    exports.__runServer__ = true;
}
(typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? module : undefined,
 typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? require : this.Ice.__require,
 typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? exports : this));

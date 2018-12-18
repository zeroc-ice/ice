// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICEthis._LICENSE file included in this distribution.
//
// **********************************************************************

(function(module, require, exports)
{
    const Ice = require("ice").Ice;
    const Test = require("Test").Test;

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

    class InitialI extends Test.Initial
    {
        constructor(communicator)
        {
            super();
            if(communicator !== undefined)
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
            }

            this._b1 = new BI();
            this._b2 = new BI();
            this._c = new CI();
            this._d = new DI();
            this._e = new EI();
            this._f = new FI(this._e, this._e);

            this._b1.theA = this._b2; // Cyclic reference to another B
            this._b1.theB = this._b1; // Self reference.
            this._b1.theC = null; // Null reference.

            this._b2.theA = this._b2; // Self reference, using base.
            this._b2.theB = this._b1; // Cyclic reference to another B
            this._b2.theC = this._c; // Cyclic reference to a C.

            this._c.theB = this._b2; // Cyclic reference to a B.

            this._d.theA = this._b1; // Reference to a B.
            this._d.theB = this._b2; // Reference to a B.
            this._d.theC = null; // Reference to a C.
        }

        getAll(current)
        {
            this._b1.preMarshalInvoked = false;
            this._b2.preMarshalInvoked = false;
            this._c.preMarshalInvoked = false;
            this._d.preMarshalInvoked = false;
            return [this._b1, this._b2, this._c, this._d];
        }

        getB1(current)
        {
            this._b1.preMarshalInvoked = false;
            this._b2.preMarshalInvoked = false;
            this._c.preMarshalInvoked = false;
            return this._b1;
        }

        getB2(current)
        {
            this._b1.preMarshalInvoked = false;
            this._b2.preMarshalInvoked = false;
            this._c.preMarshalInvoked = false;
            return this._b2;
        }

        getC(current)
        {
            this._b1.preMarshalInvoked = false;
            this._b2.preMarshalInvoked = false;
            this._c.preMarshalInvoked = false;
            return this._c;
        }

        getD(current)
        {
            this._b1.preMarshalInvoked = false;
            this._b2.preMarshalInvoked = false;
            this._c.preMarshalInvoked = false;
            this._d.preMarshalInvoked = false;
            return this._d;
        }

        getE(current)
        {
            return this._e;
        }

        getF(current)
        {
            return this._f;
        }

        setRecursive(r, current)
        {
        }

        supportsClassGraphDepthMax(current)
        {
            return false;
        }

        getMB(current)
        {
            return this._b1;
        }

        getAMDMB(current)
        {
            return Ice.Promise.resolve(this._b1);
        }

        getI(current)
        {
            return new II();
        }

        getJ(current)
        {
            return new JI();
        }

        getH(current)
        {
            return new HI();
        }

        getK(current)
        {
            return new Test.K(new Test.L("l"));
        }

        opValue(v1, current)
        {
            return [v1, v1];
        }

        opValueSeq(v1, current)
        {
            return [v1, v1];
        }

        opValueMap(v1, current)
        {
            return [v1, v1];
        }

        getD1(d1, current)
        {
            return d1;
        }

        throwEDerived(current)
        {
            throw new Test.EDerived(new Test.A1("a1"), new Test.A1("a2"), new Test.A1("a3"), new Test.A1("a4"));
        }

        setG(theG, current)
        {
        }

        setI(theI, current)
        {
        }

        opBaseSeq(inS, current)
        {
            return [inS, inS];
        }

        getCompact(current)
        {
            return new Test.CompactExt();
        }

        getInnerA(current)
        {
            return new Test.Inner.A(this._b1);
        }

        getInnerSubA(current)
        {
            return new Test.Inner.Sub.A(new Test.Inner.A(this._b1));
        }

        throwInnerEx(current)
        {
            throw new Test.Inner.Ex("Inner::Ex");
        }

        throwInnerSubEx(current)
        {
            throw new Test.Inner.Sub.Ex("Inner::Sub::Ex");
        }

        opM(v1, current)
        {
            return [v1, v1];
        }

        shutdown(current)
        {
            current.adapter.getCommunicator().shutdown();
        }
    }

    exports.InitialI = InitialI;
}(typeof global !== "undefined" && typeof global.process !== "undefined" ? module : undefined,
  typeof global !== "undefined" && typeof global.process !== "undefined" ? require :
  (typeof WorkerGlobalScope !== "undefined" && self instanceof WorkerGlobalScope) ? self.Ice._require : window.Ice._require,
  typeof global !== "undefined" && typeof global.process !== "undefined" ? exports :
  (typeof WorkerGlobalScope !== "undefined" && self instanceof WorkerGlobalScope) ? self : window));

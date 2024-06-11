//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import { Ice } from "ice";

import { Test as Test_Test } from "./Test.js";
import { Test as Test_Forward } from "./Forward.js";

const Test = {
    ...Test_Test,
    ...Test_Forward
};
Test.Inner = Test_Test.Inner;

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
        case "::Test::Inner::A":
            return new Test.Inner.A();
        case "::Test::Inner::Sub::A":
            return new Test.Inner.Sub.A();
        default:
            break;
    }
    return null;
}

export class InitialI extends Test.Initial
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

    setCycle(r, current)
    {
    }

    acceptsClassCycles(current)
    {
        return true;
    }

    getMB(current)
    {
        return this._b1;
    }

    getAMDMB(current)
    {
        return Ice.Promise.resolve(this._b1);
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

    opF1(f11, current)
    {
        return [f11, new Test.F1("F12")];
    }

    opF2(f21, current)
    {
        return [f21, current.adapter.getCommunicator().stringToProxy("F22")];
    }

    opF3(f31, current)
    {
        return [f31,
                new Test.F3(new Test.F1("F12"),
                            current.adapter.getCommunicator().stringToProxy("F22"))];
    }

    hasF3(current)
    {
        return true;
    }

    opN(p1, current)
    {
        return p1;
    }

    shutdown(current)
    {
        current.adapter.getCommunicator().shutdown();
    }
}

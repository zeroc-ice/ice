// Copyright (c) ZeroC, Inc.

import { Ice } from "ice";

import { Test } from "./Test.js";
import { Test as Test_Forward } from "./Forward.js";

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
    constructor(e?: Test.E) {
        super(e, e);
    }

    checkValues() {
        return this.e1 !== null && this.e1 === this.e2;
    }
}

function MyValueFactory(type: string): Ice.Value | null {
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

export class InitialI extends Test.Initial {
    _b1: BI;
    _b2: BI;
    _c: CI;
    _d: DI;
    _e: EI;
    _f: FI;

    constructor(communicator: Ice.Communicator) {
        super();
        if (communicator !== undefined) {
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
        this._f = new FI(this._e);

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

    getAll(current: Ice.Current): [Test.B | null, Test.B | null, Test.C | null, Test.D | null] {
        this._b1.preMarshalInvoked = false;
        this._b2.preMarshalInvoked = false;
        this._c.preMarshalInvoked = false;
        this._d.preMarshalInvoked = false;
        return [this._b1, this._b2, this._c, this._d];
    }

    getB1(current: Ice.Current) {
        this._b1.preMarshalInvoked = false;
        this._b2.preMarshalInvoked = false;
        this._c.preMarshalInvoked = false;
        return this._b1;
    }

    getB2(current: Ice.Current) {
        this._b1.preMarshalInvoked = false;
        this._b2.preMarshalInvoked = false;
        this._c.preMarshalInvoked = false;
        return this._b2;
    }

    getC(current: Ice.Current) {
        this._b1.preMarshalInvoked = false;
        this._b2.preMarshalInvoked = false;
        this._c.preMarshalInvoked = false;
        return this._c;
    }

    getD(current: Ice.Current) {
        this._b1.preMarshalInvoked = false;
        this._b2.preMarshalInvoked = false;
        this._c.preMarshalInvoked = false;
        this._d.preMarshalInvoked = false;
        return this._d;
    }

    getE(current: Ice.Current) {
        return this._e;
    }

    getF(current: Ice.Current) {
        return this._f;
    }

    setRecursive(r: Test.Recursive, current: Ice.Current) {}

    setCycle(r: Test.Recursive, current: Ice.Current) {}

    acceptsClassCycles(current: Ice.Current) {
        return true;
    }

    getMB(current: Ice.Current) {
        return this._b1;
    }

    getAMDMB(current: Ice.Current) {
        return Ice.Promise.resolve(this._b1);
    }

    getK(current: Ice.Current) {
        return new Test.K(new Test.L("l"));
    }
    opValue(v1: Ice.Value | null, current: Ice.Current): [Ice.Value | null, Ice.Value | null] {
        return [v1, v1];
    }

    opValueSeq(v1: (Ice.Value | null)[], current: Ice.Current): [(Ice.Value | null)[], (Ice.Value | null)[]] {
        return [v1, v1];
    }

    opValueMap(
        v1: Map<string, Ice.Value | null>,
        current: Ice.Current,
    ): [Map<string, Ice.Value | null>, Map<string, Ice.Value | null>] {
        return [v1, v1];
    }

    getD1(d1: Test.D1 | null, current: Ice.Current): Test.D1 | null {
        return d1;
    }

    throwEDerived(current: Ice.Current) {
        throw new Test.EDerived(new Test.A1("a1"), new Test.A1("a2"), new Test.A1("a3"), new Test.A1("a4"));
    }

    setG(theG: Test.G, current: Ice.Current) {}

    opBaseSeq(inS: (Test.Base | null)[], current: Ice.Current): [(Test.Base | null)[], (Test.Base | null)[]] {
        return [inS, inS];
    }

    getCompact(current: Ice.Current) {
        return new Test.CompactExt();
    }

    getInnerA(current: Ice.Current) {
        return new Test.Inner.A(this._b1);
    }

    getInnerSubA(current: Ice.Current) {
        return new Test.Inner.Sub.A(new Test.Inner.A(this._b1));
    }

    throwInnerEx(current: Ice.Current) {
        throw new Test.Inner.Ex("Inner::Ex");
    }

    throwInnerSubEx(current: Ice.Current) {
        throw new Test.Inner.Sub.Ex("Inner::Sub::Ex");
    }

    opM(v1: Test.M | null, current: Ice.Current): [Test.M | null, Test.M | null] {
        return [v1, v1];
    }

    opF1(f11: Test_Forward.F1 | null, current: Ice.Current): [Test_Forward.F1 | null, Test_Forward.F1 | null] {
        return [f11, new Test_Forward.F1("F12")];
    }

    opF2(f21: Test_Forward.F2Prx | null, current: Ice.Current): [Test_Forward.F2Prx | null, Test_Forward.F2Prx | null] {
        return [f21, Test_Forward.F2Prx.uncheckedCast(current.adapter.getCommunicator().stringToProxy("F22"))];
    }

    opF3(f31: Test.F3 | null, current: Ice.Current): [Test.F3 | null, Test.F3 | null] {
        return [
            f31,
            new Test.F3(
                new Test_Forward.F1("F12"),
                Test_Forward.F2Prx.uncheckedCast(current.adapter.getCommunicator().stringToProxy("F22")),
            ),
        ];
    }

    hasF3(current: Ice.Current) {
        return true;
    }

    shutdown(current: Ice.Current) {
        current.adapter.getCommunicator().shutdown();
    }
}

// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICEthis._LICENSE file included in this distribution.
//
// **********************************************************************

import {Ice} from "ice";
import {Test} from "./generated"

export class BI extends Test.B
{
    ice_preMarshal():void
    {
        this.preMarshalInvoked = true;
    }

    ice_postUnmarshal():void
    {
        this.postUnmarshalInvoked = true;
    }
}

export class CI extends Test.C
{
    ice_preMarshal():void
    {
        this.preMarshalInvoked = true;
    }

    ice_postUnmarshal():void
    {
        this.postUnmarshalInvoked = true;
    }
}

export class DI extends Test.D
{
    ice_preMarshal():void
    {
        this.preMarshalInvoked = true;
    }

    ice_postUnmarshal():void
    {
        this.postUnmarshalInvoked = true;
    }
}

export class EI extends Test.E
{
    constructor()
    {
        super(1, "hello");
    }

    checkValues():boolean
    {
        return this.i == 1 && this.s == "hello";
    }
}

export class FI extends Test.F
{
    constructor(e?:Test.E)
    {
        super(e, e);
    }

    checkValues():boolean
    {
        return this.e1 !== null && this.e1 === this.e2;
    }
}

export class HI extends Test.H
{
}

export class II extends Ice.InterfaceByValue
{
    constructor()
    {
        super(Test.I.ice_staticId());
    }
}

export class JI extends Ice.InterfaceByValue
{
    constructor()
    {
        super(Test.J.ice_staticId());
    }
}

export function MyValueFactory(type:string):Ice.Value
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

export class InitialI extends Test.Initial
{
    constructor(communicator:Ice.Communicator)
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

    getAll(current:Ice.Current):[Test.B, Test.B, Test.C, Test.D]
    {
        this._b1.preMarshalInvoked = false;
        this._b2.preMarshalInvoked = false;
        this._c.preMarshalInvoked = false;
        this._d.preMarshalInvoked = false;
        return [this._b1, this._b2, this._c, this._d];
    }

    getB1(current:Ice.Current):Test.B
    {
        this._b1.preMarshalInvoked = false;
        this._b2.preMarshalInvoked = false;
        this._c.preMarshalInvoked = false;
        return this._b1;
    }

    getB2(current:Ice.Current):Test.B
    {
        this._b1.preMarshalInvoked = false;
        this._b2.preMarshalInvoked = false;
        this._c.preMarshalInvoked = false;
        return this._b2;
    }

    getC(current:Ice.Current):Test.C
    {
        this._b1.preMarshalInvoked = false;
        this._b2.preMarshalInvoked = false;
        this._c.preMarshalInvoked = false;
        return this._c;
    }

    getD(current:Ice.Current):Test.D
    {
        this._b1.preMarshalInvoked = false;
        this._b2.preMarshalInvoked = false;
        this._c.preMarshalInvoked = false;
        this._d.preMarshalInvoked = false;
        return this._d;
    }

    getE(current:Ice.Current):Test.E
    {
        return this._e;
    }

    getF(current:Ice.Current):Test.F
    {
        return this._f;
    }

    setRecursive(r:Test.Recursive, current:Ice.Current):void
    {
    }

    supportsClassGraphDepthMax(current:Ice.Current):boolean
    {
        return false;
    }

    getMB(current:Ice.Current):Test.B
    {
        return this._b1;
    }

    getAMDMB(current:Ice.Current):PromiseLike<Test.B>
    {
        return Promise.resolve(this._b1);
    }

    getI(current:Ice.Current):Ice.Value
    {
        return new II();
    }

    getJ(current:Ice.Current):Ice.Value
    {
        return new JI();
    }

    getH(current:Ice.Current):Ice.Value
    {
        return new HI();
    }

    getK(current:Ice.Current)
    {
        return new Test.K(new Test.L("l"));
    }

    opValue(v1:Ice.Value, current:Ice.Current):[Ice.Value, Ice.Value]
    {
        return [v1, v1];
    }

    opValueSeq(v1:Ice.Value[], current:Ice.Current):[Ice.Value[], Ice.Value[]]
    {
        return [v1, v1];
    }

    opValueMap(v1:Map<string, Ice.Value>, current:Ice.Current):[Map<string, Ice.Value>, Map<string, Ice.Value>]
    {
        return [v1, v1];
    }

    getD1(d1:Test.D1, current:Ice.Current):Test.D1
    {
        return d1;
    }

    throwEDerived(current:Ice.Current)
    {
        throw new Test.EDerived(new Test.A1("a1"), new Test.A1("a2"), new Test.A1("a3"), new Test.A1("a4"));
    }

    setG(theG:Test.G, current:Ice.Current):void
    {
    }

    setI(theI:Ice.Value, current:Ice.Current):void
    {
    }

    opBaseSeq(inS:Test.BaseSeq, current:Ice.Current):[Test.BaseSeq, Test.BaseSeq]
    {
        return [inS, inS];
    }

    getCompact(current:Ice.Current):Test.Compact
    {
        return new Test.CompactExt();
    }

    getInnerA(current:Ice.Current):Test.Inner.A
    {
        return new Test.Inner.A(this._b1);
    }

    getInnerSubA(current:Ice.Current):Test.Inner.Sub.A
    {
        return new Test.Inner.Sub.A(new Test.Inner.A(this._b1));
    }

    throwInnerEx(current:Ice.Current):void
    {
        throw new Test.Inner.Ex("Inner::Ex");
    }

    throwInnerSubEx(current:Ice.Current):void
    {
        throw new Test.Inner.Sub.Ex("Inner::Sub::Ex");
    }

    shutdown(current:Ice.Current):void
    {
        current.adapter.getCommunicator().shutdown();
    }

    _b1:BI;
    _b2:BI;
    _c:CI;
    _d:DI;
    _e:EI;
    _f:FI;
}

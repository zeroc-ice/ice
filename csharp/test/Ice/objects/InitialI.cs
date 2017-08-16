// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using Test;

public sealed class InitialI : Initial
{
    public InitialI(Ice.ObjectAdapter adapter)
    {
        _adapter = adapter;
        _b1 = new BI();
        _b2 = new BI();
        _c = new CI();
        _d = new DI();
        _e = new EI();
        _f = new FI(_e);

        _b1.theA = _b2; // Cyclic reference to another B
        _b1.theB = _b1; // Self reference.
        _b1.theC = null; // Null reference.

        _b2.theA = _b2; // Self reference, using base.
        _b2.theB = _b1; // Cyclic reference to another B
        _b2.theC = _c; // Cyclic reference to a C.

        _c.theB = _b2; // Cyclic reference to a B.

        _d.theA = _b1; // Reference to a B.
        _d.theB = _b2; // Reference to a B.
        _d.theC = null; // Reference to a C.
    }

    public override void getAll(out B b1, out B b2, out C c, out D d, Ice.Current current)
    {
        b1 = _b1;
        b2 = _b2;
        c = _c;
        d = _d;
    }

    public override B getB1(Ice.Current current)
    {
        return _b1;
    }

    public override B getB2(Ice.Current current)
    {
        return _b2;
    }

    public override C getC(Ice.Current current)
    {
        return _c;
    }

    public override D getD(Ice.Current current)
    {
        return _d;
    }

    public override E getE(Ice.Current current)
    {
        return _e;
    }

    public override F getF(Ice.Current current)
    {
        return _f;
    }

    public override I getI(Ice.Current current)
    {
        return new II();
    }

    public override I getJ(Ice.Current current)
    {
        return new JI();
    }

    public override I getH(Ice.Current current)
    {
        return new HI();
    }

    public override D1 getD1(D1 d1, Ice.Current current)
    {
        return d1;
    }

    public override void setRecursive(Recursive r, Ice.Current current)
    {
    }

    public override bool supportsClassGraphDepthMax(Ice.Current current)
    {
        return true;
    }

    public override void throwEDerived(Ice.Current current)
    {
        throw new EDerived(new A1("a1"), new A1("a2"), new A1("a3"), new A1("a4"));
    }

    public override void setI(I theI, Ice.Current current)
    {
    }

    public override Base[] opBaseSeq(Base[] inS, out Base[] outS, Ice.Current current)
    {
        outS = inS;
        return inS;
    }

    public override Compact getCompact(Ice.Current current)
    {
        return new CompactExt();
    }

    public override void shutdown(Ice.Current current)
    {
        _adapter.getCommunicator().shutdown();
    }

    public override Test.Inner.A
    getInnerA(Ice.Current current)
    {
        return new Test.Inner.A(_b1);
    }

    public override Test.Inner.Sub.A
    getInnerSubA(Ice.Current current)
    {
        return new Test.Inner.Sub.A(new Test.Inner.A(_b1));
    }

    public override void throwInnerEx(Ice.Current current)
    {
        throw new Test.Inner.Ex("Inner::Ex");
    }

    public override void throwInnerSubEx(Ice.Current current)
    {
        throw new Test.Inner.Sub.Ex("Inner::Sub::Ex");
    }

    private Ice.ObjectAdapter _adapter;
    private B _b1;
    private B _b2;
    private C _c;
    private D _d;
    private E _e;
    private F _f;
}

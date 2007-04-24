// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
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

    public override void setI(I theI, Ice.Current current)
    {
    }
    
    public override void shutdown(Ice.Current current)
    {
        _adapter.getCommunicator().shutdown();
    }
    
    private Ice.ObjectAdapter _adapter;
    private B _b1;
    private B _b2;
    private C _c;
    private D _d;
}

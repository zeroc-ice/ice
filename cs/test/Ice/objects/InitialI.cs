// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

public sealed class InitialI : Initial
{
    public InitialI(Ice.ObjectAdapter adapter)
    {
        _adapter = adapter;
        _b1 = new B();
        _b2 = new B();
        _c = new C();
        _d = new D();
        
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
    
    public override void addFacetsToB1(Ice.Current current)
    {
        _b1.ice_addFacet(_b1, "b1");
        _b1.ice_addFacet(_b2, "b2");
        _b2.ice_addFacet(_c, "c");
        _b2.ice_addFacet(_d, "d");
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
    
    public override void shutdown(Ice.Current current)
    {
        _adapter.getCommunicator().shutdown();
        
        //
        // Break cyclic dependencies
        //
        _b1.theA = null;
        _b1.theB = null;
        _b1.theC = null;
        _b1.ice_removeAllFacets();
        _b2.theA = null;
        _b2.theB = null;
        _b2.theC = null;
        _b2.ice_removeAllFacets();
        _c.theB = null;
        _c.ice_removeAllFacets();
        _d.theA = null;
        _d.theB = null;
        _d.theC = null;
        _d.ice_removeAllFacets();
    }
    
    private Ice.ObjectAdapter _adapter;
    private B _b1;
    private B _b2;
    private C _c;
    private D _d;
}

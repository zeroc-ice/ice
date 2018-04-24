// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.objects;

import test.Ice.objects.Test.*;

import java.util.concurrent.CompletionStage;
import java.util.concurrent.CompletableFuture;

public final class InitialI implements Initial
{
    public InitialI(com.zeroc.Ice.ObjectAdapter adapter)
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

    @Override
    public Initial.GetAllResult getAll(com.zeroc.Ice.Current current)
    {
        Initial.GetAllResult r = new Initial.GetAllResult();
        _b1.preMarshalInvoked = false;
        _b2.preMarshalInvoked = false;
        _c.preMarshalInvoked = false;
        _d.preMarshalInvoked = false;
        r.b1 = _b1;
        r.b2 = _b2;
        r.theC = _c;
        r.theD = _d;
        return r;
    }

    @Override
    public B getB1(com.zeroc.Ice.Current current)
    {
        _b1.preMarshalInvoked = false;
        _b2.preMarshalInvoked = false;
        _c.preMarshalInvoked = false;
        return _b1;
    }

    @Override
    public B getB2(com.zeroc.Ice.Current current)
    {
        _b1.preMarshalInvoked = false;
        _b2.preMarshalInvoked = false;
        _c.preMarshalInvoked = false;
        return _b2;
    }

    @Override
    public C getC(com.zeroc.Ice.Current current)
    {
        _b1.preMarshalInvoked = false;
        _b2.preMarshalInvoked = false;
        _c.preMarshalInvoked = false;
        return _c;
    }

    @Override
    public D getD(com.zeroc.Ice.Current current)
    {
        _b1.preMarshalInvoked = false;
        _b2.preMarshalInvoked = false;
        _c.preMarshalInvoked = false;
        _d.preMarshalInvoked = false;
        return _d;
    }

    @Override
    public E getE(com.zeroc.Ice.Current current)
    {
        return _e;
    }

    @Override
    public F getF(com.zeroc.Ice.Current current)
    {
        return _f;
    }

    @Override
    public void setRecursive(Recursive r, com.zeroc.Ice.Current current)
    {
    }

    @Override
    public boolean supportsClassGraphDepthMax(com.zeroc.Ice.Current current)
    {
        return false;
    }

    @Override
    public com.zeroc.Ice.Value getI(com.zeroc.Ice.Current current)
    {
        return new II();
    }

    @Override
    public com.zeroc.Ice.Value getJ(com.zeroc.Ice.Current current)
    {
        return new JI();
    }

    @Override
    public com.zeroc.Ice.Value getH(com.zeroc.Ice.Current current)
    {
        return new HI();
    }

    @Override
    public D1 getD1(D1 d1, com.zeroc.Ice.Current current)
    {
        return d1;
    }

    @Override
    public GetMBMarshaledResult getMB(com.zeroc.Ice.Current current)
    {
        return new GetMBMarshaledResult(_b1, current);
    }

    @Override
    public CompletionStage<GetAMDMBMarshaledResult> getAMDMBAsync(com.zeroc.Ice.Current current)
    {
        return CompletableFuture.completedFuture(new GetAMDMBMarshaledResult(_b1, current));
    }

    @Override
    public void throwEDerived(com.zeroc.Ice.Current current) throws EDerived
    {
        throw new EDerived(new A1("a1"), new A1("a2"), new A1("a3"), new A1("a4"));
    }

    @Override
    public void setG(G theG, com.zeroc.Ice.Current current)
    {
    }

    @Override
    public void setI(com.zeroc.Ice.Value theI, com.zeroc.Ice.Current current)
    {
    }

    @Override
    public Initial.OpBaseSeqResult opBaseSeq(Base[] inS, com.zeroc.Ice.Current current)
    {
        return new Initial.OpBaseSeqResult(inS, inS);
    }

    @Override
    public Compact getCompact(com.zeroc.Ice.Current current)
    {
        return new CompactExt();
    }

    @Override
    public test.Ice.objects.Test.Inner.A getInnerA(com.zeroc.Ice.Current current)
    {
        return new test.Ice.objects.Test.Inner.A(_b1);
    }

    @Override
    public test.Ice.objects.Test.Inner.Sub.A getInnerSubA(com.zeroc.Ice.Current current)
    {
        return new test.Ice.objects.Test.Inner.Sub.A(new test.Ice.objects.Test.Inner.A(_b1));
    }

    @Override
    public void throwInnerEx(com.zeroc.Ice.Current current)
        throws test.Ice.objects.Test.Inner.Ex
    {
        throw new test.Ice.objects.Test.Inner.Ex("Inner::Ex");
    }

    @Override
    public void throwInnerSubEx(com.zeroc.Ice.Current current)
        throws test.Ice.objects.Test.Inner.Sub.Ex
    {
        throw new test.Ice.objects.Test.Inner.Sub.Ex("Inner::Sub::Ex");
    }

    @Override
    public void shutdown(com.zeroc.Ice.Current current)
    {
        _adapter.getCommunicator().shutdown();
    }

    private com.zeroc.Ice.ObjectAdapter _adapter;
    private B _b1;
    private B _b2;
    private C _c;
    private D _d;
    private E _e;
    private F _f;
}

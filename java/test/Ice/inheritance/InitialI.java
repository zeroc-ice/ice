// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

public final class InitialI extends _InitialDisp
{
    public
    InitialI(Ice.ObjectAdapter adapter)
    {
        _adapter = adapter;
        _ca = M_A.C_APrxHelper.uncheckedCast(
            _adapter.addWithUUID(new C_A_I()));
        _cb = M_B.C_BPrxHelper.uncheckedCast(
            _adapter.addWithUUID(new C_B_I()));
        _cc = M_A.C_CPrxHelper.uncheckedCast(
            _adapter.addWithUUID(new C_C_I()));
        _cd = M_A.C_DPrxHelper.uncheckedCast(
            _adapter.addWithUUID(new C_D_I()));
        _ia = M_A.I_APrxHelper.uncheckedCast(
            _adapter.addWithUUID(new I_A_I()));
        _ib1 = M_B.I_B1PrxHelper.uncheckedCast(
            _adapter.addWithUUID(new I_B1_I()));
        _ib2 = M_B.I_B2PrxHelper.uncheckedCast(
            _adapter.addWithUUID(new I_B2_I()));
        _ic = M_A.I_CPrxHelper.uncheckedCast(
            _adapter.addWithUUID(new I_C_I()));
    }

    public M_A.C_APrx
    c_a(Ice.Current current)
    {
        return _ca;
    }

    public M_B.C_BPrx
    c_b(Ice.Current current)
    {
        return _cb;
    }

    public M_A.C_CPrx
    c_c(Ice.Current current)
    {
        return _cc;
    }

    public M_A.C_DPrx
    c_d(Ice.Current current)
    {
        return _cd;
    }

    public M_A.I_APrx
    i_a(Ice.Current current)
    {
        return _ia;
    }

    public M_B.I_B1Prx
    i_b1(Ice.Current current)
    {
        return _ib1;
    }

    public M_B.I_B2Prx
    i_b2(Ice.Current current)
    {
        return _ib2;
    }

    public M_A.I_CPrx
    i_c(Ice.Current current)
    {
        return _ic;
    }

    public void
    shutdown(Ice.Current current)
    {
        _adapter.getCommunicator().shutdown();
    }

    private Ice.ObjectAdapter _adapter;
    private M_A.C_APrx _ca;
    private M_B.C_BPrx _cb;
    private M_A.C_CPrx _cc;
    private M_A.C_DPrx _cd;
    private M_A.I_APrx _ia;
    private M_B.I_B1Prx _ib1;
    private M_B.I_B2Prx _ib2;
    private M_A.I_CPrx _ic;
}

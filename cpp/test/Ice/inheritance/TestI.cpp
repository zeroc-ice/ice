// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestI.h>

M_A::C_APrx
C_A_I::ca(const M_A::C_APrx& p, const Ice::Current&)
{
    return p;
}

M_B::C_BPrx
C_B_I::cb(const M_B::C_BPrx& p, const Ice::Current&)
{
    return p;
}

M_A::C_CPrx
C_C_I::cc(const M_A::C_CPrx& p, const Ice::Current&)
{
    return p;
}

M_A::C_DPrx
C_D_I::cd(const M_A::C_DPrx& p, const Ice::Current&)
{
    return p;
}

M_A::I_APrx
I_A_I::ia(const M_A::I_APrx& p, const Ice::Current&)
{
    return p;
}

M_B::I_B1Prx
I_B1_I::ib1(const M_B::I_B1Prx& p, const Ice::Current&)
{
    return p;
}

M_B::I_B2Prx
I_B2_I::ib2(const M_B::I_B2Prx& p, const Ice::Current&)
{
    return p;
}

M_A::I_CPrx
I_C_I::ic(const M_A::I_CPrx& p, const Ice::Current&)
{
    return p;
}

InitialI::InitialI(const Ice::ObjectAdapterPtr& adapter) :
    _adapter(adapter)
{
    _ca = M_A::C_APrx::uncheckedCast(_adapter->addTemporary(new C_A_I));
    _cb = M_B::C_BPrx::uncheckedCast(_adapter->addTemporary(new C_B_I));
    _cc = M_A::C_CPrx::uncheckedCast(_adapter->addTemporary(new C_C_I));
    _cd = M_A::C_DPrx::uncheckedCast(_adapter->addTemporary(new C_D_I));
    _ia = M_A::I_APrx::uncheckedCast(_adapter->addTemporary(new I_A_I));
    _ib1 = M_B::I_B1Prx::uncheckedCast(_adapter->addTemporary(new I_B1_I));
    _ib2 = M_B::I_B2Prx::uncheckedCast(_adapter->addTemporary(new I_B2_I));
    _ic = M_A::I_CPrx::uncheckedCast(_adapter->addTemporary(new I_C_I));
}

void
InitialI::shutdown(const Ice::Current&)
{
    _adapter->getCommunicator()->shutdown();
}

M_A::C_APrx
InitialI::c_a(const Ice::Current&)
{
    return _ca;
}

M_B::C_BPrx
InitialI::c_b(const Ice::Current&)
{
    return _cb;
}

M_A::C_CPrx
InitialI::c_c(const Ice::Current&)
{
    return _cc;
}

M_A::C_DPrx
InitialI::c_d(const Ice::Current&)
{
    return _cd;
}

M_A::I_APrx
InitialI::i_a(const Ice::Current&)
{
    return _ia;
}

M_B::I_B1Prx
InitialI::i_b1(const Ice::Current&)
{
    return _ib1;
}

M_B::I_B2Prx
InitialI::i_b2(const Ice::Current&)
{
    return _ib2;
}

M_A::I_CPrx
InitialI::i_c(const Ice::Current&)
{
    return _ic;
}

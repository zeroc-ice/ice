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
C_A_I::ca(const M_A::C_APrx& p)
{
    return p;
}

M_B::C_BPrx
C_B_I::cb(const M_B::C_BPrx& p)
{
    return p;
}

M_A::C_CPrx
C_C_I::cc(const M_A::C_CPrx& p)
{
    return p;
}

M_A::C_DPrx
C_D_I::cd(const M_A::C_DPrx& p)
{
    return p;
}

M_A::I_APrx
I_A_I::ia(const M_A::I_APrx& p)
{
    return p;
}

M_B::I_B1Prx
I_B1_I::ib1(const M_B::I_B1Prx& p)
{
    return p;
}

M_B::I_B2Prx
I_B2_I::ib2(const M_B::I_B2Prx& p)
{
    return p;
}

M_A::I_CPrx
I_C_I::ic(const M_A::I_CPrx& p)
{
    return p;
}

InitialI::InitialI(const Ice::ObjectAdapterPtr& adapter) :
    _adapter(adapter)
{
    Ice::ObjectPtr cai = new C_A_I;
    _adapter->addTemporary(cai);
    _ca = M_A::C_APrx::uncheckedCast(_adapter->objectToProxy(cai));

    Ice::ObjectPtr cbi = new C_B_I;
    _adapter->addTemporary(cbi);
    _cb = M_B::C_BPrx::uncheckedCast(_adapter->objectToProxy(cbi));

    Ice::ObjectPtr cci = new C_C_I;
    _adapter->addTemporary(cci);
    _cc = M_A::C_CPrx::uncheckedCast(_adapter->objectToProxy(cci));

    Ice::ObjectPtr cdi = new C_D_I;
    _adapter->addTemporary(cdi);
    _cd = M_A::C_DPrx::uncheckedCast(_adapter->objectToProxy(cdi));

    Ice::ObjectPtr iai = new I_A_I;
    _adapter->addTemporary(iai);
    _ia = M_A::I_APrx::uncheckedCast(_adapter->objectToProxy(iai));

    Ice::ObjectPtr ib1i = new I_B1_I;
    _adapter->addTemporary(ib1i);
    _ib1 = M_B::I_B1Prx::uncheckedCast(_adapter->objectToProxy(ib1i));

    Ice::ObjectPtr ib2i = new I_B2_I;
    _adapter->addTemporary(ib2i);
    _ib2 = M_B::I_B2Prx::uncheckedCast(_adapter->objectToProxy(ib2i));

    Ice::ObjectPtr ici = new I_C_I;
    _adapter->addTemporary(ici);
    _ic = M_A::I_CPrx::uncheckedCast(_adapter->objectToProxy(ici));
}

void
InitialI::shutdown()
{
    _adapter->getCommunicator()->shutdown();
}

M_A::C_APrx
InitialI::c_a()
{
    return _ca;
}

M_B::C_BPrx
InitialI::c_b()
{
    return _cb;
}

M_A::C_CPrx
InitialI::c_c()
{
    return _cc;
}

M_A::C_DPrx
InitialI::c_d()
{
    return _cd;
}

M_A::I_APrx
InitialI::i_a()
{
    return _ia;
}

M_B::I_B1Prx
InitialI::i_b1()
{
    return _ib1;
}

M_B::I_B2Prx
InitialI::i_b2()
{
    return _ib2;
}

M_A::I_CPrx
InitialI::i_c()
{
    return _ic;
}

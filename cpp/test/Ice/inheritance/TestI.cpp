// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestI.h>

using namespace Test;

MA::CAPrx
CAI_::caop(const MA::CAPrx& p, const Ice::Current&)
{
    return p;
}

MB::CBPrx
CBI::cbop(const MB::CBPrx& p, const Ice::Current&)
{
    return p;
}

MA::CCPrx
CCI::ccop(const MA::CCPrx& p, const Ice::Current&)
{
    return p;
}

MA::CDPrx
CDI::cdop(const MA::CDPrx& p, const Ice::Current&)
{
    return p;
}

MA::IAPrx
IAI::iaop(const MA::IAPrx& p, const Ice::Current&)
{
    return p;
}

MB::IB1Prx
IB1I::ib1op(const MB::IB1Prx& p, const Ice::Current&)
{
    return p;
}

MB::IB2Prx
IB2I::ib2op(const MB::IB2Prx& p, const Ice::Current&)
{
    return p;
}

MA::ICPrx
ICI::icop(const MA::ICPrx& p, const Ice::Current&)
{
    return p;
}

InitialI::InitialI(const Ice::ObjectAdapterPtr& adapter)
{
    _ca = MA::CAPrx::uncheckedCast(adapter->addWithUUID(new CAI_));
    _cb = MB::CBPrx::uncheckedCast(adapter->addWithUUID(new CBI));
    _cc = MA::CCPrx::uncheckedCast(adapter->addWithUUID(new CCI));
    _cd = MA::CDPrx::uncheckedCast(adapter->addWithUUID(new CDI));
    _ia = MA::IAPrx::uncheckedCast(adapter->addWithUUID(new IAI));
    _ib1 = MB::IB1Prx::uncheckedCast(adapter->addWithUUID(new IB1I));
    _ib2 = MB::IB2Prx::uncheckedCast(adapter->addWithUUID(new IB2I));
    _ic = MA::ICPrx::uncheckedCast(adapter->addWithUUID(new ICI));
}

void
InitialI::shutdown(const Ice::Current& current)
{
    current.adapter->getCommunicator()->shutdown();
}

MA::CAPrx
InitialI::caop(const Ice::Current&)
{
    return _ca;
}

MB::CBPrx
InitialI::cbop(const Ice::Current&)
{
    return _cb;
}

MA::CCPrx
InitialI::ccop(const Ice::Current&)
{
    return _cc;
}

MA::CDPrx
InitialI::cdop(const Ice::Current&)
{
    return _cd;
}

MA::IAPrx
InitialI::iaop(const Ice::Current&)
{
    return _ia;
}

MB::IB1Prx
InitialI::ib1op(const Ice::Current&)
{
    return _ib1;
}

MB::IB2Prx
InitialI::ib2op(const Ice::Current&)
{
    return _ib2;
}

MA::ICPrx
InitialI::icop(const Ice::Current&)
{
    return _ic;
}

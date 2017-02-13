// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestI.h>

using namespace Test;

MA::CAPrxPtr
CAI_::caop(ICE_IN(MA::CAPrxPtr) p, const Ice::Current&)
{
    return p;
}

MB::CBPrxPtr
CBI::cbop(ICE_IN(MB::CBPrxPtr) p, const Ice::Current&)
{
    return p;
}

MA::CCPrxPtr
CCI::ccop(ICE_IN(MA::CCPrxPtr) p, const Ice::Current&)
{
    return p;
}

MA::CDPrxPtr
CDI::cdop(ICE_IN(MA::CDPrxPtr) p, const Ice::Current&)
{
    return p;
}

MA::IAPrxPtr
IAI::iaop(ICE_IN(MA::IAPrxPtr) p, const Ice::Current&)
{
    return p;
}

MB::IB1PrxPtr
IB1I::ib1op(ICE_IN(MB::IB1PrxPtr) p, const Ice::Current&)
{
    return p;
}

MB::IB2PrxPtr
IB2I::ib2op(ICE_IN(MB::IB2PrxPtr) p, const Ice::Current&)
{
    return p;
}

MA::ICPrxPtr
ICI::icop(ICE_IN(MA::ICPrxPtr) p, const Ice::Current&)
{
    return p;
}

InitialI::InitialI(const Ice::ObjectAdapterPtr& adapter)
{
    _ca = ICE_UNCHECKED_CAST(MA::CAPrx, adapter->addWithUUID(ICE_MAKE_SHARED(CAI_)));
    _cb = ICE_UNCHECKED_CAST(MB::CBPrx, adapter->addWithUUID(ICE_MAKE_SHARED(CBI)));
    _cc = ICE_UNCHECKED_CAST(MA::CCPrx, adapter->addWithUUID(ICE_MAKE_SHARED(CCI)));
    _cd = ICE_UNCHECKED_CAST(MA::CDPrx, adapter->addWithUUID(ICE_MAKE_SHARED(CDI)));
    _ia = ICE_UNCHECKED_CAST(MA::IAPrx, adapter->addWithUUID(ICE_MAKE_SHARED(IAI)));
    _ib1 = ICE_UNCHECKED_CAST(MB::IB1Prx, adapter->addWithUUID(ICE_MAKE_SHARED(IB1I)));
    _ib2 = ICE_UNCHECKED_CAST(MB::IB2Prx, adapter->addWithUUID(ICE_MAKE_SHARED(IB2I)));
    _ic = ICE_UNCHECKED_CAST(MA::ICPrx, adapter->addWithUUID(ICE_MAKE_SHARED(ICI)));
}

void
InitialI::shutdown(const Ice::Current& current)
{
    current.adapter->getCommunicator()->shutdown();
}

MA::CAPrxPtr
InitialI::caop(const Ice::Current&)
{
    return _ca;
}

MB::CBPrxPtr
InitialI::cbop(const Ice::Current&)
{
    return _cb;
}

MA::CCPrxPtr
InitialI::ccop(const Ice::Current&)
{
    return _cc;
}

MA::CDPrxPtr
InitialI::cdop(const Ice::Current&)
{
    return _cd;
}

MA::IAPrxPtr
InitialI::iaop(const Ice::Current&)
{
    return _ia;
}

MB::IB1PrxPtr
InitialI::ib1op(const Ice::Current&)
{
    return _ib1;
}

MB::IB2PrxPtr
InitialI::ib2op(const Ice::Current&)
{
    return _ib2;
}

MA::ICPrxPtr
InitialI::icop(const Ice::Current&)
{
    return _ic;
}

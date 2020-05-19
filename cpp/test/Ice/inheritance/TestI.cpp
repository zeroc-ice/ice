//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
#include <TestI.h>

using namespace Test;

MA::IAPrxPtr
IAI::iaop(MA::IAPrxPtr p, const Ice::Current&)
{
    return p;
}

MB::IB1PrxPtr
IB1I::ib1op(MB::IB1PrxPtr p, const Ice::Current&)
{
    return p;
}

MB::IB2PrxPtr
IB2I::ib2op(MB::IB2PrxPtr p, const Ice::Current&)
{
    return p;
}

MA::ICPrxPtr
ICI::icop(MA::ICPrxPtr p, const Ice::Current&)
{
    return p;
}

InitialI::InitialI(const Ice::ObjectAdapterPtr& adapter)
{
    _ia = ICE_UNCHECKED_CAST(MA::IAPrx, adapter->addWithUUID(std::make_shared<IAI>()));
    _ib1 = ICE_UNCHECKED_CAST(MB::IB1Prx, adapter->addWithUUID(std::make_shared<IB1I>()));
    _ib2 = ICE_UNCHECKED_CAST(MB::IB2Prx, adapter->addWithUUID(std::make_shared<IB2I>()));
    _ic = ICE_UNCHECKED_CAST(MA::ICPrx, adapter->addWithUUID(std::make_shared<ICI>()));
}

void
InitialI::shutdown(const Ice::Current& current)
{
    current.adapter->getCommunicator()->shutdown();
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

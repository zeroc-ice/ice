// Copyright (c) ZeroC, Inc.

#include "TestI.h"
#include "Ice/Ice.h"

using namespace std;
using namespace Ice;
using namespace Test;

optional<MA::IAPrx>
IAI::iaop(optional<MA::IAPrx> p, const Current&)
{
    return p;
}

optional<MB::IB1Prx>
IB1I::ib1op(optional<MB::IB1Prx> p, const Current&)
{
    return p;
}

optional<MB::IB2Prx>
IB2I::ib2op(optional<MB::IB2Prx> p, const Current&)
{
    return p;
}

optional<MA::ICPrx>
ICI::icop(optional<MA::ICPrx> p, const Current&)
{
    return p;
}

InitialI::InitialI(const ObjectAdapterPtr& adapter)
{
    _ia = adapter->addWithUUID<MA::IAPrx>(std::make_shared<IAI>());
    _ib1 = adapter->addWithUUID<MB::IB1Prx>(std::make_shared<IB1I>());
    _ib2 = adapter->addWithUUID<MB::IB2Prx>(std::make_shared<IB2I>());
    _ic = adapter->addWithUUID<MA::ICPrx>(std::make_shared<ICI>());
}

void
InitialI::shutdown(const Current& current)
{
    current.adapter->getCommunicator()->shutdown();
}

optional<MA::IAPrx>
InitialI::iaop(const Current&)
{
    return _ia;
}

optional<MB::IB1Prx>
InitialI::ib1op(const Current&)
{
    return _ib1;
}

optional<MB::IB2Prx>
InitialI::ib2op(const Current&)
{
    return _ib2;
}

optional<MA::ICPrx>
InitialI::icop(const Current&)
{
    return _ic;
}

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

std::string
AI::callA()
{
    return "A";
}

std::string
BI::callB()
{
    return "B";
}

std::string
CI::callC()
{
    return "C";
}

std::string
DI::callD()
{
    return "D";
}

std::string
EI::callE()
{
    return "E";
}

std::string
FI::callF()
{
    return "F";
}

GI::GI(const Ice::CommunicatorPtr& communicator) :
    _communicator(communicator)
{
}

void
GI::shutdown()
{
    _communicator->shutdown();
}

std::string
GI::callG()
{
    return "G";
}

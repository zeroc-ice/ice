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

#include <Ice/Ice.h>
#include <TestI.h>

std::string
AI::callA(const Ice::Current&)
{
    return "A";
}

std::string
BI::callB(const Ice::Current&)
{
    return "B";
}

std::string
CI::callC(const Ice::Current&)
{
    return "C";
}

std::string
DI::callD(const Ice::Current&)
{
    return "D";
}

std::string
EI::callE(const Ice::Current&)
{
    return "E";
}

std::string
FI::callF(const Ice::Current&)
{
    return "F";
}

GI::GI(const Ice::CommunicatorPtr& communicator) :
    _communicator(communicator)
{
}

void
GI::shutdown(const Ice::Current&)
{
    _communicator->shutdown();
}

std::string
GI::callG(const Ice::Current&)
{
    return "G";
}

HI::HI(const Ice::CommunicatorPtr& communicator) :
    GI(communicator)
{
}

std::string
HI::callH(const Ice::Current&)
{
    return "H";
}

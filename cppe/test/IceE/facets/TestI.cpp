// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/IceE.h>
#include <TestCommon.h>
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
#ifdef _WIN32_WCE
    tprintf("The server has shutdown, close the window to terminate the server.");
#endif
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

// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/IceE.h>
#include <TestI.h>

std::string
AI::callA(const IceE::Current&)
{
    return "A";
}

std::string
BI::callB(const IceE::Current&)
{
    return "B";
}

std::string
CI::callC(const IceE::Current&)
{
    return "C";
}

std::string
DI::callD(const IceE::Current&)
{
    return "D";
}

std::string
EI::callE(const IceE::Current&)
{
    return "E";
}

std::string
FI::callF(const IceE::Current&)
{
    return "F";
}

GI::GI(const IceE::CommunicatorPtr& communicator) :
    _communicator(communicator)
{
}

void
GI::shutdown(const IceE::Current&)
{
    _communicator->shutdown();
}

std::string
GI::callG(const IceE::Current&)
{
    return "G";
}

HI::HI(const IceE::CommunicatorPtr& communicator) :
    GI(communicator)
{
}

std::string
HI::callH(const IceE::Current&)
{
    return "H";
}

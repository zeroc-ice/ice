// **********************************************************************
//
// Copyright (c) 2001
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

TestI::TestI(const Ice::ObjectAdapterPtr& adapter) :
    _adapter(adapter)
{
}

void
TestI::shutdown(const Ice::Current&)
{
    _adapter->getCommunicator()->shutdown();
}

void
TestI::abort(const Ice::Current&)
{
    exit(0);
}

void
TestI::idempotentAbort(const Ice::Current&)
{
    exit(0);
}

void
TestI::nonmutatingAbort(const Ice::Current&) const
{
    exit(0);
}

Ice::Int
TestI::pid(const Ice::Current&)
{
#ifdef _WIN32
    return _getpid();
#else
    return getpid();
#endif
}

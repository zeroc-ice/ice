// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/IceE.h>
#include <TestCommon.h>
#include <TestI.h>

TestI::TestI(const Ice::ObjectAdapterPtr& adapter) :
    _adapter(adapter)
{
}

void
TestI::shutdown(const Ice::Current&)
{
    _adapter->getCommunicator()->shutdown();
#ifdef _WIN32_WCE
    tprintf("The server has shutdown, close the window to terminate the server.");
#endif
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
#if defined(_WIN32)
    return _getpid();
#else
    return getpid();
#endif
}

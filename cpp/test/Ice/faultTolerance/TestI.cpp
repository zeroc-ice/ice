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
TestI::nonmutatingAbort(const Ice::Current&)
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

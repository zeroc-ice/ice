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
TestI::shutdown()
{
    _adapter->getCommunicator()->shutdown();
}

void
TestI::abort()
{
    exit(1);
}

void
TestI::nonmutatingAbort()
{
    exit(1);
}

Ice::Int
TestI::pid()
{
#ifdef WIN32
    return _getpid();
#else
    return getpid();
#endif
}

// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestI.h>

TestI::TestI(const Ice::ObjectAdapterPtr& adapter, const CleanerPtr& cleaner) :
    _adapter(adapter),
    _cleaner(cleaner)
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
    ::_exit(0);
}

void
TestI::idempotentAbort(const Ice::Current&)
{
    ::_exit(0);
}

void
TestI::nonmutatingAbort(const Ice::Current&) const
{
    ::_exit(0);
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

CleanerI::CleanerI(const Ice::CommunicatorPtr& communicator)
{
    _communicator = communicator;
}

void
CleanerI::cleanup(const Ice::Current&)
{
    if(_communicator)
    {
        _communicator->destroy();
	_communicator = 0;
    }
}

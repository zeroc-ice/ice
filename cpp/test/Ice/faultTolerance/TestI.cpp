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
    _exit(0); // We want an *immediate* abort, without doing _onexit or atexit stuff.

}

void
TestI::idempotentAbort(const Ice::Current&)
{
    _exit(0); // We want an *immediate* abort, without doing _onexit or atexit stuff.
}

void
TestI::nonmutatingAbort(const Ice::Current&) const
{
    _exit(0); // We want an *immediate* abort, without doing _onexit or atexit stuff.
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

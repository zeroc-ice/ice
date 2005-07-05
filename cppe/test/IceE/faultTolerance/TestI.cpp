// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/IceE.h>
#include <TestCommon.h>
#include <TestI.h>

TestI::TestI(const IceE::ObjectAdapterPtr& adapter) :
    _adapter(adapter)
{
}

void
TestI::shutdown(const IceE::Current&)
{
    _adapter->getCommunicator()->shutdown();
}

void
TestI::abort(const IceE::Current&)
{
    exit(0);
}

void
TestI::idempotentAbort(const IceE::Current&)
{
    exit(0);
}

void
TestI::nonmutatingAbort(const IceE::Current&) const
{
    exit(0);
}

IceE::Int
TestI::pid(const IceE::Current&)
{
#if defined(_WIN32)
    return _getpid();
#else
    return getpid();
#endif
}

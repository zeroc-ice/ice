// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <TestI.h>
#include <Ice/Ice.h>

using namespace std;
using namespace Ice;

void
TestIntfI::op(const Ice::Current& current)
{
}

int
TestIntfI::opWithResult(const Ice::Current& current)
{
    return 15;
}

void
TestIntfI::opWithUE(const Ice::Current& current)
{
    throw Test::TestIntfException();
}

void
TestIntfI::opWithPayload(const Ice::ByteSeq&, const Ice::Current& current)
{
}

void
TestIntfI::shutdown(const Ice::Current& current)
{
    current.adapter->getCommunicator()->shutdown();
}

void
TestIntfControllerI::holdAdapter(const Ice::Current&)
{
    _adapter->hold();
}
    
void
TestIntfControllerI::resumeAdapter(const Ice::Current&)
{
    _adapter->activate();
}
    
TestIntfControllerI::TestIntfControllerI(const Ice::ObjectAdapterPtr& adapter) : _adapter(adapter)
{
}

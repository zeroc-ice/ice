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

TestI::TestI(const Ice::ObjectAdapterPtr& adapter, const Ice::ObjectPrx fwd) :
    _adapter(adapter),
    _fwd(fwd)
{
}

void
TestI::shutdown()
{
    _adapter->getCommunicator()->shutdown();
    if (_fwd)
    {
	throw Ice::LocationForward(_fwd);
    }
}

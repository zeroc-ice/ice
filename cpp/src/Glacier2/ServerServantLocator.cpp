// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Glacier2/ServerServantLocator.h>

using namespace std;
using namespace Ice;
using namespace Glacier;

Glacier::ServerServantLocator::ServerServantLocator(const ObjectAdapterPtr& clientAdapter) :
    _blobject(new ServerBlobject(clientAdapter))
{
}

ObjectPtr
Glacier::ServerServantLocator::locate(const Current& current, LocalObjectPtr&)
{
    return _blobject;
}

void
Glacier::ServerServantLocator::finished(const Current&, const ObjectPtr&, const LocalObjectPtr&)
{
    // Nothing to do
}

void
Glacier::ServerServantLocator::deactivate(const string&)
{
    assert(_blobject);
    _blobject->destroy();
    _blobject = 0;
}

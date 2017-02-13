// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <BackendI.h>

using namespace std;
using namespace Ice;
using namespace Test;

void
BackendI::shutdown(const Ice::Current& current)
{
    current.adapter->getCommunicator()->shutdown();
}

// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
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

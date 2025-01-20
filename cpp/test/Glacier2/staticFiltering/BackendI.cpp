// Copyright (c) ZeroC, Inc.

#include "BackendI.h"
#include "Ice/Ice.h"

using namespace std;
using namespace Ice;
using namespace Test;

void
BackendI::shutdown(const Ice::Current& current)
{
    current.adapter->getCommunicator()->shutdown();
}

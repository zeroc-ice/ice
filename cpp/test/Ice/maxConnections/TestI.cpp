// Copyright (c) ZeroC, Inc.

#include "TestI.h"

using namespace std;

void
TestIntfI::shutdown(const Ice::Current& current)
{
    current.adapter->getCommunicator()->shutdown();
}

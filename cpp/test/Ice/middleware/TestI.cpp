// Copyright (c) ZeroC, Inc.

#include "TestI.h"

using namespace std;

string
MyObjectI::getName(const Ice::Current&)
{
    return "Foo";
}

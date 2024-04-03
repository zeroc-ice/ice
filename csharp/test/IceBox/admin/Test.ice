//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef TEST_ICE
#define TEST_ICE

#include "Ice/PropertyDict.ice"

module Test
{

interface TestFacet
{
    Ice::PropertyDict getChanges();
}

}

#endif

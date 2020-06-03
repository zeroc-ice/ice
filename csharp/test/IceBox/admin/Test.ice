//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef TEST_ICE
#define TEST_ICE

#include <Ice/PropertiesAdmin.ice>

[[suppress-warning:reserved-identifier]]

module ZeroC::IceBox::Test::Admin
{

interface TestFacet
{
    Ice::PropertyDict getChanges();
}

}

#endif

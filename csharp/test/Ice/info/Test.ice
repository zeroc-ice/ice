//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

#include <Ice/Current.ice>

["cs:namespace:Ice.info"]
module Test
{

interface TestIntf
{
    void shutdown();

    Ice::Context getEndpointInfoAsContext();

    Ice::Context getConnectionInfoAsContext();
}

}

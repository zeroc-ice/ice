//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

#include <Ice/Context.ice>

module Test
{

interface TestIntf
{
    void shutdown();

    Ice::Context getEndpointInfoAsContext();

    Ice::Context getConnectionInfoAsContext();
}

}

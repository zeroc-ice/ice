//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

#include <Ice/Current.ice>

[["js:es6-module"]]

module Test
{

interface TestIntf
{
    void shutdown();

    Ice::Context getEndpointInfoAsContext();

    Ice::Context getConnectionInfoAsContext();
}

}

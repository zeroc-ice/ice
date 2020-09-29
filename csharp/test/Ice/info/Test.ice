//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

#include <Ice/RequestEncoding.ice>

[[suppress-warning:reserved-identifier]]

module ZeroC::Ice::Test::Info
{

interface TestIntf
{
    void shutdown();

    Ice::Context getEndpointInfoAsContext();

    Ice::Context getConnectionInfoAsContext();
}

}

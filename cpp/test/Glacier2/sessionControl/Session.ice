// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#pragma once

#include <Glacier2/Session.ice>

module Test
{

interface Session extends Glacier2::Session
{
    ["amd"] void destroyFromClient();

    void shutdown();
}

}

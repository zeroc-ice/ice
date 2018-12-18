// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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

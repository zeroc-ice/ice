//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

module Test
{

interface TestIntf
{
    void transient();

    void deactivate();
}

local class Cookie
{
    string message();
}

}

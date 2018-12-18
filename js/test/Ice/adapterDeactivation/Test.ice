// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

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
    ["cpp:const"] string message();
}

}

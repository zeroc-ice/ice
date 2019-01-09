// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#pragma once

["cs:namespace:Ice.adapterDeactivation"]
module Test
{

interface TestIntf
{
    void transient();
    void deactivate();
}

local class Cookie
{
    idempotent string message();
}

}

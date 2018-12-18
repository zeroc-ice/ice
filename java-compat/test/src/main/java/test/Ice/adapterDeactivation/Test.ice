// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#pragma once

[["java:package:test.Ice.adapterDeactivation"]]
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

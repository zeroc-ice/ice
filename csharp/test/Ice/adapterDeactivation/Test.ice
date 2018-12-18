// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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

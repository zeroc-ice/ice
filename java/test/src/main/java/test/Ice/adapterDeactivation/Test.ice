// Copyright (c) ZeroC, Inc.

#pragma once

[["java:package:test.Ice.adapterDeactivation"]]
module Test
{

interface TestIntf
{
    void transient();

    void deactivate();
}

}

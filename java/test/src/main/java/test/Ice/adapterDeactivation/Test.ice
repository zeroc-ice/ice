// Copyright (c) ZeroC, Inc.

#pragma once

[["java:package:test.Ice.adapterDeactivation"]]
module Test
{

interface TestIntf
{
    ["java:identifier:_transient"]
    void transient();

    void deactivate();
}

}

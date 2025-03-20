// Copyright (c) ZeroC, Inc.

#pragma once

[["java:package:test.Ice.adapterDeactivation"]]
module Test
{
    interface TestIntf
    {
        ["java:identifier:transientOp"]
        void transient();

        void deactivate();
    }
}

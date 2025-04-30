// Copyright (c) ZeroC, Inc.

#pragma once

["java:identifier:test.Ice.adapterDeactivation.Test"]
module Test
{
    interface TestIntf
    {
        ["java:identifier:transientOp"]
        void transient();

        void deactivate();
    }
}

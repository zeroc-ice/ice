// Copyright (c) ZeroC, Inc.

#pragma once

["cs:identifier:Ice.adapterDeactivation.Test"]
module Test
{
    interface TestIntf
    {
        void transient();
        void deactivate();
    }
}

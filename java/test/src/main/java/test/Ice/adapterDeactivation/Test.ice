//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

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

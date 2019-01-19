//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

["objc:prefix:TestAdapterDeactivation"]
module Test
{

interface TestIntf
{
    void transient();

    void deactivate();
}

}

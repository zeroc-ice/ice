//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

[cs:namespace:ZeroC.Ice.adapterDeactivation]
module Test
{

interface TestIntf
{
    void transient();
    void deactivate();
}

}

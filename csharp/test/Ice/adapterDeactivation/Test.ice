//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

module ZeroC::Ice::Test::AdapterDeactivation
{

interface TestIntf
{
    void transient();
    void deactivate();
}

}

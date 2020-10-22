//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

[[suppress-warning(reserved-identifier)]]

module ZeroC::Ice::Test::AdapterDeactivation
{

interface TestIntf
{
    void transient();
    void deactivate();
}

}

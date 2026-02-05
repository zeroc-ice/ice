// Copyright (c) ZeroC, Inc.

#pragma once

// Include from module/ which has js:module:test-nested-modules
// This file does NOT have js:module, so types from the included file
// should be imported from the external module, not aggregated.
#include "../module/First.ice"

module External
{
    // A struct that uses a type from the external module (test-nested-modules)
    struct UsesModuleType
    {
        Outer::Inner::First first;
    }
}

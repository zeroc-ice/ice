// Copyright (c) ZeroC, Inc.

#pragma once

#include "First.ice"
#include "Second.ice"
#include "Third.ice"
#include "Forward.ice"

module Outer
{
    module Inner
    {
        // Forward declaration with js:defined-in metadata.
        // ForwardDeclared is actually defined in Forward.ice.
        // This tests that the type is imported from Forward.ice, not double-exported.
        ["js:defined-in:./Forward.ice"]
        class ForwardDeclared;

        // Uses the forward declared type
        struct UsesForward
        {
            ForwardDeclared forward;
        }

        // Combined struct that uses types from both First.ice and Second.ice
        // This tests that both First and Second are accessible in the same nested module
        struct Combined
        {
            First first;
            Second second;
        }

        // Uses Third from Third.ice to ensure it's re-exported in TypeScript declarations
        struct UsesThird
        {
            Third third;
        }

        module Deep
        {
            // DeepCombined struct uses types from deep nested modules in both First.ice and Second.ice
            // This tests that multi-level nested module merging works correctly
            struct DeepCombined
            {
                DeepFirst deepFirst;
                DeepSecond deepSecond;
            }

            // Uses DeepThird from ThirdDeep.ice (via Third.ice) to test mixed direct + transitive overlap
            // Third.ice defines Outer.Inner.Third but NOT Outer.Inner.Deep.
            // Only ThirdDeep.ice (transitive) defines Outer.Inner.Deep.DeepThird.
            struct UsesDeepThird
            {
                DeepThird deepThird;
            }
        }
    }
}

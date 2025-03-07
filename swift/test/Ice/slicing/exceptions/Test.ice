// Copyright (c) ZeroC, Inc.
#pragma once
    
[["swift:class-resolver-prefix:IceSlicingExceptions"]]
module Test
{
    exception Base
    {
        string b;
    }
<<<<<<< Updated upstream

=======
        
>>>>>>> Stashed changes
    exception KnownDerived extends Base
    {
        string kd;
    }
<<<<<<< Updated upstream

=======
        
>>>>>>> Stashed changes
    exception KnownIntermediate extends Base
    {
        string ki;
    }
<<<<<<< Updated upstream

=======
        
>>>>>>> Stashed changes
    exception KnownMostDerived extends KnownIntermediate
    {
        string kmd;
    }
<<<<<<< Updated upstream

    interface TestIntf
    {
        void baseAsBase() throws Base;

        // Test that the compact metadata is ignored (exceptions are always encoded with the sliced format).
        ["format:compact"] void unknownDerivedAsBase() throws Base;

        void knownDerivedAsBase() throws Base;
        void knownDerivedAsKnownDerived() throws KnownDerived;

=======
        
    interface TestIntf
    {
        void baseAsBase() throws Base;
            
        // Test that the compact metadata is ignored (exceptions are always encoded with the sliced format).
        ["format:compact"] void unknownDerivedAsBase() throws Base;
            
        void knownDerivedAsBase() throws Base;
        void knownDerivedAsKnownDerived() throws KnownDerived;
            
>>>>>>> Stashed changes
        void unknownIntermediateAsBase() throws Base;
        void knownIntermediateAsBase() throws Base;
        void knownMostDerivedAsBase() throws Base;
        void knownIntermediateAsKnownIntermediate() throws KnownIntermediate;
        void knownMostDerivedAsKnownIntermediate() throws KnownIntermediate;
        void knownMostDerivedAsKnownMostDerived() throws KnownMostDerived;
<<<<<<< Updated upstream

        void unknownMostDerived1AsBase() throws Base;
        void unknownMostDerived1AsKnownIntermediate() throws KnownIntermediate;
        void unknownMostDerived2AsBase() throws Base;

=======
            
        void unknownMostDerived1AsBase() throws Base;
        void unknownMostDerived1AsKnownIntermediate() throws KnownIntermediate;
        void unknownMostDerived2AsBase() throws Base;
            
>>>>>>> Stashed changes
        void shutdown();
    }
}

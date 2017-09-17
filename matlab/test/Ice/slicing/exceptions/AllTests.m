%{
**********************************************************************

Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.

This copy of Ice is licensed to you under the terms described in the
ICE_LICENSE file included in this distribution.

**********************************************************************
%}

classdef AllTests
    methods(Static)
        function r = allTests(app)
            import test.Ice.slicing.exceptions.client.Test.*;

            communicator = app.communicator();

            fprintf('testing stringToProxy... ');
            ref = ['Test:', app.getTestEndpoint(0, ''), ' -t 10000'];
            base = communicator.stringToProxy(ref);
            assert(~isempty(base));
            fprintf('ok\n');

            fprintf('testing checked cast... ');
            test = TestIntfPrx.checkedCast(base);
            assert(~isempty(test));
            assert(test == base);
            fprintf('ok\n');

            fprintf('base... ');

            try
                test.baseAsBase();
                assert(false);
            catch ex
                if isa(ex, 'test.Ice.slicing.exceptions.client.Test.Base')
                    assert(strcmp(ex.b, 'Base.b'));
                    assert(strcmp(ex.ice_id(), '::Test::Base'));
                else
                    rethrow(ex);
                end
            end

            fprintf('ok\n');

            fprintf('base (AMI)... ');

            try
                test.baseAsBaseAsync().fetchOutputs();
                assert(false);
            catch ex
                if isa(ex, 'test.Ice.slicing.exceptions.client.Test.Base')
                    assert(strcmp(ex.b, 'Base.b'));
                    assert(strcmp(ex.ice_id(), '::Test::Base'));
                else
                    rethrow(ex);
                end
            end

            fprintf('ok\n');

            fprintf('slicing of unknown derived... ');

            try
                test.unknownDerivedAsBase();
                assert(false);
            catch ex
                if isa(ex, 'test.Ice.slicing.exceptions.client.Test.Base')
                    assert(strcmp(ex.b, 'UnknownDerived.b'));
                    assert(strcmp(ex.ice_id(), '::Test::Base'));
                else
                    rethrow(ex);
                end
            end

            fprintf('ok\n');

            fprintf('slicing of unknown derived (AMI)... ');

            try
                test.unknownDerivedAsBaseAsync().fetchOutputs();
                assert(false);
            catch ex
                if isa(ex, 'test.Ice.slicing.exceptions.client.Test.Base')
                    assert(strcmp(ex.b, 'UnknownDerived.b'));
                    assert(strcmp(ex.ice_id(), '::Test::Base'));
                else
                    rethrow(ex);
                end
            end

            fprintf('ok\n');

            fprintf('non-slicing of known derived as base... ');

            try
                test.knownDerivedAsBase();
                assert(false);
            catch ex
                if isa(ex, 'test.Ice.slicing.exceptions.client.Test.KnownDerived')
                    assert(strcmp(ex.b, 'KnownDerived.b'));
                    assert(strcmp(ex.kd, 'KnownDerived.kd'));
                    assert(strcmp(ex.ice_id(), '::Test::KnownDerived'));
                else
                    rethrow(ex);
                end
            end

            fprintf('ok\n');

            fprintf('non-slicing of known derived as base (AMI)... ');

            try
                test.knownDerivedAsBaseAsync().fetchOutputs();
                assert(false);
            catch ex
                if isa(ex, 'test.Ice.slicing.exceptions.client.Test.KnownDerived')
                    assert(strcmp(ex.b, 'KnownDerived.b'));
                    assert(strcmp(ex.kd, 'KnownDerived.kd'));
                    assert(strcmp(ex.ice_id(), '::Test::KnownDerived'));
                else
                    rethrow(ex);
                end
            end

            fprintf('ok\n');

            fprintf('non-slicing of known derived as derived... ');

            try
                test.knownDerivedAsKnownDerived();
                assert(false);
            catch ex
                if isa(ex, 'test.Ice.slicing.exceptions.client.Test.KnownDerived')
                    assert(strcmp(ex.b, 'KnownDerived.b'));
                    assert(strcmp(ex.kd, 'KnownDerived.kd'));
                    assert(strcmp(ex.ice_id(), '::Test::KnownDerived'));
                else
                    rethrow(ex);
                end
            end

            fprintf('ok\n');

            fprintf('non-slicing of known derived as derived (AMI)... ');

            try
                test.knownDerivedAsKnownDerivedAsync().fetchOutputs();
                assert(false);
            catch ex
                if isa(ex, 'test.Ice.slicing.exceptions.client.Test.KnownDerived')
                    assert(strcmp(ex.b, 'KnownDerived.b'));
                    assert(strcmp(ex.kd, 'KnownDerived.kd'));
                    assert(strcmp(ex.ice_id(), '::Test::KnownDerived'));
                else
                    rethrow(ex);
                end
            end

            fprintf('ok\n');

            fprintf('slicing of unknown intermediate as base... ');

            try
                test.unknownIntermediateAsBase();
                assert(false);
            catch ex
                if isa(ex, 'test.Ice.slicing.exceptions.client.Test.Base')
                    assert(strcmp(ex.b, 'UnknownIntermediate.b'));
                    assert(strcmp(ex.ice_id(), '::Test::Base'));
                else
                    rethrow(ex);
                end
            end

            fprintf('ok\n');

            fprintf('slicing of unknown intermediate as base (AMI)... ');

            try
                test.unknownIntermediateAsBaseAsync().fetchOutputs();
                assert(false);
            catch ex
                if isa(ex, 'test.Ice.slicing.exceptions.client.Test.Base')
                    assert(strcmp(ex.b, 'UnknownIntermediate.b'));
                    assert(strcmp(ex.ice_id(), '::Test::Base'));
                else
                    rethrow(ex);
                end
            end

            fprintf('ok\n');

            fprintf('slicing of known intermediate as base... ');

            try
                test.knownIntermediateAsBase();
                assert(false);
            catch ex
                if isa(ex, 'test.Ice.slicing.exceptions.client.Test.KnownIntermediate')
                    assert(strcmp(ex.b, 'KnownIntermediate.b'));
                    assert(strcmp(ex.ki, 'KnownIntermediate.ki'));
                    assert(strcmp(ex.ice_id(), '::Test::KnownIntermediate'));
                else
                    rethrow(ex);
                end
            end

            fprintf('ok\n');

            fprintf('slicing of known intermediate as base (AMI)... ');

            try
                test.knownIntermediateAsBaseAsync().fetchOutputs();
                assert(false);
            catch ex
                if isa(ex, 'test.Ice.slicing.exceptions.client.Test.KnownIntermediate')
                    assert(strcmp(ex.b, 'KnownIntermediate.b'));
                    assert(strcmp(ex.ki, 'KnownIntermediate.ki'));
                    assert(strcmp(ex.ice_id(), '::Test::KnownIntermediate'));
                else
                    rethrow(ex);
                end
            end

            fprintf('ok\n');

            fprintf('slicing of known most derived as base... ');

            try
                test.knownMostDerivedAsBase();
                assert(false);
            catch ex
                if isa(ex, 'test.Ice.slicing.exceptions.client.Test.KnownMostDerived')
                    assert(strcmp(ex.b, 'KnownMostDerived.b'));
                    assert(strcmp(ex.ki, 'KnownMostDerived.ki'));
                    assert(strcmp(ex.kmd, 'KnownMostDerived.kmd'));
                    assert(strcmp(ex.ice_id(), '::Test::KnownMostDerived'));
                else
                    rethrow(ex);
                end
            end

            fprintf('ok\n');

            fprintf('slicing of known most derived as base (AMI)... ');

            try
                test.knownMostDerivedAsBaseAsync().fetchOutputs();
                assert(false);
            catch ex
                if isa(ex, 'test.Ice.slicing.exceptions.client.Test.KnownMostDerived')
                    assert(strcmp(ex.b, 'KnownMostDerived.b'));
                    assert(strcmp(ex.ki, 'KnownMostDerived.ki'));
                    assert(strcmp(ex.kmd, 'KnownMostDerived.kmd'));
                    assert(strcmp(ex.ice_id(), '::Test::KnownMostDerived'));
                else
                    rethrow(ex);
                end
            end

            fprintf('ok\n');

            fprintf('non-slicing of known intermediate as intermediate... ');

            try
                test.knownIntermediateAsKnownIntermediate();
                assert(false);
            catch ex
                if isa(ex, 'test.Ice.slicing.exceptions.client.Test.KnownIntermediate')
                    assert(strcmp(ex.b, 'KnownIntermediate.b'));
                    assert(strcmp(ex.ki, 'KnownIntermediate.ki'));
                    assert(strcmp(ex.ice_id(), '::Test::KnownIntermediate'));
                else
                    rethrow(ex);
                end
            end

            fprintf('ok\n');

            fprintf('non-slicing of known intermediate as intermediate (AMI)... ');

            try
                test.knownIntermediateAsKnownIntermediateAsync().fetchOutputs();
                assert(false);
            catch ex
                if isa(ex, 'test.Ice.slicing.exceptions.client.Test.KnownIntermediate')
                    assert(strcmp(ex.b, 'KnownIntermediate.b'));
                    assert(strcmp(ex.ki, 'KnownIntermediate.ki'));
                    assert(strcmp(ex.ice_id(), '::Test::KnownIntermediate'));
                else
                    rethrow(ex);
                end
            end

            fprintf('ok\n');

            fprintf('non-slicing of known most derived as intermediate... ');

            try
                test.knownMostDerivedAsKnownIntermediate();
                assert(false);
            catch ex
                if isa(ex, 'test.Ice.slicing.exceptions.client.Test.KnownMostDerived')
                    assert(strcmp(ex.b, 'KnownMostDerived.b'));
                    assert(strcmp(ex.ki, 'KnownMostDerived.ki'));
                    assert(strcmp(ex.kmd, 'KnownMostDerived.kmd'));
                    assert(strcmp(ex.ice_id(), '::Test::KnownMostDerived'));
                else
                    rethrow(ex);
                end
            end

            fprintf('ok\n');

            fprintf('non-slicing of known most derived as intermediate (AMI)... ');

            try
                test.knownMostDerivedAsKnownIntermediateAsync().fetchOutputs();
                assert(false);
            catch ex
                if isa(ex, 'test.Ice.slicing.exceptions.client.Test.KnownMostDerived')
                    assert(strcmp(ex.b, 'KnownMostDerived.b'));
                    assert(strcmp(ex.ki, 'KnownMostDerived.ki'));
                    assert(strcmp(ex.kmd, 'KnownMostDerived.kmd'));
                    assert(strcmp(ex.ice_id(), '::Test::KnownMostDerived'));
                else
                    rethrow(ex);
                end
            end

            fprintf('ok\n');

            fprintf('non-slicing of known most derived as most derived... ');

            try
                test.knownMostDerivedAsKnownMostDerived();
                assert(false);
            catch ex
                if isa(ex, 'test.Ice.slicing.exceptions.client.Test.KnownMostDerived')
                    assert(strcmp(ex.b, 'KnownMostDerived.b'));
                    assert(strcmp(ex.ki, 'KnownMostDerived.ki'));
                    assert(strcmp(ex.kmd, 'KnownMostDerived.kmd'));
                    assert(strcmp(ex.ice_id(), '::Test::KnownMostDerived'));
                else
                    rethrow(ex);
                end
            end

            fprintf('ok\n');

            fprintf('non-slicing of known most derived as most derived (AMI)... ');

            try
                test.knownMostDerivedAsKnownMostDerivedAsync().fetchOutputs();
                assert(false);
            catch ex
                if isa(ex, 'test.Ice.slicing.exceptions.client.Test.KnownMostDerived')
                    assert(strcmp(ex.b, 'KnownMostDerived.b'));
                    assert(strcmp(ex.ki, 'KnownMostDerived.ki'));
                    assert(strcmp(ex.kmd, 'KnownMostDerived.kmd'));
                    assert(strcmp(ex.ice_id(), '::Test::KnownMostDerived'));
                else
                    rethrow(ex);
                end
            end

            fprintf('ok\n');

            fprintf('slicing of unknown most derived, known intermediate as base... ');

            try
                test.unknownMostDerived1AsBase();
                assert(false);
            catch ex
                if isa(ex, 'test.Ice.slicing.exceptions.client.Test.KnownIntermediate')
                    assert(strcmp(ex.b, 'UnknownMostDerived1.b'));
                    assert(strcmp(ex.ki, 'UnknownMostDerived1.ki'));
                    assert(strcmp(ex.ice_id(), '::Test::KnownIntermediate'));
                else
                    rethrow(ex);
                end
            end

            fprintf('ok\n');

            fprintf('slicing of unknown most derived, known intermediate as base (AMI)... ');

            try
                test.unknownMostDerived1AsBaseAsync().fetchOutputs();
                assert(false);
            catch ex
                if isa(ex, 'test.Ice.slicing.exceptions.client.Test.KnownIntermediate')
                    assert(strcmp(ex.b, 'UnknownMostDerived1.b'));
                    assert(strcmp(ex.ki, 'UnknownMostDerived1.ki'));
                    assert(strcmp(ex.ice_id(), '::Test::KnownIntermediate'));
                else
                    rethrow(ex);
                end
            end

            fprintf('ok\n');

            fprintf('slicing of unknown most derived, known intermediate as intermediate... ');

            try
                test.unknownMostDerived1AsKnownIntermediate();
                assert(false);
            catch ex
                if isa(ex, 'test.Ice.slicing.exceptions.client.Test.KnownIntermediate')
                    assert(strcmp(ex.b, 'UnknownMostDerived1.b'));
                    assert(strcmp(ex.ki, 'UnknownMostDerived1.ki'));
                    assert(strcmp(ex.ice_id(), '::Test::KnownIntermediate'));
                else
                    rethrow(ex);
                end
            end

            fprintf('ok\n');

            fprintf('slicing of unknown most derived, known intermediate as intermediate (AMI)... ');

            try
                test.unknownMostDerived1AsKnownIntermediateAsync().fetchOutputs();
                assert(false);
            catch ex
                if isa(ex, 'test.Ice.slicing.exceptions.client.Test.KnownIntermediate')
                    assert(strcmp(ex.b, 'UnknownMostDerived1.b'));
                    assert(strcmp(ex.ki, 'UnknownMostDerived1.ki'));
                    assert(strcmp(ex.ice_id(), '::Test::KnownIntermediate'));
                else
                    rethrow(ex);
                end
            end

            fprintf('ok\n');

            fprintf('slicing of unknown most derived, unknown intermediate thrown as base... ');

            try
                test.unknownMostDerived2AsBase();
                assert(false);
            catch ex
                if isa(ex, 'test.Ice.slicing.exceptions.client.Test.Base')
                    assert(strcmp(ex.b, 'UnknownMostDerived2.b'));
                    assert(strcmp(ex.ice_id(), '::Test::Base'));
                else
                    rethrow(ex);
                end
            end

            fprintf('ok\n');

            fprintf('slicing of unknown most derived, unknown intermediate thrown as base (AMI)... ');

            try
                test.unknownMostDerived2AsBaseAsync().fetchOutputs();
                assert(false);
            catch ex
                if isa(ex, 'test.Ice.slicing.exceptions.client.Test.Base')
                    assert(strcmp(ex.b, 'UnknownMostDerived2.b'));
                    assert(strcmp(ex.ice_id(), '::Test::Base'));
                else
                    rethrow(ex);
                end
            end

            fprintf('ok\n');

            fprintf('unknown most derived in compact format... ');

            try
                test.unknownMostDerived2AsBaseCompact();
                assert(false);
            catch ex
                if isa(ex, 'test.Ice.slicing.exceptions.client.Test.Base')
                    %
                    % For the 1.0 encoding, the unknown exception is sliced to Base.
                    %
                    test(test.ice_getEncodingVersion() == Ice.EncodingVersion(1, 0));
                elseif isa(ex, 'Ice.UnknownUserException')
                    %
                    % An UnknownUserException is raised for the compact format because the
                    % most-derived type is unknown and the exception cannot be sliced.
                    %
                    test(test.ice_getEncodingVersion() ~= Ice.EncodingVersion(1, 0));
                elseif isa(ex, 'Ice.OperationNotExistException')
                    % Ignore
                else
                    rethrow(ex);
                end
            end

            fprintf('ok\n');

            fprintf('preserved exceptions... ');

            try
                test.unknownPreservedAsBase();
                assert(false);
            catch ex
                if isa(ex, 'Ice.UserException')
                    if test.ice_getEncodingVersion() == Ice.EncodingVersion(1, 0)
                        assert(isempty(ex.ice_getSlicedData()));
                    else
                        slicedData = ex.ice_getSlicedData();
                        assert(~isempty(slicedData));
                        assert(length(slicedData.slices) == 2);
                        assert(strcmp(slicedData.slices{2}.typeId, '::Test::SPreserved1'));
                        assert(strcmp(slicedData.slices{1}.typeId, '::Test::SPreserved2'));
                    end
                else
                    rethrow(ex);
                end
            end

            try
                test.unknownPreservedAsKnownPreserved();
                assert(false);
            catch ex
                if isa(ex, 'test.Ice.slicing.exceptions.client.Test.KnownPreserved')
                    assert(strcmp(ex.kp, 'preserved'));
                    if test.ice_getEncodingVersion() == Ice.EncodingVersion(1, 0)
                        assert(isempty(ex.ice_getSlicedData()));
                    else
                        slicedData = ex.ice_getSlicedData();
                        assert(~isempty(slicedData));
                        assert(length(slicedData.slices) == 2);
                        assert(strcmp(slicedData.slices{2}.typeId, '::Test::SPreserved1'));
                        assert(strcmp(slicedData.slices{1}.typeId, '::Test::SPreserved2'));
                    end
                else
                    rethrow(ex);
                end
            end

            fprintf('ok\n');

            r = test;
        end
    end
end

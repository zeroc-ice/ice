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
            communicator = app.communicator();

            %{
                % TODO
                fprintf('testing value factory registration exception... ");
                com.zeroc.Ice.ValueFactory of = new ValueFactoryI();
                communicator.getValueFactoryManager().add(of, "::x");
                try
                {
                    communicator.getValueFactoryManager().add(of, "::x");
                    test(false);
                }
                catch(com.zeroc.Ice.AlreadyRegisteredException ex)
                {
                }
                fprintf('ok\n');
            %}

            fprintf('testing stringToProxy... ');
            ref = ['thrower:', app.getTestEndpoint(0, '')];
            base = communicator.stringToProxy(ref);
            assert(~isempty(base));
            fprintf('ok\n');

            fprintf('testing checked cast... ');
            thrower = Test.ThrowerPrx.checkedCast(base);
            assert(~isempty(thrower));
            assert(thrower == base);
            fprintf('ok\n');

            fprintf('catching exact types... ');

            try
                thrower.throwAasA(1);
                assert(false);
            catch ex
                if isa(ex, 'Test.A')
                    assert(ex.aMem == 1);
                else
                    rethrow(ex);
                end
            end

            try
                thrower.throwAorDasAorD(1);
                assert(false);
            catch ex
                if isa(ex, 'Test.A')
                    assert(ex.aMem == 1);
                else
                    rethrow(ex);
                end
            end

            try
                thrower.throwAorDasAorD(-1);
                assert(false);
            catch ex
                if isa(ex, 'Test.D')
                    assert(ex.dMem == -1);
                else
                    rethrow(ex);
                end
            end

            try
                thrower.throwBasB(1, 2);
                assert(false);
            catch ex
                if isa(ex, 'Test.B')
                    assert(ex.aMem == 1);
                    assert(ex.bMem == 2);
                else
                    rethrow(ex);
                end
            end

            try
                thrower.throwCasC(1, 2, 3);
                assert(false);
            catch ex
                if isa(ex, 'Test.C')
                    assert(ex.aMem == 1);
                    assert(ex.bMem == 2);
                    assert(ex.cMem == 3);
                else
                    rethrow(ex);
                end
            end

            fprintf('ok\n');

            fprintf('catching base types... ');

            try
                thrower.throwBasB(1, 2);
                assert(false);
            catch ex
                if isa(ex, 'Test.A')
                    assert(ex.aMem == 1);
                else
                    rethrow(ex);
                end
            end

            try
                thrower.throwCasC(1, 2, 3);
                assert(false);
            catch ex
                if isa(ex, 'Test.B')
                    assert(ex.aMem == 1);
                    assert(ex.bMem == 2);
                else
                    rethrow(ex);
                end
            end

            fprintf('ok\n');

            fprintf('catching derived types... ');

            try
                thrower.throwBasA(1, 2);
                assert(false);
            catch ex
                if isa(ex, 'Test.B')
                    assert(ex.aMem == 1);
                    assert(ex.bMem == 2);
                else
                    rethrow(ex);
                end
            end

            try
                thrower.throwCasA(1, 2, 3);
                assert(false);
            catch ex
                if isa(ex, 'Test.C')
                    assert(ex.aMem == 1);
                    assert(ex.bMem == 2);
                    assert(ex.cMem == 3);
                else
                    rethrow(ex);
                end
            end

            try
                thrower.throwCasB(1, 2, 3);
                assert(false);
            catch ex
                if isa(ex, 'Test.C')
                    assert(ex.aMem == 1);
                    assert(ex.bMem == 2);
                    assert(ex.cMem == 3);
                else
                    rethrow(ex);
                end
            end

            fprintf('ok\n');

            if thrower.supportsUndeclaredExceptions()
                fprintf('catching unknown user exception... ');

                try
                    thrower.throwUndeclaredA(1);
                    assert(false);
                catch ex
                    if ~isa(ex, 'Ice.UnknownUserException')
                        rethrow(ex);
                    end
                end

                try
                    thrower.throwUndeclaredB(1, 2);
                    assert(false);
                catch ex
                    if ~isa(ex, 'Ice.UnknownUserException')
                        rethrow(ex);
                    end
                end

                try
                    thrower.throwUndeclaredC(1, 2, 3);
                    assert(false);
                catch ex
                    if ~isa(ex, 'Ice.UnknownUserException')
                        rethrow(ex);
                    end
                end

                fprintf('ok\n');
            end

            if thrower.supportsAssertException()
                fprintf('testing assert in the server... ');

                try
                    thrower.throwAssertException();
                    assert(false);
                catch ex
                    if ~isa(ex, 'Ice.UnknownException') && ~isa(ex, 'Ice.ConnectionLostException')
                        rethrow(ex);
                    end
                end

                fprintf('ok\n');
            end

            if ~isempty(thrower.ice_getConnection())
                fprintf('testing memory limit marshal exception... ');
                try
                    thrower.throwMemoryLimitException([]);
                    assert(false);
                catch ex
                    if ~isa(ex, 'Ice.MemoryLimitException')
                        rethrow(ex);
                    end
                end

                try
                    thrower.throwMemoryLimitException(zeros(1, 20 * 1024)); % 20KB
                    assert(false);
                catch ex
                    if ~isa(ex, 'Ice.UnknownLocalException') && ~isa(ex, 'Ice.ConnectionLostException') && ...
                       ~isa(ex, 'Ice.SocketException')
                        rethrow(ex);
                    end
                end

                try
                    thrower2 = Test.ThrowerPrx.uncheckedCast(...
                        communicator.stringToProxy(['thrower:', app.getTestEndpoint(1, '')]));
                    try
                        thrower2.throwMemoryLimitException(zeros(1, 2 * 1024 * 1024)); % 2MB (no limits)
                    catch ex
                        if ~isa(ex, 'Ice.MemoryLimitException')
                            rethrow(ex);
                        end
                    end
                    thrower3 = Test.ThrowerPrx.uncheckedCast(...
                        communicator.stringToProxy(['thrower:', app.getTestEndpoint(2, '')]));
                    try
                        thrower3.throwMemoryLimitException(zeros(1, 1024)); % 1KB limit
                        assert(false);
                    catch ex
                        if ~isa(ex, 'Ice.ConnectionLostException')
                            rethrow(ex);
                        end
                    end
                catch ex
                    if ~isa(ex, 'Ice.ConnectionRefusedException') % Expected with JS bidir server
                        rethrow(ex);
                    end
                end

                fprintf('ok\n');
            end

            fprintf('catching object not exist exception... ');

            id = Ice.stringToIdentity('does not exist');
            try
                thrower2 = Test.ThrowerPrx.uncheckedCast(thrower.ice_identity(id));
                thrower2.ice_ping();
                assert(false);
            catch ex
                if isa(ex, 'Ice.ObjectNotExistException')
                    assert(isequal(ex.id, id));
                else
                    rethrow(ex);
                end
            end

            fprintf('ok\n');

            fprintf('catching facet not exist exception... ');

            thrower2 = Test.ThrowerPrx.uncheckedCast(thrower, 'no such facet');
            try
                thrower2.ice_ping();
                assert(false);
            catch ex
                if isa(ex, 'Ice.FacetNotExistException')
                    assert(strcmp(ex.facet, 'no such facet'));
                else
                    rethrow(ex);
                end
            end

            fprintf('ok\n');

            fprintf('catching operation not exist exception... ');

            try
                thrower2 = Test.WrongOperationPrx.uncheckedCast(thrower);
                thrower2.noSuchOperation();
                assert(false);
            catch ex
                if isa(ex, 'Ice.OperationNotExistException')
                    assert(strcmp(ex.operation, 'noSuchOperation'));
                else
                    rethrow(ex);
                end
            end

            fprintf('ok\n');

            fprintf('catching unknown local exception... ');

            try
                thrower.throwLocalException();
                assert(false);
            catch ex
                if ~isa(ex, 'Ice.UnknownLocalException')
                    rethrow(ex);
                end
            end

            try
                thrower.throwLocalExceptionIdempotent();
                assert(false);
            catch ex
                if ~isa(ex, 'Ice.UnknownLocalException') && ~isa(ex, 'Ice.OperationNotExistException')
                    rethrow(ex);
                end
            end

            fprintf('ok\n');

            fprintf('catching unknown non-Ice exception... ');

            try
                thrower.throwNonIceException();
                assert(false);
            catch ex
                if ~isa(ex, 'Ice.UnknownException')
                    rethrow(ex);
                end
            end

            fprintf('ok\n');

            fprintf('testing asynchronous exceptions... ');

            try
                thrower.throwAfterResponse();
            catch ex
                rethrow(ex);
            end

            try
                thrower.throwAfterException();
                assert(false);
            catch ex
                if ~isa(ex, 'Test.A')
                    rethrow(ex);
                end
            end

            fprintf('ok\n');

            fprintf('catching exact types with AMI mapping... ');

            try
                thrower.throwAasAAsync(1).fetchOutputs();
                assert(false);
            catch ex
                if isa(ex, 'Test.A')
                    assert(ex.aMem == 1);
                else
                    rethrow(ex);
                end
            end

            try
                thrower.throwAorDasAorDAsync(1).fetchOutputs();
                assert(false);
            catch ex
                if isa(ex, 'Test.A')
                    assert(ex.aMem == 1);
                else
                    rethrow(ex);
                end
            end

            try
                thrower.throwAorDasAorDAsync(-1).fetchOutputs();
                assert(false);
            catch ex
                if isa(ex, 'Test.D')
                    assert(ex.dMem == -1);
                else
                    rethrow(ex);
                end
            end

            try
                thrower.throwBasBAsync(1, 2).fetchOutputs();
                assert(false);
            catch ex
                if isa(ex, 'Test.B')
                    assert(ex.aMem == 1);
                    assert(ex.bMem == 2);
                else
                    rethrow(ex);
                end
            end

            try
                thrower.throwCasCAsync(1, 2, 3).fetchOutputs();
                assert(false);
            catch ex
                if isa(ex, 'Test.C')
                    assert(ex.aMem == 1);
                    assert(ex.bMem == 2);
                    assert(ex.cMem == 3);
                else
                    rethrow(ex);
                end
            end

            fprintf('ok\n');

            fprintf('catching derived types with AMI mapping... ');

            try
                thrower.throwBasAAsync(1, 2).fetchOutputs();
                assert(false);
            catch ex
                if isa(ex, 'Test.B')
                    assert(ex.aMem == 1);
                    assert(ex.bMem == 2);
                else
                    rethrow(ex);
                end
            end

            try
                thrower.throwCasAAsync(1, 2, 3).fetchOutputs();
                assert(false);
            catch ex
                if isa(ex, 'Test.C')
                    assert(ex.aMem == 1);
                    assert(ex.bMem == 2);
                    assert(ex.cMem == 3);
                else
                    rethrow(ex);
                end
            end

            try
                thrower.throwCasBAsync(1, 2, 3).fetchOutputs();
                assert(false);
            catch ex
                if isa(ex, 'Test.C')
                    assert(ex.aMem == 1);
                    assert(ex.bMem == 2);
                    assert(ex.cMem == 3);
                else
                    rethrow(ex);
                end
            end

            fprintf('ok\n');

            if thrower.supportsUndeclaredExceptions()
                fprintf('catching unknown user exception with AMI mapping... ');

                try
                    thrower.throwUndeclaredAAsync(1).fetchOutputs();
                    assert(false);
                catch ex
                    if ~isa(ex, 'Ice.UnknownUserException')
                        rethrow(ex);
                    end
                end

                try
                    thrower.throwUndeclaredBAsync(1, 2).fetchOutputs();
                    assert(false);
                catch ex
                    if ~isa(ex, 'Ice.UnknownUserException')
                        rethrow(ex);
                    end
                end

                try
                    thrower.throwUndeclaredCAsync(1, 2, 3).fetchOutputs();
                    assert(false);
                catch ex
                    if ~isa(ex, 'Ice.UnknownUserException')
                        rethrow(ex);
                    end
                end

                fprintf('ok\n');
            end

            if thrower.supportsAssertException()
                fprintf('catching assert in the server with AMI mapping... ');

                try
                    thrower.throwAssertExceptionAsync().fetchOutputs();
                    assert(false);
                catch ex
                    if ~isa(ex, 'Ice.UnknownException') && ~isa(ex, 'Ice.ConnectionLostException')
                        rethrow(ex);
                    end
                end

                fprintf('ok\n');
            end

            fprintf('catching object not exist exception with AMI mapping... ');

            id = Ice.stringToIdentity('does not exist');
            thrower2 = Test.ThrowerPrx.uncheckedCast(thrower.ice_identity(id));
            try
                thrower2.throwAasAAsync(1).fetchOutputs();
                assert(false);
            catch ex
                if isa(ex, 'Ice.ObjectNotExistException')
                    assert(isequal(ex.id, id));
                else
                    rethrow(ex);
                end
            end

            fprintf('ok\n');

            fprintf('catching facet not exist exception with AMI mapping... ');

            thrower2 = Test.ThrowerPrx.uncheckedCast(thrower, 'no such facet');
            try
                thrower2.throwAasAAsync(1).fetchOutputs();
                assert(false);
            catch ex
                if isa(ex, 'Ice.FacetNotExistException')
                    assert(strcmp(ex.facet, 'no such facet'));
                else
                    rethrow(ex);
                end
            end

            fprintf('ok\n');

            fprintf('catching operation not exist exception with AMI mapping... ');

            thrower2 = Test.WrongOperationPrx.uncheckedCast(thrower);
            try
                thrower2.noSuchOperationAsync().fetchOutputs();
                assert(false);
            catch ex
                if isa(ex, 'Ice.OperationNotExistException')
                    assert(strcmp(ex.operation, 'noSuchOperation'));
                else
                    rethrow(ex);
                end
            end

            fprintf('ok\n');

            fprintf('catching unknown local exception with AMI mapping... ');

            try
                thrower.throwLocalExceptionAsync().fetchOutputs();
                assert(false);
            catch ex
                if ~isa(ex, 'Ice.UnknownLocalException') && ~isa(ex, 'Ice.OperationNotExistException')
                    rethrow(ex);
                end
            end

            try
                thrower.throwLocalExceptionIdempotentAsync().fetchOutputs();
                assert(false);
            catch ex
                if ~isa(ex, 'Ice.UnknownLocalException') && ~isa(ex, 'Ice.OperationNotExistException')
                    rethrow(ex);
                end
            end

            fprintf('ok\n');

            fprintf('catching unknown non-Ice exception with AMI mapping... ');

            try
                thrower.throwNonIceExceptionAsync().fetchOutputs();
                assert(false);
            catch ex
                if ~isa(ex, 'Ice.UnknownException')
                    rethrow(ex);
                end
            end

            fprintf('ok\n');

            r = thrower;
        end
    end
end

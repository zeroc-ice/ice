% Copyright (c) ZeroC, Inc.

classdef AllTests
    methods(Static)
        function allTests(helper)
            import Test.*;

            communicator = helper.communicator();

            sref = ['test:', helper.getTestEndpoint()];
            p = TestIntfPrx(communicator, sref);

            sref = ['testController:', helper.getTestEndpoint(1)];
            testController = TestIntfControllerPrx(communicator, sref);

            fprintf('testing async invocation... ');

            ctx = configureDictionary('char', 'char');

            assert(p.ice_isAAsync('::Test::TestIntf').fetchOutputs());
            assert(p.ice_isAAsync('::Test::TestIntf', ctx).fetchOutputs());

            p.ice_pingAsync().fetchOutputs();
            p.ice_pingAsync(ctx).fetchOutputs();

            assert(strcmp(p.ice_idAsync().fetchOutputs(), '::Test::TestIntf'));
            assert(strcmp(p.ice_idAsync(ctx).fetchOutputs(), '::Test::TestIntf'));

            assert(length(p.ice_idsAsync().fetchOutputs()) == 2);
            assert(length(p.ice_idsAsync(ctx).fetchOutputs()) == 2);

            p.opAsync().fetchOutputs();
            p.opAsync(ctx).fetchOutputs();

            assert(p.opWithResultAsync().fetchOutputs() == 15);
            assert(p.opWithResultAsync(ctx).fetchOutputs() == 15);

            try
                p.opWithUEAsync().fetchOutputs();
                assert(false);
            catch ex
                assert(isa(ex, 'Test.TestIntfException'));
            end

            try
                p.opWithUEAsync(ctx).fetchOutputs();
                assert(false);
            catch ex
                assert(isa(ex, 'Test.TestIntfException'));
            end

            if p.supportsFunctionalTests()
                assert(p.opBoolAsync(true).fetchOutputs());

                assert(p.opByteAsync(255).fetchOutputs() == 255);

                assert(p.opShortAsync(intmin('int16')).fetchOutputs() == intmin('int16'));

                assert(p.opIntAsync(intmin('int32')).fetchOutputs() == intmin('int32'));

                assert(p.opLongAsync(intmin('int64')).fetchOutputs() == intmin('int64'));

                assert(p.opFloatAsync(3.14).fetchOutputs() == single(3.14));

                assert(p.opDoubleAsync(1.1E10).fetchOutputs() == 1.1E10);
            end

            fprintf('ok\n');

            fprintf('testing local exceptions... ');

            indirect = p.ice_adapterId('dummy');

            try
                indirect.opAsync().fetchOutputs();
            catch ex
                assert(isa(ex, 'Ice.NoEndpointException'));
            end

            %
            % Check that CommunicatorDestroyedException is raised directly.
            %
            if ~isempty(p.ice_getConnection())
                ic = helper.initialize(communicator.getProperties().clone());
                o = ic.stringToProxy(p.ice_toString());
                p2 = TestIntfPrx.checkedCast(o);
                ic.destroy();

                try
                    p2.opAsync();
                    assert(false);
                catch ex
                    % Expected.
                    assert(isa(ex, 'Ice.CommunicatorDestroyedException'));
                end
            end

            fprintf('ok\n');

            fprintf('testing future operations... ');

            indirect = p.ice_adapterId('dummy');
            f = indirect.opAsync();
            assert(~f.wait());
            assert(~f.wait('finished'));
            assert(~f.wait('finished', 1));

            try
                f.fetchOutputs();
            catch ex
                assert(isa(ex, 'Ice.NoEndpointException'));
            end

            testController.holdAdapter();
            f1 = [];
            f2 = [];
            try
                f1 = p.opAsync();
                f2 = p.ice_idAsync();
                while ~strcmp(f1.State, 'sent')
                    pause(0.1);
                end
                while ~strcmp(f2.State, 'sent')
                    pause(0.1);
                end
                f1.cancel();
                assert(~f1.wait());
                f2.cancel();
                assert(~f2.wait());
                try
                    f1.fetchOutputs();
                    assert(false);
                catch ex
                    assert(isa(ex, 'Ice.InvocationCanceledException'));
                end
                try
                    f2.fetchOutputs();
                    assert(false);
                catch ex
                    assert(isa(ex, 'Ice.InvocationCanceledException'));
                end

                f1 = p.opAsync();
                assert(f1.wait('sent', 1));
                assert(~f1.wait('finished', 0.1));
            catch ex
                testController.resumeAdapter();
                rethrow(ex);
            end
            testController.resumeAdapter();

            assert(f1.wait('finished', 1));

            fprintf('ok\n');

            if ~isempty(p.ice_getConnection())
                fprintf('testing connection close... ');

                %
                % Local case: begin a request, close the connection gracefully, and make sure it waits
                % for the request to complete.
                %
                con = p.ice_getConnection();
                f = p.sleepAsync(100);
                con.close().fetchOutputs();
                try
                    f.fetchOutputs(); % Should complete successfully
                catch ex
                    assert(false);
                end

                fprintf('ok\n');

                fprintf('testing connection abort... ');

                %
                % Local case: start an operation and then close the connection forcefully on the client side.
                % There will be no retry and we expect the invocation to fail with ConnectionAbortedException.
                %
                p.ice_ping();
                con = p.ice_getConnection();
                f = p.startDispatchAsync();
                while ~strcmp(f.State, 'sent') % Ensure the request was sent before we close the connection
                    pause(0.1);
                end
                con.abort();
                try
                    f.fetchOutputs();
                    assert(false);
                catch ex
                    assert(isa(ex, 'Ice.ConnectionAbortedException'));
                    assert(ex.closedByApplication);
                end
                p.finishDispatch();

                %
                % Remote case: the server closes the connection forcefully. This causes the request to fail
                % with a ConnectionLostException. Since the close() operation is not idempotent, the client
                % will not retry.
                %
                try
                    p.abortConnection();
                    assert(false);
                catch ex
                    % Expected.
                    assert(isa(ex, 'Ice.ConnectionLostException'));
                end

                fprintf('ok\n');
            end

            p.shutdown();
        end
    end
end

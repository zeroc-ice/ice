%
% Copyright (c) ZeroC, Inc. All rights reserved.
%

classdef AllTests
    methods(Static)
        function r = connect(prx)
            nRetry = 10;
            while nRetry > 0
                nRetry = nRetry - 1;
                try
                    prx.ice_getConnection();
                    break;
                catch ex
                    if isa(ex, 'Ice.ConnectTimeoutException')
                        % Can sporadically occur with slow machines
                    else
                        rethrow(ex);
                    end
                end
            end
            r = prx.ice_getConnection(); % Establish connection
        end
        function allTests(helper)
            import Test.*;

            controller = ControllerPrx(helper.communicator(), ['controller:', helper.getTestEndpoint(1)]);
            connect(controller);
            try
                AllTests.allTestsWithController(helper, controller);
            catch ex
                % Ensure the adapter is not in the holding state when an unexpected exception occurs to prevent
                % the test from hanging on exit in case a connection which disables timeouts is still opened.
                controller.resumeAdapter();
                rethrow(ex);
            end
        end
        function allTestsWithController(helper, controller)
            import Test.*;

            communicator = helper.communicator();

            sref = ['timeout:', helper.getTestEndpoint()];
            timeout = TimeoutPrx(communicator, sref);

            fprintf('testing invocation timeout... ');

            connection = timeout.ice_getConnection();
            to = timeout.ice_invocationTimeout(100);
            assert(connection == to.ice_getConnection());
            try
                to.sleep(1000);
                assert(false);
            catch ex
                assert(isa(ex, 'Ice.InvocationTimeoutException'));
            end
            timeout.ice_ping();
            to = timeout.ice_invocationTimeout(500);
            assert(connection == to.ice_getConnection());
            try
                to.sleep(100);
            catch ex
                if isa(ex, 'Ice.InvocationTimeoutException')
                    assert(false);
                else
                    rethrow(ex);
                end
            end
            assert(connection == to.ice_getConnection());

            %
            % Expect InvocationTimeoutException.
            %
            to = timeout.ice_invocationTimeout(100);
            f = to.sleepAsync(1000);
            try
                f.fetchOutputs();
            catch ex
                assert(isa(ex, 'Ice.TimeoutException'));
            end
            timeout.ice_ping();

            %
            % Expect success.
            %
            to = timeout.ice_invocationTimeout(1000);
            f = to.sleepAsync(100);
            f.fetchOutputs();

            fprintf('ok\n');

            controller.shutdown();
        end
    end
end

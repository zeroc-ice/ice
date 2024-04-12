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

            controller = ControllerPrx.checkedCast(...
                helper.communicator().stringToProxy(['controller:', helper.getTestEndpoint(1)]));
            assert(~isempty(controller));
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
            obj = communicator.stringToProxy(sref);
            assert(~isempty(obj));

            timeout = TimeoutPrx.checkedCast(obj);
            assert(~isempty(timeout));

            % The sequence needs to be large enough to fill the write/recv buffers
            bufSize = 2000000;
            seq = zeros(1, bufSize, 'uint8');

            fprintf('testing connection timeout... ');

            %
            % Expect TimeoutException.
            %
            to = timeout.ice_timeout(250);
            AllTests.connect(to);
            controller.holdAdapter(-1);
            try
                to.sendData(seq);
                assert(false);
            catch ex
                % Expected.
                assert(isa(ex, 'Ice.TimeoutException'));
            end
            controller.resumeAdapter();
            timeout.op(); % Ensure adapter is active.

            %
            % Expect success.
            %
            to = timeout.ice_timeout(2000);
            controller.holdAdapter(500);
            try
                to.sendData(zeros(1, 1000000, 'uint8'));
            catch ex
                if isa(ex, 'Ice.TimeoutException')
                    assert(false);
                else
                    rethrow(ex);
                end
            end

            fprintf('ok\n');

            fprintf('testing invocation timeout... ');

            connection = obj.ice_getConnection();
            to = timeout.ice_invocationTimeout(100);
            assert(connection == to.ice_getConnection());
            try
                to.sleep(1000);
                assert(false);
            catch ex
                assert(isa(ex, 'Ice.InvocationTimeoutException'));
            end
            obj.ice_ping();
            to = TimeoutPrx.checkedCast(obj.ice_invocationTimeout(500));
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
            obj.ice_ping();

            %
            % Expect success.
            %
            to = timeout.ice_invocationTimeout(1000);
            f = to.sleepAsync(100);
            f.fetchOutputs();

            %
            % Backward compatible connection timeouts
            %
            to = timeout.ice_invocationTimeout(-2).ice_timeout(250);
            con = AllTests.connect(to);
            try
                to.sleep(750);
                assert(false);
            catch ex
                if isa(ex, 'Ice.TimeoutException')
                    assert(~isempty(con));
                    try
                        con.getInfo();
                        assert(false);
                    catch ex
                        if isa(ex, 'Ice.TimeoutException')
                            % Connection got closed as well.
                        else
                            rethrow(ex);
                        end
                    end
                end
            end
            obj.ice_ping();

            try
                con = AllTests.connect(to);
                to.sleepAsync(750).fetchOutputs();
                assert(false);
            catch ex
                assert(isa(ex, 'Ice.TimeoutException'));
                assert(~isempty(con));
                try
                    con.getInfo();
                    assert(false);
                catch ex
                    if isa(ex, 'Ice.TimeoutException')
                        % Connection got closed as well.
                    else
                        rethrow(ex);
                    end
                end
            end
            obj.ice_ping();

            fprintf('ok\n');

            controller.shutdown();
        end
    end
end

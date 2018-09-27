%{
**********************************************************************

Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.

This copy of Ice is licensed to you under the terms described in the
ICE_LICENSE file included in this distribution.

**********************************************************************
%}

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
        function allTests(app)
            import Test.*;

            communicator = app.communicator();

            sref = ['timeout:', app.getTestEndpoint(0)];
            obj = communicator.stringToProxy(sref);
            assert(~isempty(obj));

            timeout = TimeoutPrx.checkedCast(obj);
            assert(~isempty(timeout));

            controller = ControllerPrx.checkedCast(communicator.stringToProxy(['controller:', app.getTestEndpoint(1)]));
            assert(~isempty(controller));

            fprintf('testing connect timeout... ');

            %
            % Expect ConnectTimeoutException.
            %
            to = timeout.ice_timeout(100);
            controller.holdAdapter(-1);
            try
                to.op();
                assert(false);
            catch ex
                % Expected.
                assert(isa(ex, 'Ice.ConnectTimeoutException'));
            end
            controller.resumeAdapter();
            timeout.op(); % Ensure adapter is active.

            %
            % Expect success.
            %
            to = timeout.ice_timeout(-1);
            controller.holdAdapter(100);
            try
                to.op();
            catch ex
                if isa(ex, 'Ice.ConnectTimeoutException')
                    assert(false);
                else
                    rethrow(ex);
                end
            end

            fprintf('ok\n');

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
                to.sleep(750);
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
            f = to.sleepAsync(750);
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

            fprintf('testing close timeout... ');

            to = TimeoutPrx.uncheckedCast(obj.ice_timeout(250));
            connection = AllTests.connect(to);
            controller.holdAdapter(-1);
            connection.close(Ice.ConnectionClose.GracefullyWithWait);
            try
                connection.getInfo(); % getInfo() doesn't throw in the closing state.
            catch ex
                assert(false);
            end

            pause(.650);

            try
                connection.getInfo();
                assert(false);
            catch ex
                % Expected.
                assert(isa(ex, 'Ice.ConnectionManuallyClosedException'));
                assert(ex.graceful);
            end
            controller.resumeAdapter();
            timeout.op(); % Ensure adapter is active.

            fprintf('ok\n');

            fprintf('testing timeout overrides... ');

            %
            % Test Ice.Override.Timeout. This property overrides all
            % endpoint timeouts.
            %
            initData = app.cloneInitData();
            initData.properties_.setProperty('Ice.Override.ConnectTimeout', '250');
            initData.properties_.setProperty('Ice.Override.Timeout', '100');
            comm = Ice.initialize(initData);
            to = TimeoutPrx.uncheckedCast(comm.stringToProxy(sref));
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
            % Calling ice_timeout() should have no effect.
            %
            to = TimeoutPrx.uncheckedCast(to.ice_timeout(1000));
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
            comm.destroy();

            %
            % Test Ice.Override.ConnectTimeout.
            %
            initData = app.cloneInitData();
            initData.properties_.setProperty('Ice.Override.ConnectTimeout', '250');

            comm = Ice.initialize(initData);
            to = TimeoutPrx.uncheckedCast(comm.stringToProxy(sref));
            controller.holdAdapter(-1);
            try
                to.op();
                assert(false);
            catch ex
                % Expected.
                assert(isa(ex, 'Ice.ConnectTimeoutException'));
            end
            controller.resumeAdapter();
            timeout.op(); % Ensure adapter is active.

            %
            % Calling ice_timeout() should have no effect on the connect timeout.
            %
            controller.holdAdapter(-1);
            to = to.ice_timeout(1000);
            try
                to.op();
                assert(false);
            catch ex
                % Expected.
                assert(isa(ex, 'Ice.ConnectTimeoutException'));
            end
            controller.resumeAdapter();
            timeout.op(); % Ensure adapter is active.

            %
            % Verify that timeout set via ice_timeout() is still used for requests.
            %
            to = to.ice_timeout(250);
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
            comm.destroy();

            %
            % Test Ice.Override.CloseTimeout.
            %
            initData = app.cloneInitData();
            initData.properties_.setProperty('Ice.Override.CloseTimeout', '100');
            comm = Ice.initialize(initData);
            comm.stringToProxy(sref).ice_getConnection();
            controller.holdAdapter(-1);
            tic();
            comm.destroy();
            assert(toc() < 1.0);
            controller.resumeAdapter();
            timeout.op(); % Ensure adapter is active.

            fprintf('ok\n');

            controller.shutdown();
        end
    end
end

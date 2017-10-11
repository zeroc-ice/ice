%{
**********************************************************************

Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.

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
        function r = allTests(app)
            import Test.*;

            communicator = app.communicator();

            sref = ['timeout:', app.getTestEndpoint(0)];
            obj = communicator.stringToProxy(sref);
            assert(~isempty(obj));

            mult = 1;
            if ~strcmp(communicator.getProperties().getPropertyWithDefault('Ice.Default.Protocol', 'tcp'), 'tcp')
                mult = 4;
            end

            timeout = TimeoutPrx.checkedCast(obj);
            assert(~isempty(timeout));

            fprintf('testing connect timeout... ');

            %
            % Expect ConnectTimeoutException.
            %
            to = timeout.ice_timeout(100 * mult);
            timeout.holdAdapter(500 * mult);
            try
                to.op();
                assert(false);
            catch ex
                % Expected.
                assert(isa(ex, 'Ice.ConnectTimeoutException'));
            end

            %
            % Expect success.
            %
            timeout.op(); % Ensure adapter is active.
            to = timeout.ice_timeout(1000 * mult);
            timeout.holdAdapter(500 * mult);
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
            timeout.holdAdapter(750 * mult);
            try
                to.sendData(seq);
                assert(false);
            catch ex
                % Expected.
                assert(isa(ex, 'Ice.TimeoutException'));
            end

            %
            % Expect success.
            %
            timeout.op(); % Ensure adapter is active.
            to = timeout.ice_timeout(1000 * mult);
            timeout.holdAdapter(500 * mult);
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
                to.sleep(750 * mult);
                assert(false);
            catch ex
                assert(isa(ex, 'Ice.InvocationTimeoutException'));
            end
            obj.ice_ping();
            to = TimeoutPrx.checkedCast(obj.ice_invocationTimeout(500 * mult));
            assert(connection == to.ice_getConnection());
            try
                to.sleep(100 * mult);
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
            f = to.sleepAsync(750 * mult);
            try
                f.fetchOutputs();
            catch ex
                assert(isa(ex, 'Ice.TimeoutException'));
            end
            obj.ice_ping();

            %
            % Expect success.
            %
            to = timeout.ice_invocationTimeout(500 * mult);
            f = to.sleepAsync(100 * mult);
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

            to = TimeoutPrx.uncheckedCast(obj.ice_timeout(250 * mult));
            connection = AllTests.connect(to);
            timeout.holdAdapter(600);
            connection.close(Ice.ConnectionClose.GracefullyWithWait);
            try
                connection.getInfo(); % getInfo() doesn't throw in the closing state.
            catch ex
                assert(false);
            end

            pause(.650 * mult);

            try
                connection.getInfo();
                assert(false);
            catch ex
                % Expected.
                assert(isa(ex, 'Ice.ConnectionManuallyClosedException'));
                assert(ex.graceful);
            end
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
            timeout.holdAdapter(500 * mult);
            try
                to.sendData(seq);
                assert(false);
            catch ex
                % Expected.
                assert(isa(ex, 'Ice.TimeoutException'));
            end
            %
            % Calling ice_timeout() should have no effect.
            %
            timeout.op(); % Ensure adapter is active.
            to = TimeoutPrx.uncheckedCast(to.ice_timeout(1000 * mult));
            AllTests.connect(to);
            timeout.holdAdapter(500 * mult);
            try
                to.sendData(seq);
                assert(false);
            catch ex
                % Expected.
                assert(isa(ex, 'Ice.TimeoutException'));
            end
            comm.destroy();

            %
            % Test Ice.Override.ConnectTimeout.
            %
            initData = app.cloneInitData();
            if mult == 1
                initData.properties_.setProperty('Ice.Override.ConnectTimeout', '250');
            else
                initData.properties_.setProperty('Ice.Override.ConnectTimeout', '2500');
            end

            comm = Ice.initialize(initData);
            to = TimeoutPrx.uncheckedCast(comm.stringToProxy(sref));
            timeout.holdAdapter(750 * mult);
            try
                to.op();
                assert(false);
            catch ex
                % Expected.
                assert(isa(ex, 'Ice.ConnectTimeoutException'));
            end
            %
            % Calling ice_timeout() should have no effect on the connect timeout.
            %
            timeout.op(); % Ensure adapter is active.
            timeout.holdAdapter(750 * mult);
            to = to.ice_timeout(1000 * mult);
            try
                to.op();
                assert(false);
            catch ex
                % Expected.
                assert(isa(ex, 'Ice.ConnectTimeoutException'));
            end
            %
            % Verify that timeout set via ice_timeout() is still used for requests.
            %
            timeout.op(); % Ensure adapter is active.
            to = to.ice_timeout(250);
            AllTests.connect(to);
            timeout.holdAdapter(750 * mult);
            try
                to.sendData(seq);
                assert(false);
            catch ex
                % Expected.
                assert(isa(ex, 'Ice.TimeoutException'));
            end
            comm.destroy();
            %
            % Test Ice.Override.CloseTimeout.
            %
            initData = app.cloneInitData();
            initData.properties_.setProperty('Ice.Override.CloseTimeout', '100');
            comm = Ice.initialize(initData);
            comm.stringToProxy(sref).ice_getConnection();
            timeout.holdAdapter(800);
            tic();
            comm.destroy();
            assert(toc() < .7);

            fprintf('ok\n');

            r = timeout;
        end
    end
end

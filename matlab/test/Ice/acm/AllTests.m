%{
**********************************************************************

Copyright (c) 2003-present ZeroC, Inc. All rights reserved.

**********************************************************************
%}

classdef AllTests
    methods(Static)
        function allTests(helper)
            import Test.*;

            communicator = helper.communicator();

            ref = ['communicator:', helper.getTestEndpoint()];
            com = RemoteCommunicatorPrx.uncheckedCast(communicator.stringToProxy(ref));

            AllTests.testSetACM(communicator, com);
            AllTests.testHeartbeatManual(communicator, com);

            com.shutdown();
        end
        function testSetACM(communicator, com)
            import Test.*;

            fprintf('testing setACM/getACM... ');

            adapter = com.createObjectAdapter(-1, -1, -1);

            initData = Ice.InitializationData();
            initData.properties_ = communicator.getProperties().clone();
            initData.properties_.setProperty('Ice.ACM.Timeout', '1');
            initData.properties_.setProperty('Ice.ACM.Client.Timeout', '15');
            initData.properties_.setProperty('Ice.ACM.Client.Close', '4');
            initData.properties_.setProperty('Ice.ACM.Client.Heartbeat', '2');
            testCommunicator = Ice.initialize(initData);
            proxy = TestIntfPrx.uncheckedCast(testCommunicator.stringToProxy(adapter.getTestIntf().ice_toString()));
            proxy.ice_getConnection();

            try
                proxy.ice_getCachedConnection().setACM(-19, Ice.Unset, Ice.Unset);
                assert(false);
            catch ex
            end

            acm = proxy.ice_getCachedConnection().getACM();
            assert(acm.timeout == 15);
            assert(acm.close == Ice.ACMClose.CloseOnIdleForceful);
            assert(acm.heartbeat == Ice.ACMHeartbeat.HeartbeatOnIdle);

            proxy.ice_getCachedConnection().setACM(Ice.Unset, Ice.Unset, Ice.Unset);
            acm = proxy.ice_getCachedConnection().getACM();
            assert(acm.timeout == 15);
            assert(acm.close == Ice.ACMClose.CloseOnIdleForceful);
            assert(acm.heartbeat == Ice.ACMHeartbeat.HeartbeatOnIdle);

            proxy.ice_getCachedConnection().setACM(1, Ice.ACMClose.CloseOnInvocationAndIdle, ...
                                                   Ice.ACMHeartbeat.HeartbeatAlways);
            acm = proxy.ice_getCachedConnection().getACM();
            assert(acm.timeout == 1);
            assert(acm.close == Ice.ACMClose.CloseOnInvocationAndIdle);
            assert(acm.heartbeat == Ice.ACMHeartbeat.HeartbeatAlways);

            proxy.startHeartbeatCount();
            proxy.waitForHeartbeatCount(2);

            adapter.deactivate();
            testCommunicator.destroy();
            fprintf('ok\n');
        end
        function testHeartbeatManual(communicator, com)
            import Test.*;

            fprintf('testing manual heartbeats... ');

            adapter = com.createObjectAdapter(10, -1, 0);

            initData = Ice.InitializationData();
            initData.properties_ = communicator.getProperties().clone();
            initData.properties_.setProperty('Ice.ACM.Timeout', '10');
            initData.properties_.setProperty('Ice.ACM.Client.Timeout', '10');
            initData.properties_.setProperty('Ice.ACM.Client.Close', '0');
            initData.properties_.setProperty('Ice.ACM.Client.Heartbeat', '0');
            testCommunicator = Ice.initialize(initData);
            proxy = TestIntfPrx.uncheckedCast(testCommunicator.stringToProxy(adapter.getTestIntf().ice_toString()));
            con = proxy.ice_getConnection();

            proxy.startHeartbeatCount();
            con.heartbeat();
            con.heartbeat();
            con.heartbeat();
            con.heartbeat();
            con.heartbeat();
            proxy.waitForHeartbeatCount(5);

            adapter.deactivate();
            testCommunicator.destroy();
            fprintf('ok\n');
        end
    end
end

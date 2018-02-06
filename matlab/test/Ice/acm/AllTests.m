%{
**********************************************************************

Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.

This copy of Ice is licensed to you under the terms described in the
ICE_LICENSE file included in this distribution.

**********************************************************************
%}

classdef AllTests
    methods(Static)
        function allTests(app)
            import Test.*;

            communicator = app.communicator();

            ref = 'communicator:default -p 12010';
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

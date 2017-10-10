%{
**********************************************************************

Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.

This copy of Ice is licensed to you under the terms described in the
ICE_LICENSE file included in this distribution.

**********************************************************************
%}

classdef Client < Application
    methods
        function r = run(obj, args)
            timeout = AllTests.allTests(obj);
            timeout.shutdown();
            r = 0;
        end
    end
    methods(Access=protected)
        function [r, remArgs] = getInitData(obj, args)
            [initData, remArgs] = getInitData@Application(obj, args);
            initData.properties_.setProperty('Ice.Package.Test', 'test.Ice.timeout');

            %
            % For this test, we want to disable retries.
            %
            initData.properties_.setProperty('Ice.RetryIntervals', '-1');

            %
            % This test kills connections, so we don't want warnings.
            %
            initData.properties_.setProperty('Ice.Warn.Connections', '0');

            %
            % Limit the send buffer size, this test relies on the socket
            % send() blocking after sending a given amount of data.
            %
            initData.properties_.setProperty('Ice.TCP.SndSize', '50000');

            r = initData;
        end
    end
    methods(Static)
        function status = start(args)
            addpath('generated');
            if ~libisloaded('icematlab')
                loadlibrary('icematlab', 'icematlab_proto')
            end
            c = Client();
            status = c.main('Client', args);
        end
    end
end

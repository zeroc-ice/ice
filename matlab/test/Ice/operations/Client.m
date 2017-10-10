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
            myClass = AllTests.allTests(obj);

            fprintf('testing server shutdown... ');
            myClass.shutdown();
            try
                myClass.ice_timeout(100).ice_ping(); % Use timeout to speed up testing on Windows
                throw(MException());
            catch ex
                if isa(ex, 'Ice.LocalException')
                    fprintf('ok\n');
                else
                    rethrow(ex);
                end
            end

            r = 0;
        end
    end
    methods(Access=protected)
        function [r, remArgs] = getInitData(obj, args)
            [initData, remArgs] = getInitData@Application(obj, args);
            initData.properties_.setProperty('Ice.Package.Test', 'test.Ice.operations');
            initData.properties_.setProperty('Ice.ThreadPool.Client.Size', '2');
            initData.properties_.setProperty('Ice.ThreadPool.Client.SizeWarn', '0');
            initData.properties_.setProperty('Ice.BatchAutoFlushSize', '100');
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

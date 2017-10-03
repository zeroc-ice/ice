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
            AllTests.allTests(obj);
            r = 0;
        end
    end
    methods(Static)
        function status = start(args)
            addpath('generated');
            if ~libisloaded('icematlab')
                loadlibrary('icematlab')
            end
            c = Client();
            status = c.main('Client', args);
        end
    end
end

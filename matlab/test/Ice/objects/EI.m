%{
**********************************************************************

Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.

This copy of Ice is licensed to you under the terms described in the
ICE_LICENSE file included in this distribution.

**********************************************************************
%}

classdef EI < Test.E
    methods
        function obj = EI()
            obj = obj@Test.E(1, 'hello');
        end

        function r = checkValues(obj)
            r = obj.i == 1 && strcmp(obj.s, 'hello');
        end
    end
end

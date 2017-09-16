%{
**********************************************************************

Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.

This copy of Ice is licensed to you under the terms described in the
ICE_LICENSE file included in this distribution.

**********************************************************************
%}

classdef FI < test.Ice.objects.Test.F
    methods
        function obj = FI(e)
            if nargin == 0
                e = [];
            end
            obj = obj@test.Ice.objects.Test.F(e, e);
        end

        function r = checkValues(obj)
            r = ~isempty(obj.e1) && obj.e1 == obj.e2;
        end
    end
end

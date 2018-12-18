%{
**********************************************************************

Copyright (c) 2003-present ZeroC, Inc. All rights reserved.

**********************************************************************
%}

classdef FI < Test.F
    methods
        function obj = FI(e)
            if nargin == 0
                e = [];
            end
            obj = obj@Test.F(e, e);
        end

        function r = checkValues(obj)
            r = ~isempty(obj.e1) && obj.e1 == obj.e2;
        end
    end
end

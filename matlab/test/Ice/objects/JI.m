%{
**********************************************************************

Copyright (c) 2003-present ZeroC, Inc. All rights reserved.

**********************************************************************
%}

classdef JI < Ice.InterfaceByValue
    methods
        function obj = JI()
            obj = obj@Ice.InterfaceByValue(Test.JPrx.ice_staticId());
        end
    end
end

%
% Copyright (c) ZeroC, Inc. All rights reserved.
%

classdef II < Ice.InterfaceByValue
    methods
        function obj = II()
            obj = obj@Ice.InterfaceByValue(Test.IPrx.ice_staticId());
        end
    end
end

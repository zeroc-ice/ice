%
% Copyright (c) ZeroC, Inc. All rights reserved.
%

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
